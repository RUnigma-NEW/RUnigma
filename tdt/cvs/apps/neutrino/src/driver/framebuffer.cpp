/*
	Neutrino-GUI  -   DBoxII-Project

	Copyright (C) 2001 Steffen Hehn 'McClean'
                      2003 thegoodguy

	License: GPL

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <driver/framebuffer.h>

#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <memory.h>
#include <math.h>

#include <linux/kd.h>

#include <stdint.h>

#include <gui/color.h>
#include <gui/pictureviewer.h>
#include <global.h>
#include <video_cs.h>

#ifdef __sh__
#include <linux/stmfb.h>
#include <bpamem.h>

#define DEFAULT_XRES 1280
#define DEFAULT_YRES 720

#define ICON_TEMP_SIZE 256	// should be enough for small icons
#endif

extern cVideo * videoDecoder;

extern CPictureViewer * g_PicViewer;
#define BACKGROUNDIMAGEWIDTH 720

//#undef USE_NEVIS_GXA //FIXME
/*******************************************************************************/
#ifdef USE_NEVIS_GXA

#ifdef GXA_FG_COLOR_REG
#undef GXA_FG_COLOR_REG
#endif
#ifdef GXA_BG_COLOR_REG
#undef GXA_BG_COLOR_REG
#endif
#ifdef GXA_LINE_CONTROL_REG
#undef GXA_LINE_CONTROL_REG
#endif
#ifdef GXA_DEPTH_REG
#undef GXA_DEPTH_REG
#endif
#ifdef GXA_CONTENT_ID_REG
#undef GXA_CONTENT_ID_REG
#endif

#define GXA_POINT(x, y)         (((y) & 0x0FFF) << 16) | ((x) & 0x0FFF)
#define GXA_SRC_BMP_SEL(x)      (x << 8)
#define GXA_DST_BMP_SEL(x)      (x << 5)
#define GXA_PARAM_COUNT(x)      (x << 2)

#define GXA_FG_COLOR_REG	0x0020
#define GXA_BG_COLOR_REG        0x0024
#define GXA_LINE_CONTROL_REG    0x0038
#define GXA_BMP2_TYPE_REG       0x0050
#define GXA_BMP2_ADDR_REG       0x0054
#define GXA_DEPTH_REG		0x00F4
#define GXA_CONTENT_ID_REG      0x0144

#define GXA_CMD_BLT             0x00010800
#define GXA_CMD_NOT_ALPHA       0x00011000
#define GXA_CMD_NOT_TEXT        0x00018000

/*
static unsigned int _read_gxa(volatile unsigned char *base_addr, unsigned int offset)
{
    return *(volatile unsigned int *)(base_addr + offset);
}
*/
static void _write_gxa(volatile unsigned char *base_addr, unsigned int offset, unsigned int value)
{
    while( (*(volatile unsigned int *)(base_addr + GXA_DEPTH_REG)) & 0x40000000);
    *(volatile unsigned int *)(base_addr + offset) = value;
}

#endif /* USE_NEVIS_GXA */

#ifdef __sh__
int CFrameBuffer::scaleX(const int x)
{
	int mul = x * xRes;
	mul = mul / DEFAULT_XRES + (((mul % DEFAULT_XRES) >= (DEFAULT_XRES / 2)) ? 1 : 0);
	if(mul > xRes)
		return xRes;
	return mul;
}

int CFrameBuffer::scaleY(const int y)
{
	int mul = y * yRes;
	mul = mul / DEFAULT_YRES + (((mul % DEFAULT_YRES) >= (DEFAULT_YRES / 2)) ? 1 : 0);
	if(mul > yRes)
		return yRes;
	return mul;
}
#endif

/*******************************************************************************/

static uint8_t * virtual_fb = NULL;
inline unsigned int make16color(uint16_t r, uint16_t g, uint16_t b, uint16_t t,
                                  uint32_t  rl = 0, uint32_t  ro = 0,
                                  uint32_t  gl = 0, uint32_t  go = 0,
                                  uint32_t  bl = 0, uint32_t  bo = 0,
                                  uint32_t  tl = 0, uint32_t  to = 0)
{
        return ((t << 24) & 0xFF000000) | ((r << 8) & 0xFF0000) | ((g << 0) & 0xFF00) | (b >> 8 & 0xFF);
}

CFrameBuffer::CFrameBuffer()
: active ( true )
{
	iconBasePath = "";
	available  = 0;
	cmap.start = 0;
	cmap.len = 256;
	cmap.red = red;
	cmap.green = green;
	cmap.blue  = blue;
	cmap.transp = trans;
	backgroundColor = 0;
	useBackgroundPaint = false;
	background = NULL;
	backupBackground = NULL;
	backgroundFilename = "";
	fd  = 0;
	tty = 0;
//FIXME: test
	memset(red, 0, 256*sizeof(__u16));
	memset(green, 0, 256*sizeof(__u16));
	memset(blue, 0, 256*sizeof(__u16));
	memset(trans, 0, 256*sizeof(__u16));
}

CFrameBuffer* CFrameBuffer::getInstance()
{
	static CFrameBuffer* frameBuffer = NULL;

	if(!frameBuffer) {
		frameBuffer = new CFrameBuffer();
		printf("[neutrino] frameBuffer Instance created\n");
	} else {
		//printf("[neutrino] frameBuffer Instace requested\n");
	}
	return frameBuffer;
}

void CFrameBuffer::init(const char * const fbDevice)
{
	fd = open(fbDevice, O_RDWR);
	if(!fd) fd = open(fbDevice, O_RDWR);

	if (fd<0) {
		perror(fbDevice);
		goto nolfb;
	}

	if (ioctl(fd, FBIOGET_VSCREENINFO, &screeninfo)<0) {
		perror("FBIOGET_VSCREENINFO");
		goto nolfb;
	}
#ifdef __sh__
	xRes = screeninfo.xres;
	yRes = screeninfo.yres;
	bpp = 32;
	stride = xRes * bpp / 8;

	screeninfo.xres = DEFAULT_XRES;
	screeninfo.yres = DEFAULT_YRES;
	screeninfo.xres_virtual = DEFAULT_XRES;
	screeninfo.yres_virtual = DEFAULT_YRES;
	screeninfo.bits_per_pixel = 32;
#endif

	memcpy(&oldscreen, &screeninfo, sizeof(screeninfo));

	fb_fix_screeninfo fix;
	if (ioctl(fd, FBIOGET_FSCREENINFO, &fix)<0) {
		perror("FBIOGET_FSCREENINFO");
		goto nolfb;
	}

	available=fix.smem_len;
	printf("%dk video mem\n", available/1024);
	lfb=(fb_pixel_t*)mmap(0, available, PROT_WRITE|PROT_READ, MAP_SHARED, fd, 0);

	if (!lfb) {
		perror("mmap");
		goto nolfb;
	}

#ifdef __sh__
	if (available / (1024 * 1024) < 8)
	{
		printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
		printf("error: to less memory for stmfb given, need at least 8mb\n");  	
	}
	
	fd_bpa = open("/dev/bpamem0", O_RDWR);
	
	if(fd_bpa < 0)
	{
		printf("cannot access /dev/bpamem0! err = %d\n", fd_bpa);
		return;
	}
	
	BPAMemAllocMemData bpa_data;
	bpa_data.bpa_part = "LMI_VID";
	bpa_data.mem_size = ICON_TEMP_SIZE * ICON_TEMP_SIZE * 4;	// should be enough for small icons
	int res;
	res = ioctl(fd_bpa, BPAMEMIO_ALLOCMEM, &bpa_data); // request memory from bpamem
	if(res)
	{
		printf("cannot alloc required bpa mem\n");
		close(fd_bpa);
		return;
	}
	
	char bpa_mem_device[30];
	
	sprintf(bpa_mem_device, "/dev/bpamem%d", bpa_data.device_num);
	close(fd_bpa);
	
	fd_bpa = open(bpa_mem_device, O_RDWR);
	
	if(fd_bpa < 0)
	{
		printf("cannot access %s! err = %d\n", bpa_mem_device, fd_bpa);
		return;
	}
	
	icon_space = (fb_pixel_t *)mmap(0, bpa_data.mem_size, PROT_WRITE|PROT_READ, MAP_SHARED, fd_bpa, 0);
	
	if(icon_space == MAP_FAILED) 
	{
		printf("could not map bpa mem\n");
		ioctl(fd_bpa, BPAMEMIO_FREEMEM);
		close(fd_bpa);
		return;
	}
#endif

#ifdef USE_NEVIS_GXA
	/* Open 7dev/mem for HW-register access */
	devmem_fd = open("/dev/mem", O_RDWR | O_SYNC);
	if (devmem_fd < 0) {
		perror("Unable to open /dev/mem");
		goto nolfb;
	}

	/* mmap the GXA's base address */
	gxa_base = (volatile unsigned char*) mmap(0, 0x00040000, PROT_READ | PROT_WRITE, MAP_SHARED, devmem_fd, 0xE0600000);
	if (gxa_base == (void*) -1){
		perror("Unable to mmap /dev/mem");
		goto nolfb;
	}

	/* tell the GXA where the framebuffer to draw on starts */
	smem_start = (unsigned int) fix.smem_start;
        _write_gxa(gxa_base, GXA_BMP2_TYPE_REG, (3 << 16) | screeninfo.xres);
	_write_gxa(gxa_base, GXA_BMP2_ADDR_REG, (unsigned int) fix.smem_start);
	_write_gxa(gxa_base, GXA_CONTENT_ID_REG, 0);
#endif

#if 0
	if ((tty=open("/dev/vc/0", O_RDWR))<0) {
		perror("open (tty)");
		goto nolfb;
	}

	struct sigaction act;

	memset(&act,0,sizeof(act));
	act.sa_handler  = switch_signal;
	sigemptyset(&act.sa_mask);
	sigaction(SIGUSR1,&act,NULL);
	sigaction(SIGUSR2,&act,NULL);

	struct vt_mode mode;

	if (-1 == ioctl(tty,KDGETMODE, &kd_mode)) {
		perror("ioctl KDGETMODE");
		goto nolfb;
	}

	if (-1 == ioctl(tty,VT_GETMODE, &vt_mode)) {
      		perror("ioctl VT_GETMODE");
		goto nolfb;
	}

	if (-1 == ioctl(tty,VT_GETMODE, &mode)) {
      		perror("ioctl VT_GETMODE");
		goto nolfb;
	}

	mode.mode   = VT_PROCESS;
	mode.waitv  = 0;
	mode.relsig = SIGUSR1;
	mode.acqsig = SIGUSR2;

	if (-1 == ioctl(tty,VT_SETMODE, &mode)) {
		perror("ioctl VT_SETMODE");
		goto nolfb;
	}

	if (-1 == ioctl(tty,KDSETMODE, KD_GRAPHICS)) {
		perror("ioctl KDSETMODE");
		goto nolfb;
	}
#endif

	return;

nolfb:
	printf("framebuffer not available.\n");
	lfb=0;
}


CFrameBuffer::~CFrameBuffer()
{
	if (background) {
		delete[] background;
	}

	if (backupBackground) {
		delete[] backupBackground;
	}

#if 0
#ifdef RETURN_FROM_GRAPHICS_MODE
	if (-1 == ioctl(tty,KDSETMODE, kd_mode))
		perror("ioctl KDSETMODE");
#endif

	if (-1 == ioctl(tty,VT_SETMODE, &vt_mode))
		perror("ioctl VT_SETMODE");

	if (available)
		ioctl(fd, FBIOPUT_VSCREENINFO, &oldscreen);
#endif
	if (lfb)
		munmap(lfb, available);
#ifdef __sh__
	if(icon_space)
		munmap(icon_space, ICON_TEMP_SIZE * ICON_TEMP_SIZE * 4);
	if(fd_bpa)
	{
		ioctl(fd_bpa, BPAMEMIO_FREEMEM);
		close(fd_bpa);
	}
#endif
	
	if (virtual_fb)
		delete[] virtual_fb;
	close(fd);
	close(tty);
}

int CFrameBuffer::getFileHandle() const
{
	return fd;
}

unsigned int CFrameBuffer::getStride() const
{
	return stride;
}

unsigned int CFrameBuffer::getScreenWidth(bool real)
{
	if(real)
#ifdef __sh__
		return DEFAULT_XRES;
#else
		return xRes;
#endif
	else
		return g_settings.screen_EndX - g_settings.screen_StartX;
}

unsigned int CFrameBuffer::getScreenHeight(bool real)
{
	if(real)
#ifdef __sh__
		return DEFAULT_YRES;
#else
		return yRes;
#endif
	else
		return g_settings.screen_EndY - g_settings.screen_StartY;
}

unsigned int CFrameBuffer::getScreenX()
{
	return g_settings.screen_StartX;
}

unsigned int CFrameBuffer::getScreenY()
{
	return g_settings.screen_StartY;
}

fb_pixel_t * CFrameBuffer::getFrameBufferPointer() const
{
	if (active || (virtual_fb == NULL))
		return lfb;
	else
		return (fb_pixel_t *) virtual_fb;
}

bool CFrameBuffer::getActive() const
{
	return (active || (virtual_fb != NULL));
}

void CFrameBuffer::setActive(bool enable)
{
	active = enable;
}

t_fb_var_screeninfo *CFrameBuffer::getScreenInfo()
{
	return &screeninfo;
}

int CFrameBuffer::setMode(unsigned int nxRes, unsigned int nyRes, unsigned int nbpp)
{
	if (!available&&!active)
		return -1;

#if 0
	screeninfo.xres_virtual=screeninfo.xres=nxRes;
	screeninfo.yres_virtual=screeninfo.yres=nyRes;
	screeninfo.height=0;
	screeninfo.width=0;
	screeninfo.xoffset=screeninfo.yoffset=0;
	screeninfo.bits_per_pixel=nbpp;

	if (ioctl(fd, FBIOPUT_VSCREENINFO, &screeninfo)<0) {
		perror("FBIOPUT_VSCREENINFO");
	}

	if(1) {
		printf("SetMode: %dbits, red %d:%d green %d:%d blue %d:%d transp %d:%d\n",
		screeninfo.bits_per_pixel, screeninfo.red.length, screeninfo.red.offset, screeninfo.green.length, screeninfo.green.offset, screeninfo.blue.length, screeninfo.blue.offset, screeninfo.transp.length, screeninfo.transp.offset);
	}
	if ((screeninfo.xres!=nxRes) && (screeninfo.yres!=nyRes) && (screeninfo.bits_per_pixel!=nbpp))
	{
		printf("SetMode failed: wanted: %dx%dx%d, got %dx%dx%d\n",
		       nxRes, nyRes, nbpp,
		       screeninfo.xres, screeninfo.yres, screeninfo.bits_per_pixel);
		return -1;
	}
#endif
#ifndef __sh__
	xRes = screeninfo.xres;
	yRes = screeninfo.yres;
	bpp  = screeninfo.bits_per_pixel;

	fb_fix_screeninfo fix;

	if (ioctl(fd, FBIOGET_FSCREENINFO, &fix)<0) {
		perror("FBIOGET_FSCREENINFO");
		return -1;
	}

	stride = fix.line_length;

printf("FB: %dx%dx%d line lenght %d\n", xRes, yRes, bpp, stride);

	memset(getFrameBufferPointer(), 0, stride * yRes);
        if (ioctl(fd, FBIOBLANK, FB_BLANK_UNBLANK) < 0) {
                printf("screen unblanking failed\n");
        }
#endif

	return 0;
}

void CFrameBuffer::setTransparency( int tr )
{
}

void CFrameBuffer::setBlendLevel(int blev1, int blev2)
{
	unsigned short value = blev1;
#if 0
	if (ioctl(fd, FBIO_CHANGEOPACITY, &value) < 0)
		printf("FBIO_CHANGEOPACITY failed.\n");
#endif
}

void CFrameBuffer::setAlphaFade(int in, int num, int tr)
{
	for (int i=0; i<num; i++) {
		cmap.transp[in+i]=tr;
	}
}

void CFrameBuffer::paletteFade(int i, __u32 rgb1, __u32 rgb2, int level)
{
	__u16 *r = cmap.red+i;
	__u16 *g = cmap.green+i;
	__u16 *b = cmap.blue+i;
	*r= ((rgb2&0xFF0000)>>16)*level;
	*g= ((rgb2&0x00FF00)>>8 )*level;
	*b= ((rgb2&0x0000FF)    )*level;
	*r+=((rgb1&0xFF0000)>>16)*(255-level);
	*g+=((rgb1&0x00FF00)>>8 )*(255-level);
	*b+=((rgb1&0x0000FF)    )*(255-level);
}

void CFrameBuffer::paletteGenFade(int in, __u32 rgb1, __u32 rgb2, int num, int tr)
{
	for (int i=0; i<num; i++) {
		paletteFade(in+i, rgb1, rgb2, i*(255/(num-1)));
		cmap.transp[in+i]=tr;
		tr--; //FIXME
	}
}

void CFrameBuffer::paletteSetColor(int i, __u32 rgb, int tr)
{
	cmap.red[i]    =(rgb&0xFF0000)>>8;
	cmap.green[i]  =(rgb&0x00FF00)   ;
	cmap.blue[i]   =(rgb&0x0000FF)<<8;
	cmap.transp[i] = tr;
}

void CFrameBuffer::paletteSet(struct fb_cmap *map)
{
	if (!active)
		return;
	
	if(map == NULL)
		map = &cmap;

	if(bpp == 8) {
//printf("Set palette for %dbit\n", bpp);
		ioctl(fd, FBIOPUTCMAP, map);
	}
	uint32_t  rl, ro, gl, go, bl, bo, tl, to;
        rl = screeninfo.red.length;
        ro = screeninfo.red.offset;
        gl = screeninfo.green.length;
        go = screeninfo.green.offset;
        bl = screeninfo.blue.length;
        bo = screeninfo.blue.offset;
        tl = screeninfo.transp.length;
        to = screeninfo.transp.offset;
	for (int i = 0; i < 256; i++) {
                realcolor[i] = make16color(cmap.red[i], cmap.green[i], cmap.blue[i], cmap.transp[i],
                                           rl, ro, gl, go, bl, bo, tl, to);
	}
}

#if 0
void CFrameBuffer::paintBoxRel(const int x, const int y, const int dx, const int dy, const fb_pixel_t col)
{
	if (!getActive())
		return;

	uint8_t * pos = ((uint8_t *)getFrameBufferPointer()) + x * sizeof(fb_pixel_t) + stride * y;
	for (int count = 0; count < dy; count++) {
		fb_pixel_t * dest = (fb_pixel_t *)pos;
		for (int i = 0; i < dx; i++)
			*(dest++) = col;
		pos += stride;
	}
}
#endif

#ifdef __sh__
void CFrameBuffer::paintBoxRel(const int _x, const int _y, const int _dx, const int _dy, const fb_pixel_t col, int _radius, int type)
#else
void CFrameBuffer::paintBoxRel(const int x, const int y, const int dx, const int dy, const fb_pixel_t col, int radius, int type)
#endif
{
#ifdef __sh__
    int x, y, dx, dy, radius;
    x = scaleX(_x);
    y = scaleY(_y);
    dx = scaleX(_dx);
    dy = scaleY(_dy);
    radius = scaleX(_radius);
#endif

    /* draw a filled rectangle (with additional round corners) */

    if (!getActive())
        return;


#ifdef USE_NEVIS_GXA
    /* this table contains the x coordinates for a quarter circle (the bottom right quarter) with fixed 
       radius of 540 px which is the half of the max HD graphics size of 1080 px. So with that table we
       ca draw boxes with round corners and als circles by just setting dx = dy = radius (max 540). */
    int q_circle[541] = {
	540, 540, 540, 540, 540, 540, 540, 540, 540, 540, 540, 540, 540, 540, 540, 540, 540, 540, 540, 540, 
        540, 540, 540, 540, 539, 539, 539, 539, 539, 539, 539, 539, 539, 539, 539, 539, 539, 539, 539, 539, 
        539, 538, 538, 538, 538, 538, 538, 538, 538, 538, 538, 538, 538, 537, 537, 537, 537, 537, 537, 537, 
        537, 537, 536, 536, 536, 536, 536, 536, 536, 536, 535, 535, 535, 535, 535, 535, 535, 535, 534, 534, 
	534, 534, 534, 534, 533, 533, 533, 533, 533, 533, 532, 532, 532, 532, 532, 532, 531, 531, 531, 531, 
        531, 531, 530, 530, 530, 530, 529, 529, 529, 529, 529, 529, 528, 528, 528, 528, 527, 527, 527, 527, 
	527, 526, 526, 526, 526, 525, 525, 525, 525, 524, 524, 524, 524, 523, 523, 523, 523, 522, 522, 522, 
	522, 521, 521, 521, 521, 520, 520, 520, 519, 519, 519, 518, 518, 518, 518, 517, 517, 517, 516, 516, 
	516, 515, 515, 515, 515, 514, 514, 514, 513, 513, 513, 512, 512, 512, 511, 511, 511, 510, 510, 510, 
	509, 509, 508, 508, 508, 507, 507, 507, 506, 506, 506, 505, 505, 504, 504, 504, 503, 503, 502, 502, 
	502, 501, 501, 500, 500, 499, 499, 499, 498, 498, 498, 497, 497, 496, 496, 496, 495, 495, 494, 494, 
	493, 493, 492, 492, 491, 491, 490, 490, 490, 489, 489, 488, 488, 487, 487, 486, 486, 485, 485, 484, 
	484, 483, 483, 482, 482, 481, 481, 480, 480, 479, 479, 478, 478, 477, 477, 476, 476, 475, 475, 474, 
	473, 473, 472, 472, 471, 471, 470, 470, 469, 468, 468, 467, 466, 466, 465, 465, 464, 464, 463, 462, 
	462, 461, 460, 460, 459, 459, 458, 458, 457, 456, 455, 455, 454, 454, 453, 452, 452, 451, 450, 450, 
	449, 449, 448, 447, 446, 446, 445, 445, 444, 443, 442, 441, 441, 440, 440, 439, 438, 437, 436, 436, 
	435, 435, 434, 433, 432, 431, 431, 430, 429, 428, 427, 427, 426, 425, 425, 424, 423, 422, 421, 421, 
	420, 419, 418, 417, 416, 416, 415, 414, 413, 412, 412, 411, 410, 409, 408, 407, 406, 405, 404, 403, 
	403, 402, 401, 400, 399, 398, 397, 397, 395, 394, 393, 393, 392, 391, 390, 389, 388, 387, 386, 385, 
	384, 383, 382, 381, 380, 379, 378, 377, 376, 375, 374, 373, 372, 371, 369, 368, 367, 367, 365, 364, 
	363, 362, 361, 360, 358, 357, 356, 355, 354, 353, 352, 351, 350, 348, 347, 346, 345, 343, 342, 341, 
	340, 339, 337, 336, 335, 334, 332, 331, 329, 328, 327, 326, 324, 323, 322, 321, 319, 317, 316, 315, 
	314, 312, 310, 309, 308, 307, 305, 303, 302, 301, 299, 297, 296, 294, 293, 291, 289, 288, 287, 285, 
	283, 281, 280, 278, 277, 275, 273, 271, 270, 268, 267, 265, 263, 261, 259, 258, 256, 254, 252, 250, 
	248, 246, 244, 242, 240, 238, 236, 234, 232, 230, 228, 225, 223, 221, 219, 217, 215, 212, 210, 207, 
	204, 202, 200, 197, 195, 192, 190, 187, 184, 181, 179, 176, 173, 170, 167, 164, 160, 157, 154, 150, 
	147, 144, 140, 136, 132, 128, 124, 120, 115, 111, 105, 101,  95,  89,  83,  77,  69,  61,  52,  40, 
	 23};

    int line = 0;
    unsigned int cmd = GXA_CMD_NOT_TEXT | GXA_SRC_BMP_SEL(2) | GXA_DST_BMP_SEL(2) | GXA_PARAM_COUNT(2) | GXA_CMD_NOT_ALPHA;

    _write_gxa(gxa_base, GXA_FG_COLOR_REG, (unsigned int) col);		/* setup the drawing color */
    _write_gxa(gxa_base, GXA_LINE_CONTROL_REG, 0x00000404); 		/* X is major axis, skip last pixel */

    if ((type) && (radius))
    {
	#define MUL 32768	/* just an multiplicator for all math to reduce rounding errors */
	int ofs, scf, scl;

	/* limit the radius */
	if (radius > dx)
	    radius = dx;
	if (radius > dy)
	    radius = dy;
	if (radius > 540)
	    radius = 540;

	scf = (540 * MUL) / radius;

	while (line < dy)
	{
	    ofs = 0;

	    if ((line < radius) && (type & 1))
	    {
		/* uper round corners */
		scl = scf * (radius - line) / MUL;
		if ((scf * (radius - line) % MUL) >= (MUL / 2))	/* round up */
		    scl++;
		ofs = radius - (q_circle[scl] * MUL / scf);
	    }
	    else if ((line >= (dy - radius)) && (type & 2))
	    {
		/* lower round corners */
		scl = scf * (radius - (dy - (line + 1))) / MUL;
		if ((scf * (radius - (dy - (line + 1))) % MUL) >= (MUL / 2))	/* round up */
		    scl++;
		ofs = radius - (q_circle[scl] * MUL / scf);
	    }
	    _write_gxa(gxa_base, cmd, GXA_POINT(x + dx - ofs, y + line));		/* endig point */
    	    _write_gxa(gxa_base, cmd, GXA_POINT(x      + ofs, y + line));		/* start point */

	    line++;
	}
    }
    else
    {
	while (line < dy)
	{
    	    _write_gxa(gxa_base, cmd, GXA_POINT(x + dx, y + line));		/* endig point */
    	    _write_gxa(gxa_base, cmd, GXA_POINT(x,      y + line));		/* start point */
	    line++;
	}
    }

#else
    int F,R=radius,sx,sy,dxx=dx,dyy=dy,rx,ry,wx,wy;

    if (!getActive())
        return;

    uint8_t *pos = ((uint8_t *)getFrameBufferPointer()) + x*sizeof(fb_pixel_t) + stride*y;
    uint8_t *pos0 = 0, *pos1 = 0, *pos2 = 0, *pos3 = 0;

    fb_pixel_t *dest0, *dest1;

    if(R) {
        if(--dyy<=0) {
        //if(dyy <= 0)
            dyy=1;
        }

        if(R==1 || R>(dxx/2) || R>(dyy/2)) {
            R=dxx/10;
            F=dyy/10;
            if(R>F) {
                if(R>(dyy/3)) {
                    R=dyy/3;
                }
            } else {
                R=F;
                if(R>(dxx/3)) {
                    R=dxx/3;
                }
            }
        }
        sx=0;
        sy=R;
        F=1-R;

        rx=R-sx;
        ry=R-sy;

	if(type & 1) {
        	pos1=pos+(ry*stride); // top 1
        	pos2=pos+(rx*stride); // top 2
	}
	if(type & 2) {
        	pos0=pos+((dyy-ry)*stride); // bottom 1
        	pos3=pos+((dyy-rx)*stride); // bottom 2
	}
        while (sx <= sy) {
            rx=R-sx;
            ry=R-sy;
            wx=rx<<1;
            wy=ry<<1;
            dest0=(fb_pixel_t *)(pos0+rx*sizeof(fb_pixel_t));
            dest1=(fb_pixel_t *)(pos1+rx*sizeof(fb_pixel_t));
            for (int i=0; i<(dxx-wx); i++) {
		if(type & 2)
                	*(dest0++)=col;	//bottom 1
		if(type & 1)
                	*(dest1++)=col;	// top 1
            }
            dest0=(fb_pixel_t *)(pos2+ry*sizeof(fb_pixel_t));
            dest1=(fb_pixel_t *)(pos3+ry*sizeof(fb_pixel_t));
            for (int i=0; i<(dxx-wy); i++) {
		if(type & 1)
                	*(dest0++)=col;	// top 2
		if(type & 2)
                	*(dest1++)=col;	//bottom 2
            }
            sx++;
            pos2-=stride;
            pos3+=stride;
            if (F<0) {
                F+=(sx<<1)-1;
            } else {
                F+=((sx-sy)<<1);
                sy--;
                pos0-=stride;
                pos1+=stride;
            }
        }
	if(type & 1)
        	pos+=R*stride;
    }

    int start = R;
    int end = dyy - R;
    if(!(type & 1))
	start = 0;
    if(!(type & 2))
	end = dyy+ (R ? 1 : 0);

#ifdef __sh__
	blitRect(x, y + start, dx, end - start, col);
#else
    for (int count= start; count < end; count++) {
        dest0=(fb_pixel_t *)pos;
        for (int i=0; i<dxx; i++)
            *(dest0++)=col;
        pos+=stride;
    }
#endif
#endif /* USE_NEVIS_GXA */
}

void CFrameBuffer::paintVLine(int x, int ya, int yb, const fb_pixel_t col)
{
	if (!getActive())
		return;

#ifdef __sh__
	x = scaleX(x);
	ya = scaleY(ya);
	yb = scaleY(yb);
#endif

#ifdef USE_NEVIS_GXA
    /* draw a single vertical line from point x/ya to x/yb */
    unsigned int cmd = GXA_CMD_NOT_TEXT | GXA_SRC_BMP_SEL(2) | GXA_DST_BMP_SEL(2) | GXA_PARAM_COUNT(2) | GXA_CMD_NOT_ALPHA;

    _write_gxa(gxa_base, GXA_FG_COLOR_REG, (unsigned int) col);	/* setup the drawing color */
    _write_gxa(gxa_base, GXA_LINE_CONTROL_REG, 0x00000404); 	/* X is major axis, skip last pixel */
    _write_gxa(gxa_base, cmd, GXA_POINT(x, ya + (yb - ya)));	/* end point */
    _write_gxa(gxa_base, cmd, GXA_POINT(x, ya));		/* start point */
#else /* USE_NEVIS_GXA */
#ifdef __sh__
	int dy = yb-ya;
	blitRect(x, ya, 1, dy, col);
#else
	uint8_t * pos = ((uint8_t *)getFrameBufferPointer()) + x * sizeof(fb_pixel_t) + stride * ya;

	int dy = yb-ya;
	for (int count = 0; count < dy; count++) {
		*(fb_pixel_t *)pos = col;
		pos += stride;
	}
#endif
#endif	/* USE_NEVIS_GXA */
}

void CFrameBuffer::paintVLineRel(int x, int y, int dy, const fb_pixel_t col)
{	
	if (!getActive())
		return;

#ifdef __sh__
	x = scaleX(x);
	y = scaleY(y);
	dy = scaleY(dy);
#endif

#ifdef USE_NEVIS_GXA
    /* draw a single vertical line from point x/y with hight dx */
    unsigned int cmd = GXA_CMD_NOT_TEXT | GXA_SRC_BMP_SEL(2) | GXA_DST_BMP_SEL(2) | GXA_PARAM_COUNT(2) | GXA_CMD_NOT_ALPHA;

    _write_gxa(gxa_base, GXA_FG_COLOR_REG, (unsigned int) col);	/* setup the drawing color */
    _write_gxa(gxa_base, GXA_LINE_CONTROL_REG, 0x00000404); 	/* X is major axis, skip last pixel */
    _write_gxa(gxa_base, cmd, GXA_POINT(x, y + dy));		/* end point */
    _write_gxa(gxa_base, cmd, GXA_POINT(x, y));			/* start point */
#else /* USE_NEVIS_GXA */
#ifdef __sh__
	blitRect(x, y, 1, dy, col);
#else
	uint8_t * pos = ((uint8_t *)getFrameBufferPointer()) + x * sizeof(fb_pixel_t) + stride * y;

	for(int count=0;count<dy;count++) {
		*(fb_pixel_t *)pos = col;
		pos += stride;
	}
#endif
#endif /* USE_NEVIS_GXA */
}

void CFrameBuffer::paintHLine(int xa, int xb, int y, const fb_pixel_t col)
{	
	if (!getActive())
		return;

#ifdef __sh__
	xa = scaleX(xa);
	xb = scaleX(xb);
	y = scaleY(y);
#endif

#ifdef USE_NEVIS_GXA
	/* draw a single horizontal line from point xa/y to xb/y */
	unsigned int cmd = GXA_CMD_NOT_TEXT | GXA_SRC_BMP_SEL(2) | GXA_DST_BMP_SEL(2) | GXA_PARAM_COUNT(2) | GXA_CMD_NOT_ALPHA;

	_write_gxa(gxa_base, GXA_FG_COLOR_REG, (unsigned int) col);	/* setup the drawing color */
	_write_gxa(gxa_base, GXA_LINE_CONTROL_REG, 0x00000404); 	/* X is major axis, skip last pixel */
	_write_gxa(gxa_base, cmd, GXA_POINT(xa + (xb - xa), y));	/* end point */
	_write_gxa(gxa_base, cmd, GXA_POINT(xa, y));		/* start point */
#else /* USE_NEVIS_GXA */
#ifdef __sh__
	int dx = xb -xa;
	blitRect(xa, y, dx, 1, col);
#else
	uint8_t * pos = ((uint8_t *)getFrameBufferPointer()) + xa * sizeof(fb_pixel_t) + stride * y;

	int dx = xb -xa;
	fb_pixel_t * dest = (fb_pixel_t *)pos;
	for (int i = 0; i < dx; i++)
		*(dest++) = col;
#endif
#endif /* USE_NEVIS_GXA */
}

void CFrameBuffer::paintHLineRel(int x, int dx, int y, const fb_pixel_t col)
{
	if (!getActive())
		return;

#ifdef __sh__
	x = scaleX(x);
	dx = scaleX(dx);
	y = scaleY(y);
#endif

#ifdef USE_NEVIS_GXA
	/* draw a single horizontal line from point x/y with width dx */
	unsigned int cmd = GXA_CMD_NOT_TEXT | GXA_SRC_BMP_SEL(2) | GXA_DST_BMP_SEL(2) | GXA_PARAM_COUNT(2) | GXA_CMD_NOT_ALPHA;

	_write_gxa(gxa_base, GXA_FG_COLOR_REG, (unsigned int) col);	/* setup the drawing color */
	_write_gxa(gxa_base, GXA_LINE_CONTROL_REG, 0x00000404); 	/* X is major axis, skip last pixel */
	_write_gxa(gxa_base, cmd, GXA_POINT(x + dx, y));		/* end point */
	_write_gxa(gxa_base, cmd, GXA_POINT(x, y));			/* start point */
#else /* USE_NEVIS_GXA */
#ifdef __sh__
	blitRect(x, y, dx, 1, col);
#else
	uint8_t * pos = ((uint8_t *)getFrameBufferPointer()) + x * sizeof(fb_pixel_t) + stride * y;

	fb_pixel_t * dest = (fb_pixel_t *)pos;
	for (int i = 0; i < dx; i++)
		*(dest++) = col;
#endif
#endif /* USE_NEVIS_GXA */
}

void CFrameBuffer::setIconBasePath(const std::string & iconPath)
{
	iconBasePath = iconPath;
}

#ifdef __sh__
bool CFrameBuffer::paintIcon8(const std::string & filename, const int _x, const int _y, const unsigned char offset)
#else
bool CFrameBuffer::paintIcon8(const std::string & filename, const int x, const int y, const unsigned char offset)
#endif
{
	if (!getActive())
		return false;

#ifdef __sh__
	int x, y;
	x = scaleX(_x);
	y = scaleY(_y);
#endif

//printf("%s(file, %d, %d, %d)\n", __FUNCTION__, x, y, offset);

	struct rawHeader header;
	uint16_t         width, height;
	int              fd;

	fd = open((iconBasePath + filename).c_str(), O_RDONLY);

	if (fd == -1) {
		printf("paintIcon8: error while loading icon: %s%s\n", iconBasePath.c_str(), filename.c_str());
		return false;
	}

	read(fd, &header, sizeof(struct rawHeader));

	width  = (header.width_hi  << 8) | header.width_lo;
	height = (header.height_hi << 8) | header.height_lo;
#ifdef __sh__
	if(width > ICON_TEMP_SIZE || height > ICON_TEMP_SIZE)
	{
		close(fd);
		return false;
	}
	
	unsigned char pixbuf[768];

	uint8_t * d = (uint8_t *)icon_space;
	fb_pixel_t * d2;
	for (int count=0; count<height; count ++ ) {
		read(fd, &pixbuf[0], width );
		unsigned char *pixpos = &pixbuf[0];
		d2 = (fb_pixel_t *) d;
		for (int count2=0; count2<width; count2 ++ ) {
			unsigned char color = *pixpos;
			if (color != header.transp)
				paintPixel(d2, color + offset);
			else
				*d2 = 0;
			d2++;
			pixpos++;
		}
		d += width * 4;
	}
	blitIcon(width, height, x, y, scaleX(width), scaleY(height));

#else
	unsigned char pixbuf[768];

	uint8_t * d = ((uint8_t *)getFrameBufferPointer()) + x * sizeof(fb_pixel_t) + stride * y;
	fb_pixel_t * d2;
	for (int count=0; count<height; count ++ ) {
		read(fd, &pixbuf[0], width );
		unsigned char *pixpos = &pixbuf[0];
		d2 = (fb_pixel_t *) d;
		for (int count2=0; count2<width; count2 ++ ) {
			unsigned char color = *pixpos;
			if (color != header.transp) {
//printf("icon8: col %d transp %d real %08X\n", color+offset, header.transp, realcolor[color+offset]);
				paintPixel(d2, color + offset);
			}
			d2++;
			pixpos++;
		}
		d += stride;
	}
#endif

	if (d) d = NULL;
	if (d2) d2 = NULL;
	close(fd);

	return true;
}

bool CFrameBuffer::paintIcon(const std::string & filename, const int _x, const int _y, const unsigned char offset)
{
	if (!getActive())
		return false;

#ifdef __sh__
	int x, y;
	x = scaleX(_x);
	y = scaleY(_y);
#endif

//printf("%s(file, %d, %d, %d)\n", __FUNCTION__, x, y, offset);
#ifdef __sh__
	char * ptr = (char *)rindex(filename.c_str(), '.');
#else
	char * ptr = rindex(filename.c_str(), '.');
#endif
	if(ptr) {
		*ptr = 0;
		std::string newname = iconBasePath + filename.c_str() + ".gif";
		*ptr = '.';
		if(!access(newname.c_str(), F_OK))
			return g_PicViewer->DisplayImage(newname, x, y, 0, 0);
	}
	struct rawHeader header;
	uint16_t         width, height;
	int              fd;
#if 0 // no need if we have whole / as r/w
	std::string iconBasePath1 = "/var/share/icons/";
	fd = open((iconBasePath1 + filename).c_str(), O_RDONLY);
	if (fd == -1)
		fd = open((iconBasePath + filename).c_str(), O_RDONLY);
#endif
	fd = open((iconBasePath + filename).c_str(), O_RDONLY);

	if (fd == -1) {
		printf("paintIcon: error while loading icon: %s%s\n", iconBasePath.c_str(), filename.c_str());
		return false;
	}

	read(fd, &header, sizeof(struct rawHeader));

	width  = (header.width_hi  << 8) | header.width_lo;
	height = (header.height_hi << 8) | header.height_lo;

#ifdef __sh__
	if(width > ICON_TEMP_SIZE || height > ICON_TEMP_SIZE)
	{
		close(fd);
		return false;
	}

	unsigned char pixbuf[768];
	uint8_t * d = (uint8_t *)icon_space;
	fb_pixel_t * d2;
	for (int count=0; count<height; count ++ ) {
		read(fd, &pixbuf[0], width >> 1 );
		unsigned char *pixpos = &pixbuf[0];
		d2 = (fb_pixel_t *) d;
		for (int count2=0; count2<width >> 1; count2 ++ ) {
			unsigned char compressed = *pixpos;
			unsigned char pix1 = (compressed & 0xf0) >> 4;
			unsigned char pix2 = (compressed & 0x0f);
			if (pix1 != header.transp)
				paintPixel(d2, pix1 + offset);
			else
				*d2 = 0;
			
			d2++;
			if (pix2 != header.transp)
				paintPixel(d2, pix2 + offset);
			else
				*d2 = 0;
			d2++;
			pixpos++;
		}
		d += width * 4;
	}
	blitIcon(width, height, x, y, scaleX(width), scaleY(height));
#else
	unsigned char pixbuf[768];
	uint8_t * d = ((uint8_t *)getFrameBufferPointer()) + x * sizeof(fb_pixel_t) + stride * y;
	fb_pixel_t * d2;
	for (int count=0; count<height; count ++ ) {
		read(fd, &pixbuf[0], width >> 1 );
		unsigned char *pixpos = &pixbuf[0];
		d2 = (fb_pixel_t *) d;
		for (int count2=0; count2<width >> 1; count2 ++ ) {
			unsigned char compressed = *pixpos;
			unsigned char pix1 = (compressed & 0xf0) >> 4;
			unsigned char pix2 = (compressed & 0x0f);
			if (pix1 != header.transp) {
				paintPixel(d2, pix1 + offset);
			}
			d2++;
			if (pix2 != header.transp) {
				paintPixel(d2, pix2 + offset);
			}
			d2++;
			pixpos++;
		}
		d += stride;
	}
#endif

	if (d) d = NULL;
	if (d2) d2 = NULL;
	close(fd);

	return true;
}

bool CFrameBuffer::paintIcon(const char * const filename, const int x, const int y, const unsigned char offset)
{
//printf("%s(%s, %d, %d, %d)\n", __FUNCTION__, filename, x, y, offset);
	return paintIcon(std::string(filename), x, y, offset);
}

void CFrameBuffer::loadPal(const std::string & filename, const unsigned char offset, const unsigned char endidx)
{
	if (!getActive())
		return;

//printf("%s()\n", __FUNCTION__);

	struct rgbData rgbdata;
	int            fd;

	fd = open((iconBasePath + filename).c_str(), O_RDONLY);

	if (fd == -1) {
		printf("error while loading palette: %s%s\n", iconBasePath.c_str(), filename.c_str());
		return;
	}

	int pos = 0;
	int readb = read(fd, &rgbdata,  sizeof(rgbdata) );
	while(readb) {
		__u32 rgb = (rgbdata.r<<16) | (rgbdata.g<<8) | (rgbdata.b);
		int colpos = offset+pos;
		if( colpos>endidx)
			break;

		paletteSetColor(colpos, rgb, 0xFF);
		readb = read(fd, &rgbdata,  sizeof(rgbdata) );
		pos++;
	}
	paletteSet(&cmap);
	close(fd);
}

void CFrameBuffer::paintPixel(const int _x, const int _y, const fb_pixel_t col, bool applyScaling)
{
#ifdef __sh__
	int x, y;
	if(applyScaling)
	{
		x = scaleX(_x);
		y = scaleY(_y);
	}
	else
	{
		x = _x;
		y = _y;
	}
#endif
	
	if (!getActive())
		return;

	#ifdef USE_NEVIS_GXA
	paintHLineRel(x, 1, y, col);
	#else
	fb_pixel_t * pos = getFrameBufferPointer();
	pos += (stride / sizeof(fb_pixel_t)) * y;
	pos += x;

	*pos = col;
	#endif
}

void CFrameBuffer::paintLine(int xa, int ya, int xb, int yb, const fb_pixel_t col)
{
	if (!getActive())
		return;

#ifdef __sh__
	xa = scaleX(xa);
	xb = scaleX(xb);
	ya = scaleY(ya);
	yb = scaleY(yb);
#endif

//printf("%s(%d, %d, %d, %d, %.8X)\n", __FUNCTION__, xa, ya, xb, yb, col);

	int dx;
	int dy;
	int x;
	int y;
	int End;
	int step;

	dx = abs (xa - xb);
	dy = abs (ya - yb);

	if ( dx > dy )
	{
		int	p = 2 * dy - dx;
		int	twoDy = 2 * dy;
		int	twoDyDx = 2 * (dy-dx);

		if ( xa > xb )
		{
			x = xb;
			y = yb;
			End = xa;
			step = ya < yb ? -1 : 1;
		}
		else
		{
			x = xa;
			y = ya;
			End = xb;
			step = yb < ya ? -1 : 1;
		}

#ifdef __sh__
		paintPixel (x, y, col, false);
#else
		paintPixel (x, y, col);
#endif

		while( x < End )
		{
			x++;
			if ( p < 0 )
				p += twoDy;
			else
			{
				y += step;
				p += twoDyDx;
			}
#ifdef __sh__
			paintPixel (x, y, col, false);
#else
			paintPixel (x, y, col);
#endif
		}
	}
	else
	{
		int	p = 2 * dx - dy;
		int	twoDx = 2 * dx;
		int	twoDxDy = 2 * (dx-dy);

		if ( ya > yb )
		{
			x = xb;
			y = yb;
			End = ya;
			step = xa < xb ? -1 : 1;
		}
		else
		{
			x = xa;
			y = ya;
			End = yb;
			step = xb < xa ? -1 : 1;
		}

#ifdef __sh__
		paintPixel (x, y, col, false);
#else
		paintPixel (x, y, col);
#endif

		while( y < End )
		{
			y++;
			if ( p < 0 )
				p += twoDx;
			else
			{
				x += step;
				p += twoDxDy;
			}
#ifdef __sh__
			paintPixel (x, y, col, false);
#else
			paintPixel (x, y, col);
#endif
		}
	}

}

void CFrameBuffer::setBackgroundColor(const fb_pixel_t color)
{
	backgroundColor = color;
}

bool CFrameBuffer::loadPictureToMem(const std::string & filename, const uint16_t width, const uint16_t height, const uint16_t stride, fb_pixel_t * memp)
{
	struct rawHeader header;
	int              fd;

//printf("%s(%d, %d, memp)\n", __FUNCTION__, width, height);

	fd = open((iconBasePath + filename).c_str(), O_RDONLY );

	if (fd == -1)
	{
		printf("error while loading icon: %s%s\n", iconBasePath.c_str(), filename.c_str());
		return false;
	}

	read(fd, &header, sizeof(struct rawHeader));


	if ((width  != ((header.width_hi  << 8) | header.width_lo)) ||
	    (height != ((header.height_hi << 8) | header.height_lo)))
	{
		printf("error while loading icon: %s - invalid resolution = %hux%hu\n", filename.c_str(), width, height);
		return false;
	}

	if ((stride == 0) || (stride == width * sizeof(fb_pixel_t)))
		read(fd, memp, height * width * sizeof(fb_pixel_t));
	else
		for (int i = 0; i < height; i++)
			read(fd, ((uint8_t *)memp) + i * stride, width * sizeof(fb_pixel_t));

	close(fd);
	return true;
}

bool CFrameBuffer::loadPicture2Mem(const std::string & filename, fb_pixel_t * memp)
{
#ifdef __sh__
	return loadPictureToMem(filename, xRes, yRes, 0, memp);
#else
	return loadPictureToMem(filename, BACKGROUNDIMAGEWIDTH, 576, 0, memp);
#endif
}

bool CFrameBuffer::loadPicture2FrameBuffer(const std::string & filename)
{
	if (!getActive())
		return false;
#ifdef __sh__
	return loadPictureToMem(filename, xRes, yRes, getStride(), getFrameBufferPointer());
#else
	return loadPictureToMem(filename, BACKGROUNDIMAGEWIDTH, 576, getStride(), getFrameBufferPointer());
#endif
}

bool CFrameBuffer::savePictureFromMem(const std::string & filename, const fb_pixel_t * const memp)
{
	struct rawHeader header;
	uint16_t         width, height;
	int              fd;

#ifdef __sh__	
	width = xRes;
	height = yRes;
#else
	width = BACKGROUNDIMAGEWIDTH;
	height = 576;
#endif

	header.width_lo  = width  &  0xFF;
	header.width_hi  = width  >>    8;
	header.height_lo = height &  0xFF;
	header.height_hi = height >>    8;
	header.transp    =              0;

	fd = open((iconBasePath + filename).c_str(), O_WRONLY | O_CREAT);

	if (fd==-1)
	{
		printf("error while saving icon: %s%s", iconBasePath.c_str(), filename.c_str() );
		return false;
	}

	write(fd, &header, sizeof(struct rawHeader));

	write(fd, memp, width * height * sizeof(fb_pixel_t));

	close(fd);
	return true;
}

bool CFrameBuffer::loadBackground(const std::string & filename, const unsigned char offset)
{
	if ((backgroundFilename == filename) && (background))
		return true;

	if (background)
		delete[] background;

#ifdef __sh__
	background = new fb_pixel_t[xRes * yRes];

	if (!loadPictureToMem(filename, xRes, yRes, 0, background))
#else
	background = new fb_pixel_t[BACKGROUNDIMAGEWIDTH * 576];

	if (!loadPictureToMem(filename, BACKGROUNDIMAGEWIDTH, 576, 0, background))
#endif
	{
		delete[] background;
		background=0;
		return false;
	}

	if (offset != 0)//pic-offset
	{
		fb_pixel_t * bpos = background;
#ifdef __sh__
		int pos = xRes * yRes;
#else
		int pos = BACKGROUNDIMAGEWIDTH * 576;
#endif
		while (pos > 0)
		{
			*bpos += offset;
			bpos++;
			pos--;
		}
	}

#ifdef __sh__
	fb_pixel_t * dest = background + xRes * yRes;
	uint8_t    * src  = ((uint8_t * )background)+ xRes * yRes;
	for (int i = yRes - 1; i >= 0; i--)
		for (int j = xRes - 1; j >= 0; j--)
#else
	fb_pixel_t * dest = background + BACKGROUNDIMAGEWIDTH * 576;
	uint8_t    * src  = ((uint8_t * )background)+ BACKGROUNDIMAGEWIDTH * 576;
	for (int i = 576 - 1; i >= 0; i--)
		for (int j = BACKGROUNDIMAGEWIDTH - 1; j >= 0; j--)
#endif
		{
			dest--;
			src--;
			paintPixel(dest, *src);
		}
	backgroundFilename = filename;

	return true;
}

bool CFrameBuffer::loadBackgroundPic(const std::string & filename, bool show)
{
	if ((backgroundFilename == filename) && (background))
		return true;

//printf("loadBackgroundPic: %s\n", filename.c_str());
	if (background)
		delete[] background;

#ifdef __sh__
	background = g_PicViewer->getImage(iconBasePath + filename, xRes, yRes);
#else
	background = g_PicViewer->getImage(iconBasePath + filename, BACKGROUNDIMAGEWIDTH, 576);
#endif

	if (background == NULL) {
		background=0;
		return false;
	}

	backgroundFilename = filename;
	if(show) {
		useBackgroundPaint = true;
		paintBackground();
	}
	return true;
}

void CFrameBuffer::useBackground(bool ub)
{
	useBackgroundPaint = ub;
	if(!useBackgroundPaint) {
		delete[] background;
		background=0;
	}
}

bool CFrameBuffer::getuseBackground(void)
{
	return useBackgroundPaint;
}

void CFrameBuffer::saveBackgroundImage(void)
{
	if (backupBackground != NULL)
		delete[] backupBackground;

	backupBackground = background;
	//useBackground(false); // <- necessary since no background is available
	useBackgroundPaint = false;
	background = NULL;
}

void CFrameBuffer::restoreBackgroundImage(void)
{
	fb_pixel_t * tmp = background;

	if (backupBackground != NULL)
	{
		background = backupBackground;
		backupBackground = NULL;
	}
	else
		useBackground(false); // <- necessary since no background is available

	if (tmp != NULL)
		delete[] tmp;
}

void CFrameBuffer::paintBackgroundBoxRel(int x, int y, int dx, int dy)
{
	if (!getActive())
		return;

	if(!useBackgroundPaint)
	{
		paintBoxRel(x, y, dx, dy, backgroundColor);
	}
	else
	{
		x = scaleX(x);
		y = scaleY(y);
		dx = scaleX(dx);
		dy = scaleY(dy);
		uint8_t * fbpos = ((uint8_t *)getFrameBufferPointer()) + x * sizeof(fb_pixel_t) + stride * y;
#ifdef __sh__
		fb_pixel_t * bkpos = background + x + xRes * y;
#else
		fb_pixel_t * bkpos = background + x + BACKGROUNDIMAGEWIDTH * y;
#endif
		for(int count = 0;count < dy; count++)
		{
			memcpy(fbpos, bkpos, dx * sizeof(fb_pixel_t));
			fbpos += stride;
#ifdef __sh__
			bkpos += xRes;
#else
			bkpos += BACKGROUNDIMAGEWIDTH;
#endif
		}
	}
}

void CFrameBuffer::paintBackground()
{
	if (!getActive())
		return;

	if (useBackgroundPaint && (background != NULL))
	{
#ifdef __sh__
		for (int i = 0; i < yRes; i++)
			memcpy(((uint8_t *)getFrameBufferPointer()) + i * stride, (background + i * xRes), xRes * sizeof(fb_pixel_t));
#else
		for (int i = 0; i < 576; i++)
			memcpy(((uint8_t *)getFrameBufferPointer()) + i * stride, (background + i * BACKGROUNDIMAGEWIDTH), BACKGROUNDIMAGEWIDTH * sizeof(fb_pixel_t));
#endif

	}
	else
	{
		paintBoxRel(0, 0, xRes, yRes, backgroundColor);
	}
}

#ifdef __sh__
fb_pixel_t *CFrameBuffer::allocPixelBuffer(int width, int height)
{
	return new fb_pixel_t[scaleX(width) * scaleY(height)];
}


int CFrameBuffer::SaveScreen(int x, int y, int dx, int dy, fb_pixel_t * const memp)
#else
void CFrameBuffer::SaveScreen(int x, int y, int dx, int dy, fb_pixel_t * const memp)
#endif
{
#ifdef __sh__
	x = scaleX(x);
	dx = scaleX(dx);
	y = scaleY(y);
	dy = scaleY(dy);
#endif
	
	if (!getActive())
#ifdef __sh__
		return -1;
#else
		return;
#endif

	uint8_t * pos = ((uint8_t *)getFrameBufferPointer()) + x * sizeof(fb_pixel_t) + stride * y;
	fb_pixel_t * bkpos = memp;
	for (int count = 0; count < dy; count++) {
		fb_pixel_t * dest = (fb_pixel_t *)pos;
		for (int i = 0; i < dx; i++)
			//*(dest++) = col;
			*(bkpos++) = *(dest++);
		pos += stride;
	}
#ifdef __sh__
	return dx * dy * bpp / 4;
#endif
#if 0 //FIXME test to flush cache
        if (ioctl(fd, 1, FB_BLANK_UNBLANK) < 0);
#endif
	//RestoreScreen(x, y, dx, dy, memp); //FIXME
#if 0
	uint8_t * fbpos = ((uint8_t *)getFrameBufferPointer()) + x * sizeof(fb_pixel_t) + stride * y;
	fb_pixel_t * bkpos = memp;
	for (int count = 0; count < dy; count++)
	{
		memcpy(bkpos, fbpos, dx * sizeof(fb_pixel_t));
		fbpos += stride;
		bkpos += dx;
	}
#endif

}

#ifdef __sh__
void CFrameBuffer::RestoreScreen(int x, int y, int dx, int dy, fb_pixel_t * const memp, int checkSize)
{
	x = scaleX(x);
	dx = scaleX(dx);
	y = scaleY(y);
	dy = scaleY(dy);

#else
void CFrameBuffer::RestoreScreen(int x, int y, int dx, int dy, fb_pixel_t * const memp)
{
#endif
	if (!getActive())
		return;

	uint8_t * fbpos = ((uint8_t *)getFrameBufferPointer()) + x * sizeof(fb_pixel_t) + stride * y;
	fb_pixel_t * bkpos = memp;
	for (int count = 0; count < dy; count++)
	{
#ifdef __sh__
		if(dx * dy * bpp / 4 != checkSize)  // to prevent restoring when the buffersize changed
			for(int w = 0; w < dx; w++) // TODO: copy to blitter and resize the image to fit
				*fbpos = 0;	    // (but practically this only occurs when the screen is black during boot...)
		else
#endif
			memcpy(fbpos, bkpos, dx * sizeof(fb_pixel_t));
		fbpos += stride;
		bkpos += dx;
	}
}

void CFrameBuffer::switch_signal (int signal)
{
	CFrameBuffer * thiz = CFrameBuffer::getInstance();
	if (signal == SIGUSR1) {
		if (virtual_fb != NULL)
			delete[] virtual_fb;
		virtual_fb = new uint8_t[thiz->stride * thiz->yRes];
		thiz->active = false;
		if (virtual_fb != NULL)
			memcpy(virtual_fb, thiz->lfb, thiz->stride * thiz->yRes);
		ioctl(thiz->tty, VT_RELDISP, 1);
		printf ("release display\n");
	}
	else if (signal == SIGUSR2) {
		ioctl(thiz->tty, VT_RELDISP, VT_ACKACQ);
		thiz->active = true;
		printf ("acquire display\n");
		thiz->paletteSet(NULL);
		if (virtual_fb != NULL)
			memcpy(thiz->lfb, virtual_fb, thiz->stride * thiz->yRes);
		else
			memset(thiz->lfb, 0, thiz->stride * thiz->yRes);
	}
}

void CFrameBuffer::ClearFrameBuffer()
{
	int tr;

	tr = 0xFF;
	//paletteSetColor(0, 0xFFFFFF, 0xFF);
	//Windows Colors
	paletteSetColor(0x1, 0x010101, tr);
	paletteSetColor(0x2, 0x800000, tr);
	paletteSetColor(0x3, 0x008000, tr);
	paletteSetColor(0x4, 0x808000, tr);
	paletteSetColor(0x5, 0x000080, tr);
	paletteSetColor(0x6, 0x800080, tr);
	paletteSetColor(0x7, 0x008080, tr);
	paletteSetColor(0x8, 0xA0A0A0, tr);
	paletteSetColor(0x9, 0x505050, tr);
	paletteSetColor(0xA, 0xFF0000, tr);
	paletteSetColor(0xB, 0x00FF00, tr);
	paletteSetColor(0xC, 0xFFFF00, tr);
	paletteSetColor(0xD, 0x0000FF, tr);
	paletteSetColor(0xE, 0xFF00FF, tr);
	paletteSetColor(0xF, 0x00FFFF, tr);
	paletteSetColor(0x10, 0xFFFFFF, tr);
	paletteSetColor(0x11, 0x000000, tr);
	useBackground(false);

	paintBackground();

	//background
	paletteSetColor(COL_BACKGROUND, 0x000000, 0xffff);

	paletteSet();
}

void CFrameBuffer::showFrame(const std::string & filename)
{
	std::string varpath = "/var/tuxbox/config/neutrino/icons/";
	if(!access((varpath + filename).c_str(), F_OK))
		videoDecoder->ShowPicture((varpath + filename).c_str());
	else
		videoDecoder->ShowPicture((iconBasePath + filename).c_str());
}

#if defined(__sh__) 

void CFrameBuffer::blitRect(int x, int y, int width, int height, unsigned long color)
{
//printf ("[fb - blitRect]: x=%d, y=%d, width=%d, height=%d, x+width=%d, y+height=%d\n", x, y, width, height, x+width, y+height);
if (width == 0 || height == 0) return;

	STMFBIO_BLT_DATA  bltData;
	memset(&bltData, 0, sizeof(STMFBIO_BLT_DATA));

	bltData.operation  = BLT_OP_FILL;
	bltData.dstOffset  = 0;
	bltData.dstPitch   = stride;

	bltData.dst_left   = x;
	bltData.dst_top    = y;
	bltData.dst_right  = x + width;
	bltData.dst_bottom = y + height;

	bltData.dstFormat  = SURF_ARGB8888;
	bltData.srcFormat  = SURF_ARGB8888;
	bltData.dstMemBase = STMFBGP_FRAMEBUFFER;
	bltData.srcMemBase = STMFBGP_FRAMEBUFFER;
	bltData.colour     = color; 

	if (ioctl(fd, STMFBIO_BLT, &bltData ) < 0)
		perror("FBIO_BLIT");
	else
		ioctl(fd, STMFBIO_SYNC_BLITTER);
}

void CFrameBuffer::blitIcon(int original_width, int original_height, int fb_x, int fb_y, int width, int height)
{
	STMFBIO_BLT_EXTERN_DATA blt_data;
	memset(&blt_data, 0, sizeof(STMFBIO_BLT_EXTERN_DATA));
	blt_data.operation  = BLT_OP_COPY;
	blt_data.ulFlags    = BLT_OP_FLAGS_BLEND_SRC_ALPHA | BLT_OP_FLAGS_BLEND_DST_MEMORY;	// we need alpha blending
	blt_data.srcOffset  = 0;
	blt_data.srcPitch   = original_width * 4;
	blt_data.dstOffset  = 0;
	blt_data.dstPitch   = stride;
	blt_data.src_top    = 0;
	blt_data.src_left   = 0;
	blt_data.src_right  = original_width;
	blt_data.src_bottom = original_height;
	blt_data.dst_left   = fb_x;
	blt_data.dst_top    = fb_y;
	blt_data.dst_right  = fb_x + width;
	blt_data.dst_bottom = fb_y + height;
	blt_data.srcFormat  = SURF_ARGB8888;
	blt_data.dstFormat  = SURF_ARGB8888;
	blt_data.srcMemBase = (char *)icon_space;
	blt_data.dstMemBase = (char *)lfb;
	blt_data.srcMemSize = ICON_TEMP_SIZE * ICON_TEMP_SIZE * 4;
	blt_data.dstMemSize = stride * yRes;
	
	// icons are so small that they will still be in cache
	msync(icon_space, ICON_TEMP_SIZE * ICON_TEMP_SIZE * 4, MS_SYNC);
	
	if(ioctl(fd, STMFBIO_BLT_EXTERN, &blt_data) < 0)
		perror("FBIO_BLIT");
	else
		ioctl(fd, STMFBIO_SYNC_BLITTER);
}

void CFrameBuffer::blitRGBtoRGB(int original_width, int original_height, int height, int width, char *original_data, char *dest_data)
{
	STMFBIO_BLT_EXTERN_DATA blt_data;
	memset(&blt_data, 0, sizeof(STMFBIO_BLT_EXTERN_DATA));
	blt_data.operation  = BLT_OP_COPY;
	blt_data.ulFlags    = 0;
	blt_data.srcOffset  = 0;
	blt_data.srcPitch   = original_width * 3;
	blt_data.dstOffset  = 0;
	blt_data.dstPitch   = width * 3;
	blt_data.src_top    = 0;
	blt_data.src_left   = 0;
	blt_data.src_right  = original_width;
	blt_data.src_bottom = original_height;
	blt_data.dst_left   = 0;
	blt_data.dst_top    = 0;
	blt_data.dst_right  = width;
	blt_data.dst_bottom = height;
	blt_data.srcFormat  = SURF_RGB888;
	blt_data.dstFormat  = SURF_RGB888;
	blt_data.srcMemBase = (char *)original_data;
	blt_data.dstMemBase = (char *)dest_data;
	blt_data.srcMemSize = original_width * original_height * 3; // we don't need to know the actual mem size
	blt_data.dstMemSize = width * height * 3;
	
	if(ioctl(fd, STMFBIO_BLT_EXTERN, &blt_data) < 0)
		perror("FBIO_BLIT");
	else
		ioctl(fd, STMFBIO_SYNC_BLITTER);
}

void CFrameBuffer::blitRGBtoFB(int pan_x, int pan_y, int original_width, int original_height, int fb_x, int fb_y, int width, int height, char *bpaData)
{
	STMFBIO_BLT_EXTERN_DATA blt_data;
	memset(&blt_data, 0, sizeof(STMFBIO_BLT_EXTERN_DATA));
	blt_data.operation  = BLT_OP_COPY;
	blt_data.ulFlags    = 0;
	blt_data.srcOffset  = 0;
	blt_data.srcPitch   = original_width * 3;
	blt_data.dstOffset  = 0;
	blt_data.dstPitch   = stride;
	blt_data.src_top    = pan_x;
	blt_data.src_left   = pan_y;
	blt_data.src_right  = pan_x + original_width;
	blt_data.src_bottom = pan_y + original_height;
	blt_data.dst_left   = fb_x;
	blt_data.dst_top    = fb_y;
	blt_data.dst_right  = fb_x + width;
	blt_data.dst_bottom = fb_y + height;
	blt_data.srcFormat  = SURF_BGR888;
	blt_data.dstFormat  = SURF_ARGB8888;
	blt_data.srcMemBase = (char *)bpaData;
	blt_data.dstMemBase = (char *)lfb;
	blt_data.srcMemSize = (pan_x + original_width) * (pan_y + original_height) * 3; // we don't need to know the actual mem size
	blt_data.dstMemSize = stride * yRes;
	
	if(ioctl(fd, STMFBIO_BLT_EXTERN, &blt_data) < 0)
		perror("FBIO_BLIT");
	else
		ioctl(fd, STMFBIO_SYNC_BLITTER);
}

void CFrameBuffer::resize(int format)
{
	const char *aVideoSystems[][2] = {
	{"VIDEO_STD_NTSC", "pal"},
	{"VIDEO_STD_SECAM", "pal"},
	{"VIDEO_STD_PAL", "pal"},
	{"VIDEO_STD_480P", "480p"},
	{"VIDEO_STD_576P", "576p50"},
	{"VIDEO_STD_720P60", "720p60"},
	{"VIDEO_STD_1080I60", "1080i60"},
	{"VIDEO_STD_720P50", "720p50"},
	{"VIDEO_STD_1080I50", "1080i50"},
	{"VIDEO_STD_1080P30", "1080p30"},
	{"VIDEO_STD_1080P24", "1080p24"},
	{"VIDEO_STD_1080P25", "1080p25"},
	{"VIDEO_STD_AUTO", "1080i50"}
	};

	printf("###RESIZE###\n");
	printf("video_system=%s\n", aVideoSystems[format][0]);

	int iaVideoSystems[][2] = {
	{720, 576},
	{720, 576},
	{720, 576},
	{640, 480},
	{720, 576},
	{1280, 720},
	{1920, 1080},
	{1280, 720},
	{1920, 1080},
	{1920, 1080},
	{1920, 1080},
	{1920, 1080},
	{1920, 1080}
	};
	
	paintBackground(); // clear framebuffer
	xRes = iaVideoSystems[format][0];
	yRes = iaVideoSystems[format][1];
	bpp = 32;
	stride = xRes * bpp / 8;
}
#endif

