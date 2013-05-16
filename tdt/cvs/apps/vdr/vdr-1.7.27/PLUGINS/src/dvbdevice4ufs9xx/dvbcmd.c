/*
 * hdffcmd.c: TODO(short description)
 *
 * See the README file for copyright information and how to reach the author.
 *
 * $Id: hdffcmd.c 1.1 2009/12/29 14:30:08 kls Exp $
 */

#include "dvbcmd.h"
#include <linux/dvb/osd.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <vdr/tools.h>


namespace DVB
{

cDvbCmdIf::cDvbCmdIf(int OsdDev)
{
    mOsdDev = OsdDev;
    if (mOsdDev < 0)
    {
        //printf("ERROR: invalid OSD device handle (%d)!\n", mOsdDev);
    }
}

cDvbCmdIf::~cDvbCmdIf(void)
{
}

void cDvbCmdIf::CmdAvSetVideoFormat(uint8_t DecoderIndex, const tVideoFormat * pVideoFormat)
{
    printf("AvSetVideoFormat %d %d %d\n", DecoderIndex, pVideoFormat->TvFormat, pVideoFormat->VideoDisplay);

    /*cmdBuf.SetBits(4, DecoderIndex);
    if (pVideoFormat->AutomaticEnabled)
    {
        cmdBuf.SetBits(1, 1);
    }
    else
    {
        cmdBuf.SetBits(1, 0);
    }
    if (pVideoFormat->AfdEnabled)
    {
        cmdBuf.SetBits(1, 1);
    }
    else
    {
        cmdBuf.SetBits(1, 0);
    }
    cmdBuf.SetBits(2, pVideoFormat->TvFormat);
    cmdBuf.SetBits(8, pVideoFormat->VideoConversion);*/

  int   fd;

  const char* sAspect[] = {
     "4:3", // TvFormat16_9=0
     "16:9"
     };

  fd = open("/proc/stb/video/aspect", O_WRONLY);
  write(fd, sAspect[pVideoFormat->TvFormat], strlen(sAspect[pVideoFormat->TvFormat]));
  close(fd);

  const char* sMode[] = {
     "panscan",
     "letterbox",
     "bestfit",    //centercutout?
     "non"
     };

  fd = open("/proc/stb/video/policy", O_WRONLY);
  write(fd, sMode[pVideoFormat->VideoDisplay],strlen((const char*) sMode[pVideoFormat->VideoDisplay]));
  close(fd);

  isyslog("[%s, %s, %d] Set VideoDisplay (TvFormat): %d (%d)\n", __FILE__, __func__, __LINE__, pVideoFormat->VideoDisplay, pVideoFormat->TvFormat);
}

void cDvbCmdIf::CmdHdmiSetVideoMode(eHdmiVideoMode VideoMode)
{
    printf("HdmiSetVideoMode %d\n", VideoMode);

    //cmdBuf.SetBits(8, VideoMode);

//void cDvbHdDevice::SetResolution(int Resolution)
  int Resolution = VideoMode;
  //cDevice::SetVideoSystem(Resolution);

  const char* sResolution[] = {
     "pal",
     "480p",
     "576p50",
     "720p60",
     "1080i60",
     "720p50",
     "1080i50",
     "1080p30",
     "1080p24",
     "1080p25"
     };

  int fd = open("/proc/stb/video/videomode", O_RDWR);
  isyslog("[%s, %s, %d] Set Resolution: %d (%s)\n", __FILE__, __func__, __LINE__, Resolution, sResolution[Resolution]);
  write(fd, sResolution[Resolution], strlen(sResolution[Resolution]));
  close(fd);
}

} // end of namespace
