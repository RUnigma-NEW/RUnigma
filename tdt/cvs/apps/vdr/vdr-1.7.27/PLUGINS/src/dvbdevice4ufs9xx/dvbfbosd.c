/*
 * dvbfbosd.c: Implementation of the DVB FB On Screen Display
 *
 * See the README file for copyright information and how to reach the author.
 *
 * $Id: dvbfbosd.c $
 */

#include "dvbfbosd.h"
#include <linux/dvb/osd.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/unistd.h>
#include "vdr/tools.h"

#ifdef PLATFORM_STB71XX
/* Dagobert */
#include <sys/mman.h>
#include <linux/fb.h>

typedef struct
{
  int x;
  int y;
  int width;
  int height;
  int depth;

  int numberColors;
  tColor* colors;

  bool isOpened;
} tWindows;
#endif
// --- cDvbFbOsd -----------------------------------------------------------

#define MAXNUMWINDOWS 7 // OSD windows are counted 1...7
#define MAXOSDMEMORY  92000 // number of bytes available to the OSD (for unmodified DVB cards)

class cDvbFbOsd : public cOsd {
private:
#ifdef PLATFORM_STB71XX
  uint32_t* theMem;
  uint32_t* lfb;
  int available;
  int fd;
  struct  fb_var_screeninfo vinfo;
  int screensize;
  int screenWidth;
  int screenHeight;
  bool isFrameBuffer;
  tWindows windows[MAXNUMWINDOWS];
  int currentWindow;
  int bytes_per_line;
#endif
  int osdDev;
  int osdMem;
  bool shown;
  void Cmd(OSD_Command cmd, int color = 0, int x0 = 0, int y0 = 0, int x1 = 0, int y1 = 0, const void *data = NULL);
protected:
  virtual void SetActive(bool On);
public:
  cDvbFbOsd(int Left, int Top, int OsdDev, uint Level);
  virtual ~cDvbFbOsd();
  virtual eOsdError CanHandleAreas(const tArea *Areas, int NumAreas);
  virtual eOsdError SetAreas(const tArea *Areas, int NumAreas);
  virtual void Flush(void);
#ifdef PLATFORM_STB71XX
  void fb_Clear(int x, int y, int width, int height);
  void fb_DrawBlock(const void* data, int depth, int x, int y, int width, int height, int width_bmp);
  void fb_DrawRectangle(int x, int y, int width, int height, unsigned char r, unsigned char g, unsigned char b);
  void getRGB(int index, unsigned char* a, unsigned char* r, unsigned char* g, unsigned char* b);
#endif
  };

cDvbFbOsd::cDvbFbOsd(int Left, int Top, int OsdDev, uint Level)
:cOsd(Left, Top, Level)
{
  osdDev = OsdDev;
  shown = false;
  if (osdDev < 0) {
#ifndef PLATFORM_STB71XX
     esyslog("ERROR: invalid OSD device handle (%d)!", osdDev);
#endif
#ifdef PLATFORM_STB71XX
/* Dagobert */
     osdDev = open("/dev/fb0", O_RDWR);
     fd = osdDev;
     dsyslog("-> fb0 handle (%d)", osdDev);

     // get fixed information about a frame buffer
     fb_fix_screeninfo fix;
     if (ioctl(fd, FBIOGET_FSCREENINFO, &fix)<0) {
        dsyslog("FBIOGET_FSCREENINFO");
        exit(1);
        }

     available=fix.smem_len;
     dsyslog("Fix: %dk video mem\n", available/1024);
     dsyslog("Fix: %x fix.smem_len\n", fix.smem_len);
     dsyslog("Fix: %lx fix.smem_start\n", fix.smem_start);
     dsyslog("Fix: %d line length\n", fix.line_length);
     dsyslog("Fix: %s id\n", fix.id);
     dsyslog("Fix: %x type\n", fix.type);

     //
     if (ioctl(osdDev, FBIOGET_VSCREENINFO, &vinfo)) {
        dsyslog("Error reading variable information.\n");
        exit(1);
        }

     screensize = vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8;
     screenWidth = vinfo.xres;
     screenHeight = vinfo.yres;
     bytes_per_line = (vinfo.bits_per_pixel / 8) * screenWidth;
     dsyslog("%d %dx%d@%d %d\n", screensize, screenWidth, screenHeight, vinfo.bits_per_pixel, bytes_per_line);

     //theMem = (unsigned char *)mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, osdDev, 0);
     theMem = (uint32_t *)mmap(0, available, PROT_READ | PROT_WRITE, MAP_SHARED, osdDev, 0);
     if (!theMem) {
        dsyslog("mmap");
        exit(1);
        }

     isFrameBuffer = true;
     osdMem = screensize;

     for (int vLoop = 0; vLoop < MAXNUMWINDOWS; vLoop++)
        windows[vLoop].isOpened = false;
     }
#endif
  else {
#ifdef PLATFORM_STB71XX
     /* Dagobert */
     isFrameBuffer = false;
#endif
     osdMem = MAXOSDMEMORY;
#ifdef OSD_CAP_MEMSIZE
     // modified DVB cards may have more OSD memory:
     osd_cap_t cap;
     cap.cmd = OSD_CAP_MEMSIZE;
     if (ioctl(osdDev, OSD_GET_CAPABILITY, &cap) == 0)
        osdMem = cap.val;
#endif
     }
#ifdef USE_YAEPGHD
  if (vidWin.bpp != 0) {
     Cmd(OSD_SetWindow, 0, 5);
     Cmd(OSD_Close);
     }
#endif
}

cDvbFbOsd::~cDvbFbOsd()
{
  SetActive(false);
}

void cDvbFbOsd::SetActive(bool On)
{
  if (On != Active()) {
     cOsd::SetActive(On);
     if (On) {
        // must clear all windows here to avoid flashing effects - doesn't work if done
        // in Flush() only for the windows that are actually used...
        for (int i = 0; i < MAXNUMWINDOWS; i++) {
            Cmd(OSD_SetWindow, 0, i + 1);
            Cmd(OSD_Clear);
            }
        if (GetBitmap(0)) // only flush here if there are already bitmaps
           Flush();
        }
     else if (shown) {
        cBitmap *Bitmap;
        for (int i = 0; (Bitmap = GetBitmap(i)) != NULL; i++) {
            Cmd(OSD_SetWindow, 0, i + 1);
            Cmd(OSD_Close);
            }
        shown = false;
        }
     }
}

eOsdError cDvbFbOsd::CanHandleAreas(const tArea *Areas, int NumAreas)
{
  eOsdError Result = cOsd::CanHandleAreas(Areas, NumAreas);
  if (Result == oeOk) {
     if (NumAreas > MAXNUMWINDOWS)
        return oeTooManyAreas;
     int TotalMemory = 0;
     for (int i = 0; i < NumAreas; i++) {
         if (Areas[i].bpp != 1 && Areas[i].bpp != 2 && Areas[i].bpp != 4 && Areas[i].bpp != 8)
            return oeBppNotSupported;
         if ((Areas[i].Width() & (8 / Areas[i].bpp - 1)) != 0)
            return oeWrongAlignment;
#ifdef PLATFORM_STB71XX
         if (Areas[i].Width() < 1 || Areas[i].Height() < 1 || Areas[i].Width() > 1920 || Areas[i].Height() > 1080)
#else
         if (Areas[i].Width() < 1 || Areas[i].Height() < 1 || Areas[i].Width() > 720 || Areas[i].Height() > 576)
#endif
            return oeWrongAreaSize;
         TotalMemory += Areas[i].Width() * Areas[i].Height() / (8 / Areas[i].bpp);
         }
     if (TotalMemory > osdMem)
        return oeOutOfMemory;
     }
  return Result;
}

eOsdError cDvbFbOsd::SetAreas(const tArea *Areas, int NumAreas)
{
  if (shown) {
     cBitmap *Bitmap;
     for (int i = 0; (Bitmap = GetBitmap(i)) != NULL; i++) {
         Cmd(OSD_SetWindow, 0, i + 1);
         Cmd(OSD_Close);
         }
     shown = false;
     }
  return cOsd::SetAreas(Areas, NumAreas);
}

#ifdef PLATFORM_STB71XX
void cDvbFbOsd::getRGB(int index, unsigned char* a, unsigned char* r, unsigned char* g, unsigned char* b)
{
  // dsyslog("0x%x", windows[currentWindow].colors[index]);
  *a = (windows[currentWindow].colors[index] & 0xFF000000) >> 24;
  *b = (windows[currentWindow].colors[index] & 0xFF0000) >> 16;
  *g = (windows[currentWindow].colors[index] & 0xFF00) >> 8;
  *r = (windows[currentWindow].colors[index] & 0xFF);
  // dsyslog("0x%x 0x%x 0x%x 0x%x", *a, *r, *g, *b);
}

void cDvbFbOsd::fb_Clear(int x, int y, int width, int height)
{
  dsyslog("fb_Clear: x %d, y %d, width %d, height %d\n", x, y, width, height);
  int     vLoopX, vLoopY;

  uint8_t * pos = (uint8_t *)theMem + x * sizeof(uint32_t) + y * bytes_per_line;
  for (vLoopY = 0; vLoopY < height; vLoopY++) {
     uint32_t * dest = (uint32_t *)pos;
     for (vLoopX = 0; vLoopX < width; vLoopX++) {
        *(dest++) = 0;
        }
     pos += bytes_per_line;
     }
}

void cDvbFbOsd::fb_DrawBlock(const void* data, int depth, int x, int y, int width, int height, int width_bmp)
{
  dsyslog("fb_DrawBlock: depth %d, x %d, y %d, width %d, height %d, width_bmp %d\n", depth, x, y, width, height, width_bmp);
  int  vLoopX, vLoopY;

  uint8_t * pos = (uint8_t *)theMem + x * sizeof(uint32_t) + y * bytes_per_line;
  for (vLoopY = 0; vLoopY < height; vLoopY++) {
     unsigned char* quelle = (unsigned char*) data + vLoopY * width_bmp;
     uint32_t * dest = (uint32_t *)pos;
     for (vLoopX = 0; vLoopX < width; vLoopX++) {
        unsigned char index = (unsigned char) (*quelle & 0xff);
        *(dest++) = windows[currentWindow].colors[index];
  /*dsyslog("colors[index] %dx%d: %x, %x, %x, %x\n", vLoopX, vLoopY,
     (int)((windows[currentWindow].colors[index]>>24) & 0xFF),
     (int)((windows[currentWindow].colors[index]>>16) & 0xFF),
     (int)((windows[currentWindow].colors[index]>>8) & 0xFF),
     (int)(windows[currentWindow].colors[index] & 0xFF));
  */
        quelle++;
        }
     pos += bytes_per_line;
     }
}

void cDvbFbOsd::fb_DrawRectangle(int x, int y, int width, int height, unsigned char r, unsigned char g, unsigned char b)
{
  int     vLoopX, vLoopY;

  for (vLoopY = 0; vLoopY < height; vLoopY++) {
     for (vLoopX = 0; vLoopX < width; vLoopX++) {
        *(theMem + ((y + vLoopY) * bytes_per_line) + ((x + vLoopX) * 4) + 0) = 128;
        *(theMem + ((y + vLoopY) * bytes_per_line) + ((x + vLoopX) * 4) + 1) = r;
        *(theMem + ((y + vLoopY) * bytes_per_line) + ((x + vLoopX) * 4) + 2) = g;
        *(theMem + ((y + vLoopY) * bytes_per_line) + ((x + vLoopX) * 4) + 3) = b;
        }
     }
}

#endif
void cDvbFbOsd::Cmd(OSD_Command cmd, int color, int x0, int y0, int x1, int y1, const void *data)
{
#ifdef PLATFORM_STB71XX
  if ((osdDev >= 0) && (!isFrameBuffer)) {
#else
  if (osdDev >= 0) {
#endif
     osd_cmd_t dc;
     dc.cmd   = cmd;
     dc.color = color;
     dc.x0    = x0;
     dc.y0    = y0;
     dc.x1    = x1;
     dc.y1    = y1;
     dc.data  = (void *)data;
     ioctl(osdDev, OSD_SEND_CMD, &dc);
     }
#ifdef PLATFORM_STB71XX
  else {
     /* Dagobert */
     switch (cmd) {
       case OSD_Close:
            //dsyslog("OSD_Close\n");
            windows[currentWindow].isOpened = false;
            fb_Clear( windows[currentWindow].x, windows[currentWindow].y, windows[currentWindow].width, windows[currentWindow].height);
            break;
       case OSD_Open:
            //dsyslog("OSD_Open %d, %d, %d, %d, %d\n", x0, y0, x1, y1, color);
            windows[currentWindow].isOpened = true;
            windows[currentWindow].x = x0;
            windows[currentWindow].y = y0;
            windows[currentWindow].width = x1 - x0;
            windows[currentWindow].height = y1 - y0;
            windows[currentWindow].depth = color;
            windows[currentWindow].numberColors = 0;
            windows[currentWindow].colors = NULL;
            break;
       case OSD_Show:
            //unused, not supported
            break;
       case OSD_Hide:
            //unused, not supported
            break;
       case OSD_Clear:
            //dsyslog("OSD_Clear\n");
            if (windows[currentWindow].isOpened)
               fb_Clear( windows[currentWindow].x, windows[currentWindow].y, windows[currentWindow].width, windows[currentWindow].height);
            break;
       case OSD_Fill:
            //unused, not supported
            break;
       case OSD_SetColor:
            //unused, not supported
            break;
       case OSD_SetPalette:
            //dsyslog("OSD_SetPalette\n");
            windows[currentWindow].numberColors = x0;
            if (windows[currentWindow].colors != NULL)
               free(windows[currentWindow].colors);
            windows[currentWindow].colors = (tColor*) malloc(sizeof(tColor) * x0);
            memcpy(windows[currentWindow].colors, data, sizeof(tColor) * x0);
            break;
       case OSD_SetPixel:
            //unused, not supported
            break;
       case OSD_SetRow:
            //unused, not supported
            break;
       case OSD_SetBlock:
            //dsyslog("OSD_SetBlock %d\n", color);
            //fb_DrawBlock(data, windows[currentWindow].depth, x0, y0, x1 - x0, y1 - y0, color);
            fb_DrawBlock(data, windows[currentWindow].depth, windows[currentWindow].x + x0, windows[currentWindow].y + y0, x1 - x0, y1 - y0, color);
            break;
       case OSD_FillRow:
            //unused, not supported
            break;
       case OSD_FillBlock:
            //unused, not supported
            break;
       case OSD_Line:
            //unused, not supported
            break;
       case OSD_Text:
            //unused, not supported
            break;
       case OSD_SetWindow:
            //dsyslog("OSD_SetWindow %d\n", x0);
            currentWindow = x0 - 1;
            break;
       case OSD_MoveWindow:
            //dsyslog("OSD_MoveWindow %d %d\n", x0, y0);
            break;
       case OSD_OpenRaw:
            //unused, not supported
            break;
       default:
            dsyslog("switch on unknown cmd (%d)\n", cmd);
            break;
       }
     }
#endif
}

void cDvbFbOsd::Flush(void)
{
  if (!Active())
     return;
  cBitmap *Bitmap;
  for (int i = 0; (Bitmap = GetBitmap(i)) != NULL; i++) {
      Cmd(OSD_SetWindow, 0, i + 1);
      if (!shown)
         Cmd(OSD_Open, Bitmap->Bpp(), Left() + Bitmap->X0(), Top() + Bitmap->Y0(), Left() + Bitmap->X0() + Bitmap->Width() - 1, Top() + Bitmap->Y0() + Bitmap->Height() - 1, (void *)1); // initially hidden!
      int x1 = 0, y1 = 0, x2 = 0, y2 = 0;
      if (!shown || Bitmap->Dirty(x1, y1, x2, y2)) {
         if (!shown) {
            x1 = y1 = 0;
            x2 = Bitmap->Width() - 1;
            y2 = Bitmap->Height() - 1;
            }
         //TODO Workaround: apparently the bitmap sent to the driver always has to be a multiple
         //TODO of 8 bits wide, and (dx * dy) also has to be a multiple of 8.
         //TODO Fix driver (should be able to handle any size bitmaps!)
         while ((x1 > 0 || x2 < Bitmap->Width() - 1) && ((x2 - x1) & 7) != 7) {
               if (x2 < Bitmap->Width() - 1)
                  x2++;
               else if (x1 > 0)
                  x1--;
               }
         //TODO "... / 2" <==> Bpp???
         while ((y1 > 0 || y2 < Bitmap->Height() - 1) && (((x2 - x1 + 1) * (y2 - y1 + 1) / 2) & 7) != 0) {
               if (y2 < Bitmap->Height() - 1)
                  y2++;
               else if (y1 > 0)
                  y1--;
               }
         while ((x1 > 0 || x2 < Bitmap->Width() - 1) && (((x2 - x1 + 1) * (y2 - y1 + 1) / 2) & 7) != 0) {
               if (x2 < Bitmap->Width() - 1)
                  x2++;
               else if (x1 > 0)
                  x1--;
               }
         // commit colors:
         int NumColors;
         const tColor *Colors = Bitmap->Colors(NumColors);
         if (Colors) {
            /* kiowa
            //TODO this should be fixed in the driver!
            tColor colors[NumColors];
            for (int i = 0; i < NumColors; i++) {
                // convert AARRGGBB to AABBGGRR (the driver expects the colors the wrong way):
                colors[i] = (Colors[i] & 0xFF000000) | ((Colors[i] & 0x0000FF) << 16) | (Colors[i] & 0x00FF00) | ((Colors[i] & 0xFF0000) >> 16);
                }
            Colors = colors;
            //TODO end of stuff that should be fixed in the driver
             */
            Cmd(OSD_SetPalette, 0, NumColors - 1, 0, 0, 0, Colors);
            }
         // commit modified data:
#ifdef PLATFORM_STB71XX
         dsyslog("%-14s: %4d, %4d, %4d, %4d, %4d, %4d\n", "Flush", Left(), Top(), x1, y1, x2, y2);
         Cmd(OSD_SetBlock, Bitmap->Width(), Left() + x1, Top() + y1, Left() + x2, Top() + y2, Bitmap->Data(x1, y1));
#else
         Cmd(OSD_SetBlock, Bitmap->Width(), x1, y1, x2, y2, Bitmap->Data(x1, y1));
#endif
         }
      Bitmap->Clean();
      }
  if (!shown) {
     // Showing the windows in a separate loop to avoid seeing them come up one after another
#ifdef PLATFORM_STB71XX
/* Dagobert: we set it directly otherwise we must
 * hold the data in an extra buffer ...
 */
#else
     for (int i = 0; (Bitmap = GetBitmap(i)) != NULL; i++) {
         Cmd(OSD_SetWindow, 0, i + 1);
         Cmd(OSD_MoveWindow, 0, Left() + Bitmap->X0(), Top() + Bitmap->Y0());
         }
#endif
#ifdef USE_YAEPGHD
     if (vidWin.bpp != 0) {
        Cmd(OSD_SetWindow, 0, 5);
        Cmd(OSD_OpenRaw, vidWin.bpp, vidWin.x1, vidWin.y1, vidWin.x2, vidWin.y2, NULL);
        }
#endif
     shown = true;
     }
}

// --- cDvbOsdProvider -------------------------------------------------------

cDvbOsdProvider::cDvbOsdProvider(int OsdDev)
{
  osdDev = OsdDev;
}

cOsd *cDvbOsdProvider::CreateOsd(int Left, int Top, uint Level)
{
  return new cDvbFbOsd(Left, Top, osdDev, Level);
}
