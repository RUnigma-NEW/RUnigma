#include <global.h>
#include <neutrino.h>
#include "pictureviewer.h"
#include "config.h"
#include "fb_display.h"
#include "pv_config.h"
#include "driver/framebuffer.h"


#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#ifdef __sh__
#include <bpamem.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#endif

#ifndef __sh__
/* resize.cpp */
extern unsigned char *simple_resize (unsigned char *orgin, int ox, int oy, int dx, int dy);
extern unsigned char *color_average_resize (unsigned char *orgin, int ox, int oy, int dx, int dy);
#endif


#ifdef FBV_SUPPORT_GIF
extern int fh_gif_getsize (const char *, int *, int *, int, int);
extern int fh_gif_load (const char *, unsigned char **, int *, int *);
extern int fh_gif_id (const char *);
#endif
#ifdef FBV_SUPPORT_JPEG
extern int fh_jpeg_getsize (const char *, int *, int *, int, int);
extern int fh_jpeg_load (const char *, unsigned char **, int *, int *);
extern int fh_jpeg_id (const char *);
#endif
#ifdef FBV_SUPPORT_PNG
extern int fh_png_getsize (const char *, int *, int *, int, int);
extern int fh_png_load (const char *, unsigned char **, int *, int *);
extern int fh_png_id (const char *);
#endif
#ifdef FBV_SUPPORT_BMP
extern int fh_bmp_getsize (const char *, int *, int *, int, int);
extern int fh_bmp_load (const char *, unsigned char **, int *, int *);
extern int fh_bmp_id (const char *);
#endif
#ifdef FBV_SUPPORT_CRW
extern int fh_crw_getsize (const char *, int *, int *, int, int);
extern int fh_crw_load (const char *, unsigned char **, int *, int *);
extern int fh_crw_id (const char *);
#endif

double CPictureViewer::m_aspect_ratio_correction;


#ifdef __sh__
// TODO: fix neutrino code when there's not enough mem for resizing
int hw_resize(int *hwdev, char **hwbuffer, int original_width, int original_height, int height, int width)
{
	int fd_bpa;
	char *dest;
	BPAMemAllocMemData bpa_data;
	int res;
	char bpa_mem_device[30];
	
	fd_bpa = open("/dev/bpamem0", O_RDWR);
	
	if(fd_bpa < 0)
	{
		printf("cannot access /dev/bpamem0! err = %d\n", fd_bpa);
		return -1;
	}
	
	bpa_data.bpa_part = "LMI_VID";
	bpa_data.mem_size = width * height * 3;
	res = ioctl(fd_bpa, BPAMEMIO_ALLOCMEM, &bpa_data); // request memory from bpamem
	if(res)
	{
		printf("cannot alloc required mem\n");
		return -1;
	}
	
	sprintf(bpa_mem_device, "/dev/bpamem%d", bpa_data.device_num);
	close(fd_bpa);
	
	fd_bpa = open(bpa_mem_device, O_RDWR);
	
	// if somebody forgot to add all bpamem devs then this gets really bad here
	if(fd_bpa < 0)
	{
		printf("cannot access %s! err = %d\n", bpa_mem_device, fd_bpa);
		return -1;
	}
	
	dest = (char *)mmap(0, bpa_data.mem_size, PROT_WRITE|PROT_READ, MAP_SHARED, fd_bpa, 0);
	
	if(dest == MAP_FAILED) 
	{
		printf("could not map bpa mem\n");
		ioctl(fd_bpa, BPAMEMIO_FREEMEM);
		close(fd_bpa);
		return -1;
	}
	CFrameBuffer::getInstance()->blitRGBtoRGB(original_width, original_height, width, height, *hwbuffer, dest);
	
	munmap(*hwbuffer, original_width * original_height * 3);
	ioctl(*hwdev, BPAMEMIO_FREEMEM);
	close(*hwdev);
	*hwdev = fd_bpa;
	*hwbuffer = dest;
	return 0;
	
	
}
#endif


void CPictureViewer::add_format (int (*picsize) (const char *, int *, int *, int, int), int (*picread) (const char *, unsigned char **, int *, int *), int (*id) (const char *))
{
  CFormathandler *fhn;
  fhn = (CFormathandler *) malloc (sizeof (CFormathandler));
  fhn->get_size = picsize;
  fhn->get_pic = picread;
  fhn->id_pic = id;
  fhn->next = fh_root;
  fh_root = fhn;
}

void CPictureViewer::init_handlers (void)
{
#ifdef FBV_SUPPORT_GIF
  add_format (fh_gif_getsize, fh_gif_load, fh_gif_id);
#endif
#ifdef FBV_SUPPORT_JPEG
  add_format (fh_jpeg_getsize, fh_jpeg_load, fh_jpeg_id);
#endif
#ifdef FBV_SUPPORT_PNG
  add_format (fh_png_getsize, fh_png_load, fh_png_id);
#endif
#ifdef FBV_SUPPORT_BMP
  add_format (fh_bmp_getsize, fh_bmp_load, fh_bmp_id);
#endif
#ifdef FBV_SUPPORT_CRW
  add_format (fh_crw_getsize, fh_crw_load, fh_crw_id);
#endif
}

CPictureViewer::CFormathandler * CPictureViewer::fh_getsize (const char *name, int *x, int *y, int width_wanted, int height_wanted)
{
  CFormathandler *fh;
  for (fh = fh_root; fh != NULL; fh = fh->next) {
	if (fh->id_pic (name))
	  if (fh->get_size (name, x, y, width_wanted, height_wanted) == FH_ERROR_OK)
		return (fh);
  }
  return (NULL);
}

bool CPictureViewer::DecodeImage (const std::string & name, bool showBusySign, bool unscaled)
{
 //dbout("DecodeImage {\n"); 
  if (name == m_NextPic_Name) {
//      dbout("DecodeImage }\n"); 
	return true;
  }

  int x, y, xs, ys, imx, imy;
  getCurrentRes (&xs, &ys);

  // Show red block for "next ready" in view state
  if (showBusySign)
	showBusy (m_startx + 3, m_starty + 3, 10, 0xff, 00, 00);

  CFormathandler *fh;
  if (unscaled)
	fh = fh_getsize (name.c_str (), &x, &y, INT_MAX, INT_MAX);
  else
	fh = fh_getsize (name.c_str (), &x, &y, m_endx - m_startx, m_endy - m_starty);
	
  if (fh) {
#ifndef __sh__
	if (m_NextPic_Buffer != NULL) {
	  free (m_NextPic_Buffer);
	}

	m_NextPic_Buffer = (unsigned char *) malloc (x * y * 3);
	if (m_NextPic_Buffer == NULL) {
	  printf ("Error: malloc\n");
	  return false;
	}
#else
	if (m_NextPic_Buffer != NULL) 	// Alloc video mem instead of system mem
	{
		munmap(m_NextPic_Buffer, m_NextPic_X * m_NextPic_Y * 3);
		ioctl(m_NextPic_HwDev, BPAMEMIO_FREEMEM);
		close(m_NextPic_HwDev);
	}
	
	m_NextPic_HwDev = open("/dev/bpamem0", O_RDWR);
	
	if(m_NextPic_HwDev < 0)
	{
		printf("cannot access /dev/bpamem0! err = %d\n", m_NextPic_HwDev);
		return false;
	}
	BPAMemAllocMemData bpa_data;
	bpa_data.bpa_part = "LMI_VID";
	bpa_data.mem_size = x * y * 3;
	int res;
	res = ioctl(m_NextPic_HwDev, BPAMEMIO_ALLOCMEM, &bpa_data); // request memory from bpamem
	if(res)
	{
		printf("cannot alloc required bpa mem for image\n");
		close(m_NextPic_HwDev);
		return false;
	}
	
	char bpa_mem_device[30];
	
	sprintf(bpa_mem_device, "/dev/bpamem%d", bpa_data.device_num);
	close(m_NextPic_HwDev);
	
	m_NextPic_HwDev = open(bpa_mem_device, O_RDWR);
	
	if(m_NextPic_HwDev < 0)
	{
		printf("cannot access %s! err = %d\n", bpa_mem_device, m_NextPic_HwDev);
		return false;
	}
	
	m_NextPic_Buffer = (unsigned char*)mmap(0, bpa_data.mem_size, PROT_WRITE|PROT_READ, MAP_SHARED, m_NextPic_HwDev, 0);
	
	if(m_NextPic_Buffer == MAP_FAILED) 
	{
		printf("could not map bpa mem\n");
		ioctl(m_NextPic_HwDev, BPAMEMIO_FREEMEM);
		close(m_NextPic_HwDev);
		return false;
	}
#endif
     // dbout("---Decoding Start(%d/%d)\n",x,y);
	if (fh->get_pic (name.c_str (), &m_NextPic_Buffer, &x, &y) == FH_ERROR_OK) {
#ifdef __sh__
	// the blitter needs to access the memory - flush cache
	msync(m_NextPic_Buffer, x * y * 3, MS_SYNC);
#endif
      // dbout("---Decoding Done\n");
	  if ((x > (m_endx - m_startx) || y > (m_endy - m_starty)) && m_scaling != NONE && !unscaled) {
		if ((m_aspect_ratio_correction * y * (m_endx - m_startx) / x) <= (m_endy - m_starty)) {
		  imx = (m_endx - m_startx);
		  imy = (int) (m_aspect_ratio_correction * y * (m_endx - m_startx) / x);
		} else {
		  imx = (int) ((1.0 / m_aspect_ratio_correction) * x * (m_endy - m_starty) / y);
		  imy = (m_endy - m_starty);
		}
#ifdef __sh__
		hw_resize (&m_NextPic_HwDev, (char **)&m_NextPic_Buffer, x, y, imx, imy);
#else
		if (m_scaling == SIMPLE)
		  m_NextPic_Buffer = simple_resize (m_NextPic_Buffer, x, y, imx, imy);
		else
		  m_NextPic_Buffer = color_average_resize (m_NextPic_Buffer, x, y, imx, imy);
#endif
		x = imx;
		y = imy;
	  }
	  m_NextPic_X = x;
	  m_NextPic_Y = y;

	 
	  if (x < (m_endx - m_startx))
		m_NextPic_XPos = (m_endx - m_startx - x) / 2 + m_startx;
	  else
		m_NextPic_XPos = m_startx;
	  if (y < (m_endy - m_starty))
		m_NextPic_YPos = (m_endy - m_starty - y) / 2 + m_starty;
	  else
		m_NextPic_YPos = m_starty;
	  if (x > (m_endx - m_startx))
		m_NextPic_XPan = (x - (m_endx - m_startx)) / 2;
	  else
		m_NextPic_XPan = 0;
	  if (y > (m_endy - m_starty))
		m_NextPic_YPan = (y - (m_endy - m_starty)) / 2;
	  else
		m_NextPic_YPan = 0;
	} else {
	  printf ("Unable to read file !\n");
#ifdef __sh__
	munmap(m_NextPic_Buffer, m_NextPic_X * m_NextPic_Y * 3);
	ioctl(m_NextPic_HwDev, BPAMEMIO_FREEMEM);
	close(m_NextPic_HwDev);
	return false;	// TODO: Find out why this bad hack is down there
#else
	  free (m_NextPic_Buffer);
	  m_NextPic_Buffer = (unsigned char *) malloc (3);
	  if (m_NextPic_Buffer == NULL) {
		printf ("Error: malloc\n");
		return false;
	  }
	  memset (m_NextPic_Buffer, 0, 3);

	  m_NextPic_X = 1;
	  m_NextPic_Y = 1;
	  m_NextPic_XPos = 0;
	  m_NextPic_YPos = 0;
	  m_NextPic_XPan = 0;
	  m_NextPic_YPan = 0;
#endif
	}
  } else {
	printf ("Unable to read file or format not recognized!\n");
#ifdef __sh__
	munmap(m_NextPic_Buffer, m_NextPic_X * m_NextPic_Y * 3);
	ioctl(m_NextPic_HwDev, BPAMEMIO_FREEMEM);
	close(m_NextPic_HwDev);
	return false;	// TODO: Find out why this bad hack is down there
#else
	if (m_NextPic_Buffer != NULL) {
	  free (m_NextPic_Buffer);
	}
	m_NextPic_Buffer = (unsigned char *) malloc (3);
	if (m_NextPic_Buffer == NULL) {
	  printf ("Error: malloc\n");
	  return false;
	}
	memset (m_NextPic_Buffer, 0, 3);
	m_NextPic_X = 1;
	m_NextPic_Y = 1;
	m_NextPic_XPos = 0;
	m_NextPic_YPos = 0;
	m_NextPic_XPan = 0;
	m_NextPic_YPan = 0;
#endif
  }
  m_NextPic_Name = name;
  hideBusy ();

  //   dbout("DecodeImage }\n"); 
  return (m_NextPic_Buffer != NULL);
}

void CPictureViewer::SetVisible (int startx, int endx, int starty, int endy)
{
#ifdef __sh__
  m_startx = CFrameBuffer::getInstance()->scaleX(startx);
  m_endx = CFrameBuffer::getInstance()->scaleX(endx);
  m_starty = CFrameBuffer::getInstance()->scaleY(starty);
  m_endy = CFrameBuffer::getInstance()->scaleY(endy);
#else
  m_startx = startx;
  m_endx = endx;
  m_starty = starty;
  m_endy = endy;
#endif
}


bool CPictureViewer::ShowImage (const std::string & filename, bool unscaled)
{
//  dbout("Show Image {\n");
  // Wird eh ueberschrieben ,also schonmal freigeben... (wenig speicher)
  if (m_CurrentPic_Buffer != NULL) {
#ifdef __sh__
	munmap(m_CurrentPic_Buffer, m_CurrentPic_X * m_CurrentPic_Y * 3);
	ioctl(m_CurrentPic_HwDev, BPAMEMIO_FREEMEM);
	close(m_CurrentPic_HwDev);
#else
	free (m_CurrentPic_Buffer);
#endif
	m_CurrentPic_Buffer = NULL;
  }
  DecodeImage (filename, true, unscaled);
  DisplayNextImage ();
//  dbout("Show Image }\n");
  return true;
}

bool CPictureViewer::DisplayNextImage ()
{
//  dbout("DisplayNextImage {\n");
  if (m_CurrentPic_Buffer != NULL) {
#ifdef __sh__
	munmap(m_CurrentPic_Buffer, m_CurrentPic_X * m_CurrentPic_Y * 3);
	ioctl(m_CurrentPic_HwDev, BPAMEMIO_FREEMEM);
	close(m_CurrentPic_HwDev);
#else
	free (m_CurrentPic_Buffer);
#endif
	m_CurrentPic_Buffer = NULL;
  }
  if (m_NextPic_Buffer != NULL)
	fb_display (m_NextPic_Buffer, m_NextPic_X, m_NextPic_Y, m_NextPic_XPan, m_NextPic_YPan, m_NextPic_XPos, m_NextPic_YPos);
//  dbout("DisplayNextImage fb_disp done\n");
  m_CurrentPic_Buffer = m_NextPic_Buffer;
  m_NextPic_Buffer = NULL;
  m_CurrentPic_Name = m_NextPic_Name;
  m_CurrentPic_X = m_NextPic_X;
  m_CurrentPic_Y = m_NextPic_Y;
  m_CurrentPic_XPos = m_NextPic_XPos;
  m_CurrentPic_YPos = m_NextPic_YPos;
  m_CurrentPic_XPan = m_NextPic_XPan;
  m_CurrentPic_YPan = m_NextPic_YPan;
#ifdef __sh__
  m_CurrentPic_HwDev = m_NextPic_HwDev;
#endif
//  dbout("DisplayNextImage }\n");
  return true;
}

void CPictureViewer::Zoom (float factor)
{
//  dbout("Zoom %f\n",factor);
  showBusy (m_startx + 3, m_starty + 3, 10, 0xff, 0xff, 00);

  int oldx = m_CurrentPic_X;
  int oldy = m_CurrentPic_Y;
  unsigned char *oldBuf = m_CurrentPic_Buffer;
  m_CurrentPic_X = (int) (factor * m_CurrentPic_X);
  m_CurrentPic_Y = (int) (factor * m_CurrentPic_Y);

#ifdef __sh__
		hw_resize (&m_NextPic_HwDev, (char **)&m_NextPic_Buffer, oldx, oldy, m_CurrentPic_X, m_CurrentPic_Y);
#else
  if (m_scaling == COLOR)
	m_CurrentPic_Buffer = color_average_resize (m_CurrentPic_Buffer, oldx, oldy, m_CurrentPic_X, m_CurrentPic_Y);
  else
	m_CurrentPic_Buffer = simple_resize (m_CurrentPic_Buffer, oldx, oldy, m_CurrentPic_X, m_CurrentPic_Y);
#endif

  if (m_CurrentPic_Buffer == oldBuf) {
	// resize failed
	hideBusy ();
	return;
  }

  if (m_CurrentPic_X < (m_endx - m_startx))
	m_CurrentPic_XPos = (m_endx - m_startx - m_CurrentPic_X) / 2 + m_startx;
  else
	m_CurrentPic_XPos = m_startx;
  if (m_CurrentPic_Y < (m_endy - m_starty))
	m_CurrentPic_YPos = (m_endy - m_starty - m_CurrentPic_Y) / 2 + m_starty;
  else
	m_CurrentPic_YPos = m_starty;
  if (m_CurrentPic_X > (m_endx - m_startx))
	m_CurrentPic_XPan = (m_CurrentPic_X - (m_endx - m_startx)) / 2;
  else
	m_CurrentPic_XPan = 0;
  if (m_CurrentPic_Y > (m_endy - m_starty))
	m_CurrentPic_YPan = (m_CurrentPic_Y - (m_endy - m_starty)) / 2;
  else
	m_CurrentPic_YPan = 0;
  fb_display (m_CurrentPic_Buffer, m_CurrentPic_X, m_CurrentPic_Y, m_CurrentPic_XPan, m_CurrentPic_YPan, m_CurrentPic_XPos, m_CurrentPic_YPos);
}

void CPictureViewer::Move (int dx, int dy)
{
//  dbout("Move %d %d\n",dx,dy);
  showBusy (m_startx + 3, m_starty + 3, 10, 0x00, 0xff, 00);

  int xs, ys;
  getCurrentRes (&xs, &ys);
  m_CurrentPic_XPan += dx;
  if (m_CurrentPic_XPan + xs >= m_CurrentPic_X)
	m_CurrentPic_XPan = m_CurrentPic_X - xs - 1;
  if (m_CurrentPic_XPan < 0)
	m_CurrentPic_XPan = 0;

  m_CurrentPic_YPan += dy;
  if (m_CurrentPic_YPan + ys >= m_CurrentPic_Y)
	m_CurrentPic_YPan = m_CurrentPic_Y - ys - 1;
  if (m_CurrentPic_YPan < 0)
	m_CurrentPic_YPan = 0;

  if (m_CurrentPic_X < (m_endx - m_startx))
	m_CurrentPic_XPos = (m_endx - m_startx - m_CurrentPic_X) / 2 + m_startx;
  else
	m_CurrentPic_XPos = m_startx;
  if (m_CurrentPic_Y < (m_endy - m_starty))
	m_CurrentPic_YPos = (m_endy - m_starty - m_CurrentPic_Y) / 2 + m_starty;
  else
	m_CurrentPic_YPos = m_starty;
//  dbout("Display x(%d) y(%d) xpan(%d) ypan(%d) xpos(%d) ypos(%d)\n",m_CurrentPic_X, m_CurrentPic_Y, 
//          m_CurrentPic_XPan, m_CurrentPic_YPan, m_CurrentPic_XPos, m_CurrentPic_YPos);

  fb_display (m_CurrentPic_Buffer, m_CurrentPic_X, m_CurrentPic_Y, m_CurrentPic_XPan, m_CurrentPic_YPan, m_CurrentPic_XPos, m_CurrentPic_YPos);
}

CPictureViewer::CPictureViewer ()
{
  fh_root = NULL;
  m_scaling = COLOR;
  //m_aspect = 4.0 / 3;
  m_aspect = 16.0 / 9;
  m_CurrentPic_Name = "";
  m_CurrentPic_Buffer = NULL;
  m_CurrentPic_X = 0;
  m_CurrentPic_Y = 0;
  m_CurrentPic_XPos = 0;
  m_CurrentPic_YPos = 0;
  m_CurrentPic_XPan = 0;
  m_CurrentPic_YPan = 0;
  m_NextPic_Name = "";
  m_NextPic_Buffer = NULL;
  m_NextPic_X = 0;
  m_NextPic_Y = 0;
  m_NextPic_XPos = 0;
  m_NextPic_YPos = 0;
  m_NextPic_XPan = 0;
  m_NextPic_YPan = 0;
  int xs, ys;
  getCurrentRes (&xs, &ys);
  m_startx = 0;
  m_endx = xs - 1;
  m_starty = 0;
  m_endy = ys - 1;
  m_aspect_ratio_correction = m_aspect / ((double) xs / ys);

  m_busy_buffer = NULL;

  init_handlers ();
}

void CPictureViewer::showBusy (int sx, int sy, int width, char r, char g, char b)
{
//  dbout("Show Busy{\n");
  unsigned char rgb_buffer[3];
  unsigned char *fb_buffer;
  unsigned char *busy_buffer_wrk;
  int cpp;
  struct fb_var_screeninfo *var;
  var = CFrameBuffer::getInstance ()->getScreenInfo ();

  rgb_buffer[0] = r;
  rgb_buffer[1] = g;
  rgb_buffer[2] = b;

  fb_buffer = (unsigned char *) convertRGB2FB (rgb_buffer, 1, 1, var->bits_per_pixel, &cpp);
  if (fb_buffer == NULL) {
	printf ("Error: malloc\n");
	return;
  }
  if (m_busy_buffer != NULL) {
	free (m_busy_buffer);
	m_busy_buffer = NULL;
  }
  m_busy_buffer = (unsigned char *) malloc (width * width * cpp);
  if (m_busy_buffer == NULL) {
	printf ("Error: malloc\n");
	return;
  }
  busy_buffer_wrk = m_busy_buffer;
  unsigned char *fb = (unsigned char *) CFrameBuffer::getInstance ()->getFrameBufferPointer ();
  unsigned int stride = CFrameBuffer::getInstance ()->getStride ();

  for (int y = sy; y < sy + width; y++) {
	for (int x = sx; x < sx + width; x++) {
	  memcpy (busy_buffer_wrk, fb + y * stride + x * cpp, cpp);
	  busy_buffer_wrk += cpp;
	  memcpy (fb + y * stride + x * cpp, fb_buffer, cpp);
	}
  }
  m_busy_x = sx;
  m_busy_y = sy;
  m_busy_width = width;
  m_busy_cpp = cpp;
  free (fb_buffer);

//  dbout("Show Busy}\n");
}

void CPictureViewer::hideBusy ()
{
//  dbout("Hide Busy{\n");
  if (m_busy_buffer != NULL) {
	unsigned char *fb = (unsigned char *) CFrameBuffer::getInstance ()->getFrameBufferPointer ();
	unsigned int stride = CFrameBuffer::getInstance ()->getStride ();
	unsigned char *busy_buffer_wrk = m_busy_buffer;

	for (int y = m_busy_y; y < m_busy_y + m_busy_width; y++) {
	  for (int x = m_busy_x; x < m_busy_x + m_busy_width; x++) {
		memcpy (fb + y * stride + x * m_busy_cpp, busy_buffer_wrk, m_busy_cpp);
		busy_buffer_wrk += m_busy_cpp;
	  }
	}
	free (m_busy_buffer);
	m_busy_buffer = NULL;
  }
//  dbout("Hide Busy}\n");
}
void CPictureViewer::Cleanup ()
{
  if (m_busy_buffer != NULL) {
	free (m_busy_buffer);
	m_busy_buffer = NULL;
  }
  if (m_NextPic_Buffer != NULL) {
#ifdef __sh__
	munmap(m_NextPic_Buffer, m_NextPic_X * m_NextPic_Y * 3);
	ioctl(m_NextPic_HwDev, BPAMEMIO_FREEMEM);
	close(m_NextPic_HwDev);
#else
	free (m_NextPic_Buffer);
#endif
	m_NextPic_Buffer = NULL;
  }
  if (m_CurrentPic_Buffer != NULL) {
#ifdef __sh__
	munmap(m_CurrentPic_Buffer, m_CurrentPic_X * m_CurrentPic_Y * 3);
	ioctl(m_CurrentPic_HwDev, BPAMEMIO_FREEMEM);
	close(m_CurrentPic_HwDev);
#else
	free (m_CurrentPic_Buffer);
#endif
	m_CurrentPic_Buffer = NULL;
  }
}
#define LOGO_DIR1 "/share/tuxbox/neutrino/icons/logo"
#define LOGO_DIR2 "/var/share/icons/logo"
#define LOGO_FMT ".jpg"

bool CPictureViewer::DisplayLogo (uint64_t channel_id, int posx, int posy, int width, int height)
{
        char fname[255];
	bool ret = false;
        sprintf(fname, "%s/%llx.jpg", LOGO_DIR2, channel_id & 0xFFFFFFFFFFFFULL);
printf("logo file: %s\n", fname);
        if(access(fname, F_OK))
                sprintf(fname, "%s/%llx.gif", LOGO_DIR2, channel_id & 0xFFFFFFFFFFFFULL);
        if(!access(fname, F_OK)) {
                ret = DisplayImage(fname, posx, posy, width, height);
        }
	return ret;
}

bool CPictureViewer::DisplayImage (const std::string & name, int posx, int posy, int width, int height)
{
  int x, y;
  CFormathandler *fh;
  bool ret = false;
#ifdef __sh__
  posx = CFrameBuffer::getInstance()->scaleX(posx);
  posy = CFrameBuffer::getInstance()->scaleY(posy);
  width = CFrameBuffer::getInstance()->scaleX(width);
  height = CFrameBuffer::getInstance()->scaleY(height);
#endif

  if (m_NextPic_Name != name || m_NextPic_X != width || m_NextPic_Y != height) {

  	//getCurrentRes (&xs, &ys);

  	fh = fh_getsize (name.c_str (), &x, &y, INT_MAX, INT_MAX);
  	if (fh) {
#ifdef __sh__
		if (m_NextPic_Buffer != NULL) 
		{
			munmap(m_NextPic_Buffer, m_NextPic_X * m_NextPic_Y * 3);
			ioctl(m_NextPic_HwDev, BPAMEMIO_FREEMEM);
			close(m_NextPic_HwDev);
		}

		m_NextPic_HwDev = open("/dev/bpamem0", O_RDWR);

		if(m_NextPic_HwDev < 0)
		{
			printf("cannot access /dev/bpamem0! err = %d\n", m_NextPic_HwDev);
			return false;
		}
		BPAMemAllocMemData bpa_data;
		bpa_data.bpa_part = "LMI_VID";
		bpa_data.mem_size = x * y * 3;
		int res;
		res = ioctl(m_NextPic_HwDev, BPAMEMIO_ALLOCMEM, &bpa_data); // request memory from bpamem
		if(res)
		{
			printf("cannot alloc required bpa mem for image\n");
			close(m_NextPic_HwDev);
			return false;
		}

		char bpa_mem_device[30];

		sprintf(bpa_mem_device, "/dev/bpamem%d", bpa_data.device_num);
		close(m_NextPic_HwDev);

		m_NextPic_HwDev = open(bpa_mem_device, O_RDWR);

		if(m_NextPic_HwDev < 0)
		{
			printf("cannot access %s! err = %d\n", bpa_mem_device, m_NextPic_HwDev);
			return false;
		}

		m_NextPic_Buffer = (unsigned char *)mmap(0, bpa_data.mem_size, PROT_WRITE|PROT_READ, MAP_SHARED, m_NextPic_HwDev, 0);

		if(m_NextPic_Buffer == MAP_FAILED) 
		{
			printf("could not map bpa mem\n");
			ioctl(m_NextPic_HwDev, BPAMEMIO_FREEMEM);
			close(m_NextPic_HwDev);
			return false;
		}
#else
		if (m_NextPic_Buffer != NULL)
	  		free (m_NextPic_Buffer);

		m_NextPic_Buffer = (unsigned char *) malloc (x * y * 3);
		if (m_NextPic_Buffer == NULL) {
		  	printf ("Error: malloc\n");
		  	return false;
		}
#endif
		if (fh->get_pic (name.c_str (), &m_NextPic_Buffer, &x, &y) == FH_ERROR_OK) {
#ifdef __sh__
			// the blitter needs to access the memory - flush cache
			msync(m_NextPic_Buffer, x * y * 3, MS_SYNC);
			// the user always expects a fixed fb size so scale the image if no sizes are supplied
			if(!width)
				width = CFrameBuffer::getInstance()->scaleX(x);
			if(!height)
				height = CFrameBuffer::getInstance()->scaleY(y);
#endif
//printf("DisplayImage: decoded %s, %d x %d to x=%d y=%d\n", name.c_str (), x, y, posx, posy);
			//FIXME m_aspect_ratio_correction ?
			if(width && height) {
				if(x != width || y != height) {
#ifdef __sh__
					hw_resize (&m_NextPic_HwDev, (char **)&m_NextPic_Buffer, x, y, width, height);
#else
					//m_NextPic_Buffer = simple_resize (m_NextPic_Buffer, x, y, imx, imy);
					m_NextPic_Buffer = color_average_resize (m_NextPic_Buffer, x, y, width, height);
#endif
					x = width;
					y = height;
				}
				posx += (width-x)/2;
				posy += (height-y)/2;
			} 
//printf("DisplayImage: display %s, %d x %d to x=%d y=%d\n", name.c_str (), x, y, posx, posy);
			fb_display (m_NextPic_Buffer, x, y, 0, 0, posx, posy, false, convertSetupAlpha2Alpha(g_settings.infobar_alpha));
			m_NextPic_X = x;
			m_NextPic_Y = y;
  			m_NextPic_Name = name;
			ret = true;
		} else {
	  		printf ("Error decoding file %s\n", name.c_str ());
	  		free (m_NextPic_Buffer);
	  		m_NextPic_Buffer = NULL;
  	  		m_NextPic_Name = "";
		}
  	} else {
		printf("Error open file %s\n", name.c_str ());
  	}
  } else if(m_NextPic_Buffer) {
	m_NextPic_XPos = posx;
	m_NextPic_YPos = posy;
	fb_display (m_NextPic_Buffer, m_NextPic_X, m_NextPic_Y, 0, 0, m_NextPic_XPos, m_NextPic_YPos, false, convertSetupAlpha2Alpha(g_settings.infobar_alpha));
	ret = true;
  }
  return ret;
}

fb_pixel_t * CPictureViewer::getImage (const std::string & name, int width, int height)
{
#ifdef __sh__
	return NULL; // TODO: is only used for background drawing and expects user space pointer
		     //  --> rewrite background drawing for bpamem pointers
#else
	int x, y;
	CFormathandler *fh;
	unsigned char * buffer;
	fb_pixel_t * ret = NULL;

  	fh = fh_getsize (name.c_str (), &x, &y, INT_MAX, INT_MAX);
  	if (fh) {

		buffer = (unsigned char *) malloc (x * y * 3);
		if (buffer == NULL) {
		  	printf ("Error: malloc\n");
		  	return false;
		}
		if (fh->get_pic (name.c_str (), &buffer, &x, &y) == FH_ERROR_OK) {
#ifdef __sh__
			// the blitter needs to access the memory - flush cache
			msync(buffer, x * y * 3, MS_SYNC);
#endif
printf("getImage: decoded %s, %d x %d \n", name.c_str (), x, y);
			if(x != width || y != height)
			{
		  		buffer = color_average_resize (buffer, x, y, width, height);
				x = width;
				y = height;
			} 
			struct fb_var_screeninfo *var = CFrameBuffer::getInstance()->getScreenInfo();
			int bp;
			ret = (fb_pixel_t *) convertRGB2FB(buffer, x, y, var->bits_per_pixel, &bp, 0);
			free(buffer);
			//fb_display (buffer, x, y, 0, 0, posx, posy, false, convertSetupAlpha2Alpha(g_settings.infobar_alpha));
		} else {
	  		printf ("Error decoding file %s\n", name.c_str ());
	  		free (buffer);
	  		buffer = NULL;
		}
  	} else
		printf("Error open file %s\n", name.c_str ());

	return ret;
#endif
}
