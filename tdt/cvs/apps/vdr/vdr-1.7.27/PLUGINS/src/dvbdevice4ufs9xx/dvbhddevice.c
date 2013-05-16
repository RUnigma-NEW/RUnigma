/*
 * dvbhddevice.c: The DVB HD device interface
 *
 * See the README file for copyright information and how to reach the author.
 *
 * $Id: dvbhddevice.c $
 */

#include "dvbhddevice.h"
#include "setup.h"
#include <errno.h>
#include <limits.h>
#include <linux/videodev2.h>
#include <linux/dvb/audio.h>
#include <linux/dvb/dmx.h>
#include <linux/dvb/video.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#ifdef PLATFORM_STB71XX
#include <linux/fb.h>
#endif
#include "dvbfbosd.h"
#include "vdr/eitscan.h"
#include "vdr/transfer.h"

#ifdef PLATFORM_STB71XX
#define VIDEO_SET_ENCODING              _IO('o',  81)
#define AUDIO_SET_ENCODING              _IO('o',  70)
#endif

// --- cDvbHdDevice --------------------------------------------------------

int cDvbHdDevice::devVideoOffset = -1;
int cDvbHdDevice::devDvbOffset = -1;

cDvbHdDevice::cDvbHdDevice(int Adapter, int Frontend)
:cDvbDevice(Adapter, Frontend)
{
  spuDecoder = NULL;
  digitalAudio = false;
  playMode = pmNone;
  mDvbCmdIf = NULL;

  // Devices that are only present on cards with decoders:

  fd_osd      = DvbOpen(DEV_DVB_OSD,    adapter, frontend, O_RDWR);
  fd_video    = DvbOpen(DEV_DVB_VIDEO,  adapter, frontend, O_RDWR | O_NONBLOCK);
  fd_audio    = DvbOpen(DEV_DVB_AUDIO,  adapter, frontend, O_RDWR | O_NONBLOCK);
  fd_stc      = DvbOpen(DEV_DVB_DEMUX,  adapter, frontend, O_RDWR);

#ifdef PLATFORM_STB71XX
  int card = DMX_SOURCE_FRONT0 + Adapter;       // ? Adapter/Frontend ???
  if (ioctl(fd_stc, DMX_SET_SOURCE, &card) < 0)
     esyslog("DMX_SET_SOURCE failed");
#endif

  // The offset of the /dev/video devices:

esyslog("adapter=%d frontend=%d Adapter=%d Frontend=%d", adapter, frontend, Adapter, Frontend);

esyslog("devVideo Index=%d Offset=%d", devVideoIndex, devVideoOffset);
  if (devVideoOffset < 0) { // the first one checks this
     FILE *f = NULL;
     char buffer[PATH_MAX];
     for (int ofs = 0; ofs < 100; ofs++) {
         snprintf(buffer, sizeof(buffer), "/proc/video/dev/video%d", ofs);
         if ((f = fopen(buffer, "r")) != NULL) {
            if (fgets(buffer, sizeof(buffer), f)) {
               if (strstr(buffer, "DVB Board")) { // found the _first_ DVB card
                  devVideoOffset = ofs;
                  dsyslog("video device offset is %d", devVideoOffset);
                  break;
                  }
               }
            else
               break;
            fclose(f);
            }
         else
            break;
         }
     if (devVideoOffset < 0)
        devVideoOffset = 0;
     if (f)
        fclose(f);
     }
  devVideoIndex = devVideoOffset >= 0 ? devVideoOffset++ : -1;
esyslog("devVideo Index=%d Offset=%d", devVideoIndex, devVideoOffset);

  isDvbPrimary = false;
  if (devDvbOffset < 0) {
     devDvbOffset = adapter;
     isDvbPrimary = true;
     mDvbCmdIf = new DVB::cDvbCmdIf(fd_osd);
     mDvbCmdIf->CmdHdmiSetVideoMode(gDvbSetup.GetVideoMode());
     }

  // Video format:

  SetVideoFormat(Setup.VideoFormat);
}

cDvbHdDevice::~cDvbHdDevice()
{
  delete spuDecoder;
  if (isDvbPrimary)
     delete mDvbCmdIf;
  // We're not explicitly closing any device files here, since this sometimes
  // caused segfaults. Besides, the program is about to terminate anyway...
}

void cDvbHdDevice::MakePrimaryDevice(bool On)
{
  if (On)
     new cDvbOsdProvider(fd_osd);
  cDvbDevice::MakePrimaryDevice(On);
}

bool cDvbHdDevice::HasDecoder(void) const
{
  //return true;
  return isDvbPrimary;
}

cSpuDecoder *cDvbHdDevice::GetSpuDecoder(void)
{
  if (!spuDecoder && IsPrimaryDevice())
     spuDecoder = new cDvbSpuDecoder();
  return spuDecoder;
}

uchar *cDvbHdDevice::GrabImage(int &Size, bool Jpeg, int Quality, int SizeX, int SizeY)
{
  if (devVideoIndex < 0)
     return NULL;
  char buffer[PATH_MAX];
  snprintf(buffer, sizeof(buffer), "%s%d", DEV_VIDEO, devVideoIndex);
  int videoDev = open(buffer, O_RDWR);
  if (videoDev >= 0) {
     uchar *result = NULL;
     // set up the size and RGB
     v4l2_format fmt;
     memset(&fmt, 0, sizeof(fmt));
     fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
     fmt.fmt.pix.width = SizeX;
     fmt.fmt.pix.height = SizeY;
     fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_BGR24;
     fmt.fmt.pix.field = V4L2_FIELD_ANY;
     if (ioctl(videoDev, VIDIOC_S_FMT, &fmt) == 0) {
        v4l2_requestbuffers reqBuf;
        memset(&reqBuf, 0, sizeof(reqBuf));
        reqBuf.count = 2;
        reqBuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        reqBuf.memory = V4L2_MEMORY_MMAP;
        if (ioctl(videoDev, VIDIOC_REQBUFS, &reqBuf) >= 0) {
           v4l2_buffer mbuf;
           memset(&mbuf, 0, sizeof(mbuf));
           mbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
           mbuf.memory = V4L2_MEMORY_MMAP;
           if (ioctl(videoDev, VIDIOC_QUERYBUF, &mbuf) == 0) {
              int msize = mbuf.length;
              unsigned char *mem = (unsigned char *)mmap(0, msize, PROT_READ | PROT_WRITE, MAP_SHARED, videoDev, 0);
              if (mem && mem != (unsigned char *)-1) {
                 v4l2_buffer buf;
                 memset(&buf, 0, sizeof(buf));
                 buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                 buf.memory = V4L2_MEMORY_MMAP;
                 buf.index = 0;
                 if (ioctl(videoDev, VIDIOC_QBUF, &buf) == 0) {
                    v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                    if (ioctl (videoDev, VIDIOC_STREAMON, &type) == 0) {
                       memset(&buf, 0, sizeof(buf));
                       buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                       buf.memory = V4L2_MEMORY_MMAP;
                       buf.index = 0;
                       if (ioctl(videoDev, VIDIOC_DQBUF, &buf) == 0) {
                          if (ioctl(videoDev, VIDIOC_STREAMOFF, &type) == 0) {
                             // make RGB out of BGR:
                             int memsize = fmt.fmt.pix.width * fmt.fmt.pix.height;
                             unsigned char *mem1 = mem;
                             for (int i = 0; i < memsize; i++) {
                                 unsigned char tmp = mem1[2];
                                 mem1[2] = mem1[0];
                                 mem1[0] = tmp;
                                 mem1 += 3;
                                 }

                             if (Quality < 0)
                                Quality = 100;

                             dsyslog("grabbing to %s %d %d %d", Jpeg ? "JPEG" : "PNM", Quality, fmt.fmt.pix.width, fmt.fmt.pix.height);
                             if (Jpeg) {
                                // convert to JPEG:
                                result = RgbToJpeg(mem, fmt.fmt.pix.width, fmt.fmt.pix.height, Size, Quality);
                                if (!result)
                                   esyslog("ERROR: failed to convert image to JPEG");
                                }
                             else {
                                // convert to PNM:
                                char buf[32];
                                snprintf(buf, sizeof(buf), "P6\n%d\n%d\n255\n", fmt.fmt.pix.width, fmt.fmt.pix.height);
                                int l = strlen(buf);
                                int bytes = memsize * 3;
                                Size = l + bytes;
                                result = MALLOC(uchar, Size);
                                if (result) {
                                   memcpy(result, buf, l);
                                   memcpy(result + l, mem, bytes);
                                   }
                                else
                                   esyslog("ERROR: failed to convert image to PNM");
                                }
                             }
                          else
                             esyslog("ERROR: video device VIDIOC_STREAMOFF failed");
                          }
                       else
                          esyslog("ERROR: video device VIDIOC_DQBUF failed");
                       }
                    else
                       esyslog("ERROR: video device VIDIOC_STREAMON failed");
                    }
                 else
                    esyslog("ERROR: video device VIDIOC_QBUF failed");
                 munmap(mem, msize);
                 }
              else
                 esyslog("ERROR: failed to memmap video device");
              }
           else
              esyslog("ERROR: video device VIDIOC_QUERYBUF failed");
           }
        else
           esyslog("ERROR: video device VIDIOC_REQBUFS failed");
        }
     else
        esyslog("ERROR: video device VIDIOC_S_FMT failed");
     close(videoDev);
     return result;
     }
  else
     LOG_ERROR_STR(buffer);
  return NULL;
}

#ifdef xPLATFORM_STB71XX
void cDvbHdDevice::SetResolution(int Resolution)
{
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

#endif
#ifdef REMOVED
void cDvbHdDevice::SetVideoDisplay(eVideoDisplay VideoDisplay)
{
  //cDevice::SetVideoDisplay(VideoDisplay);
#ifdef PLATFORM_STB71XX
  int   fd;
  const char* sMode[] = {
     "panscan",
     "letterbox",
     "bestfit",    //centercutout?
     "non"
     };

  fd = open("/proc/stb/video/policy", O_WRONLY);

  isyslog("[%s, %s, %d] Set VideoDisplay (TvFormat): %d (%d)\n", __FILE__, __func__, __LINE__, VideoDisplay, gDvbSetup.TvFormat);
  write(fd, sMode[VideoDisplay],strlen((const char*) sMode[VideoDisplay]));

  close(fd);
#else
  if (Setup.VideoFormat) {
     CHECK(ioctl(fd_video, VIDEO_SET_DISPLAY_FORMAT, VIDEO_LETTER_BOX));
     }
  else {
     switch (VideoDisplayFormat) {
       case vdfPanAndScan:
            CHECK(ioctl(fd_video, VIDEO_SET_DISPLAY_FORMAT, VIDEO_PAN_SCAN));
            break;
       case vdfLetterBox:
            CHECK(ioctl(fd_video, VIDEO_SET_DISPLAY_FORMAT, VIDEO_LETTER_BOX));
            break;
       case vdfCenterCutOut:
            CHECK(ioctl(fd_video, VIDEO_SET_DISPLAY_FORMAT, VIDEO_CENTER_CUT_OUT));
            break;
       default: esyslog("ERROR: unknown video display format %d", VideoDisplayFormat);
       }
     }
#endif
}

void cDvbHdDevice::SetTvFormat(bool TvFormat16_9)
{
#ifdef PLATFORM_STB71XX
  int   fd;
  const char* sAspect[] = {
     "4:3", // TvFormat16_9=0
     "16:9"
     };

  fd = open("/proc/stb/video/aspect", O_WRONLY);
  write(fd, sAspect[TvFormat16_9], strlen(sAspect[TvFormat16_9]));
  close(fd);
#else
  CHECK(ioctl(fd_video, VIDEO_SET_FORMAT, TvFormat16_9 ? VIDEO_FORMAT_16_9 : VIDEO_FORMAT_4_3));
#endif
  SetVideoDisplay(eVideoDisplay(gDvbSetup.VideoDisplay));
}
#endif // REMOVED

void cDvbHdDevice::SetVideoDisplayFormat(eVideoDisplayFormat VideoDisplayFormat)
{
  //TODO???
  cDevice::SetVideoDisplayFormat(VideoDisplayFormat);
}

void cDvbHdDevice::SetVideoFormat(bool VideoFormat16_9)
{
  DVB::tVideoFormat videoFormat;

  videoFormat.TvFormat = (DVB::eTvFormat) gDvbSetup.TvFormat;
  videoFormat.VideoDisplay = (DVB::eVideoDisplay) gDvbSetup.VideoDisplay;

  mDvbCmdIf->CmdAvSetVideoFormat(0, &videoFormat);
}

eVideoSystem cDvbHdDevice::GetVideoSystem(void)
{
  eVideoSystem VideoSystem = vsPAL;
  if (fd_video >= 0) {
     video_size_t vs;
     if (ioctl(fd_video, VIDEO_GET_SIZE, &vs) == 0) {
        if (vs.h == 480 || vs.h == 240)
           VideoSystem = vsNTSC;
        }
     else
        LOG_ERROR;
     }
  return VideoSystem;
}

void cDvbHdDevice::GetVideoSize(int &Width, int &Height, double &VideoAspect)
{
  if (fd_video >= 0) {
     video_size_t vs;
     if (ioctl(fd_video, VIDEO_GET_SIZE, &vs) == 0) {
        Width = vs.w;
        Height = vs.h;
        switch (vs.aspect_ratio) {
          default:
          case VIDEO_FORMAT_4_3:   VideoAspect =  4.0 / 3.0; break;
          case VIDEO_FORMAT_16_9:  VideoAspect = 16.0 / 9.0; break;
          case VIDEO_FORMAT_221_1: VideoAspect =       2.21; break;
          }
        return;
        }
     else
        LOG_ERROR;
     }
  cDevice::GetVideoSize(Width, Height, VideoAspect);
}

#ifdef REMOVED
void cDvbHdDevice::GetOsdSize(int &Width, int &Height, double &PixelAspect)
{
#ifdef PLATFORM_STB71XX
  int fd;
  struct fb_var_screeninfo vinfo;
  //int screensize;
  //int bytes_per_line;

  fd = open("/dev/fb0", O_RDWR);
  if (fd >= 0) {
     //isyslog("fb0 handle (%d)", fd);

     if (ioctl(fd, FBIOGET_VSCREENINFO, &vinfo)) {
        esyslog("Error reading variable information.\n");
        exit(1);
     }
     Width = vinfo.xres;
     Height = vinfo.yres;
     //screensize = vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8;
     //bytes_per_line = (vinfo.bits_per_pixel / 8) * Width;
     //isyslog("fb0 info: %d %d %d %d\n", screensize, Width, Height, bytes_per_line);

     PixelAspect = 4.0 / 3.0;
     PixelAspect /= double(Width) / Height;

     if (close(fd) != 0)
        esyslog("Close failed.\n");

     return;
  }
#else
  if (fd_video >= 0) {
     video_size_t vs;
     if (ioctl(fd_video, VIDEO_GET_SIZE, &vs) == 0) {
        Width = 720;
        if (vs.h != 480 && vs.h != 240)
           Height = 576; // PAL
        else
           Height = 480; // NTSC
        switch (Setup.VideoFormat ? vs.aspect_ratio : VIDEO_FORMAT_4_3) {
          default:
          case VIDEO_FORMAT_4_3:   PixelAspect =  4.0 / 3.0; break;
          case VIDEO_FORMAT_221_1: // FF DVB cards only distinguish between 4:3 and 16:9
          case VIDEO_FORMAT_16_9:  PixelAspect = 16.0 / 9.0; break;
          }
        PixelAspect /= double(Width) / Height;
        return;
        }
     else
        LOG_ERROR;
     }
#endif
  cDevice::GetOsdSize(Width, Height, PixelAspect);
}
#endif // REMOVED

void cDvbHdDevice::GetOsdSize(int &Width, int &Height, double &PixelAspect)
{
  gDvbSetup.GetOsdSize(Width, Height, PixelAspect);
}

#define VIDEO_STREAMTYPE_MPEG2 2
#define VIDEO_STREAMTYPE_H264  8
#define AUDIO_STREAMTYPE_MPEG2 4
#define AUDIO_STREAMTYPE_AC3  6

/** 
  * SetVideoStreamtype()
  * Calls the LinuxDvbApi to set the Video Encoding, MPEG2 or H264.
  * int VideoStreamtype: Represents the streamtype as defined in pat/pmt
  */
void cDvbHdDevice::SetVideoStreamtype(int VideoStreamtype)
{
  //video streamtype 2=mpeg2 27=h264
  const int MPEG2 = 2;
  const int H264 = 27;

  int streamtype = VIDEO_STREAMTYPE_MPEG2;
  if (VideoStreamtype == MPEG2)
     streamtype = VIDEO_STREAMTYPE_MPEG2;
  else if (VideoStreamtype == H264)
     streamtype = VIDEO_STREAMTYPE_H264;

  if (videoStreamType != streamtype) {
     videoStreamType = streamtype;
     dsyslog("cDvbHdDevice::%s:%d change VideoStreamType=%d\n", __func__, __LINE__, videoStreamType);
     CHECK(ioctl(fd_video, VIDEO_SET_STREAMTYPE, videoStreamType));
     }
}

bool cDvbHdDevice::SetAudioBypass(bool On)
{
#ifdef PLATFORM_STB71XX
  dsyslog("cDvbHdDevice::%s:%d %s\n", __func__, __LINE__, On ? "On" : "Off");
#endif
  if (setTransferModeForDolbyDigital != 1)
     return false;
  return ioctl(fd_audio, AUDIO_SET_BYPASS_MODE, On) == 0;
}

//                                   ptAudio        ptVideo        ptPcr        ptTeletext        ptDolby        ptOther
static dmx_pes_type_t PesTypes[] = { DMX_PES_AUDIO, DMX_PES_VIDEO, DMX_PES_PCR, DMX_PES_TELETEXT, DMX_PES_OTHER, DMX_PES_OTHER };

bool cDvbHdDevice::SetPid(cPidHandle *Handle, int Type, bool On)
{
#ifdef PLATFORM_STB71XX
  dsyslog("cDvbHdDevice::%s:%d Type=%d %s -> %d\n", __func__, __LINE__, Type, On ? "On" : "Off", Handle->pid);
#endif
  if (Handle->pid) {
     dmx_pes_filter_params pesFilterParams;
     memset(&pesFilterParams, 0, sizeof(pesFilterParams));
     if (On) {
        if (Handle->handle < 0) {
           Handle->handle = DvbOpen(DEV_DVB_DEMUX, adapter, frontend, O_RDWR | O_NONBLOCK, true);
           if (Handle->handle < 0) {
              LOG_ERROR;
              return false;
              }
           }
        pesFilterParams.pid     = Handle->pid;
        pesFilterParams.input   = DMX_IN_FRONTEND;
        pesFilterParams.output  = (Type <= ptTeletext && Handle->used <= 1) ? DMX_OUT_DECODER : DMX_OUT_TS_TAP;
        //pesFilterParams.output  = DMX_OUT_DECODER;
        pesFilterParams.pes_type= PesTypes[Type < ptOther ? Type : ptOther];
        pesFilterParams.flags   = DMX_IMMEDIATE_START;
        //pesFilterParams.flags   = 0;
        if (ioctl(Handle->handle, DMX_SET_PES_FILTER, &pesFilterParams) < 0) {
           LOG_ERROR;
           return false;
           }
        }
     else if (!Handle->used) {
        CHECK(ioctl(Handle->handle, DMX_STOP));
        if (Type <= ptTeletext) {
           pesFilterParams.pid     = 0x1FFF;
           pesFilterParams.input   = DMX_IN_FRONTEND;
           pesFilterParams.output  = DMX_OUT_DECODER;
           pesFilterParams.pes_type= PesTypes[Type];
           pesFilterParams.flags   = DMX_IMMEDIATE_START;
           CHECK(ioctl(Handle->handle, DMX_SET_PES_FILTER, &pesFilterParams));
           if (PesTypes[Type] == DMX_PES_VIDEO) // let's only do this once
              SetPlayMode(pmNone); // necessary to switch a PID from DMX_PES_VIDEO/AUDIO to DMX_PES_OTHER
           }
        close(Handle->handle);
        Handle->handle = -1;
        }
     }
  return true;
}

void cDvbHdDevice::TurnOffLiveMode(bool LiveView)
{
#ifdef PLATFORM_STB71XX
  dsyslog("cDvbHdDevice::%s:%d - LiveView=%d\n", __func__, __LINE__, LiveView);
#endif
  if (LiveView) {
     StopAudio();
     StopVideo(true);
     // Avoid noise while switching:
     CHECK(ioctl(fd_audio, AUDIO_SET_MUTE, true));
     CHECK(ioctl(fd_video, VIDEO_SET_BLANK, true));
#ifdef PLATFORM_STB71XX
#else
     CHECK(ioctl(fd_audio, AUDIO_CLEAR_BUFFER));
     CHECK(ioctl(fd_video, VIDEO_CLEAR_BUFFER));
#endif
     }

  // Turn off live PIDs:

  DetachAll(pidHandles[ptAudio].pid);
  DetachAll(pidHandles[ptVideo].pid);
  DetachAll(pidHandles[ptPcr].pid);
  DetachAll(pidHandles[ptTeletext].pid);
  DelPid(pidHandles[ptAudio].pid);
  DelPid(pidHandles[ptVideo].pid);
  DelPid(pidHandles[ptPcr].pid, ptPcr);
  DelPid(pidHandles[ptTeletext].pid);
  DelPid(pidHandles[ptDolby].pid);
}

bool cDvbHdDevice::SetChannelDevice(const cChannel *Channel, bool LiveView)
{
#ifdef PLATFORM_STB71XX
  dsyslog("cDvbHdDevice::%s:%d \n", __func__, __LINE__);
#endif
  int apid = Channel->Apid(0);
  int vpid = Channel->Vpid();
  int dpid = Channel->Dpid(0);

  bool DoTune = !IsTunedToTransponder(Channel);

  bool pidHandlesVideo = pidHandles[ptVideo].pid == vpid;
  bool pidHandlesAudio = pidHandles[ptAudio].pid == apid;

  bool TurnOffLivePIDs = DoTune
                         || !IsPrimaryDevice()
                         || LiveView // for a new live view the old PIDs need to be turned off
                         || pidHandlesVideo // for recording the PIDs must be shifted from DMX_PES_AUDIO/VIDEO to DMX_PES_OTHER
                         ;

  bool StartTransferMode = IsPrimaryDevice() && !DoTune
                           && (LiveView && HasPid(vpid ? vpid : apid) && (!pidHandlesVideo || (!pidHandlesAudio && (dpid ? pidHandles[ptAudio].pid != dpid : true)))// the PID is already set as DMX_PES_OTHER
                              || !LiveView && (pidHandlesVideo || pidHandlesAudio) // a recording is going to shift the PIDs from DMX_PES_AUDIO/VIDEO to DMX_PES_OTHER
                              );
  if (CamSlot() && !ChannelCamRelations.CamDecrypt(Channel->GetChannelID(), CamSlot()->SlotNumber()))
     StartTransferMode |= LiveView && IsPrimaryDevice() && Channel->Ca() >= CA_ENCRYPTED_MIN;

  bool TurnOnLivePIDs = !StartTransferMode && LiveView;

  // Turn off live PIDs if necessary:

  if (TurnOffLivePIDs)
     TurnOffLiveMode(LiveView);

  // Set the tuner:

  if (!cDvbDevice::SetChannelDevice(Channel, LiveView))
     return false;

  // If this channel switch was requested by the EITScanner we don't wait for
  // a lock and don't set any live PIDs (the EITScanner will wait for the lock
  // by itself before setting any filters):

#if APIVERSNUM >= 10726
#else
  if (EITScanner.UsesDevice(this)) //XXX
     return true;
#endif

  // PID settings:

  if (TurnOnLivePIDs) {
     SetAudioBypass(false);

     esyslog("TurnOnLivePIDs 1");

     if (!(AddPid(Channel->Ppid(), ptPcr) && AddPid(vpid, ptVideo) && AddPid(apid, ptAudio))) {
        esyslog("ERROR: failed to set PIDs for channel %d on device %d", Channel->Number(), CardIndex() + 1);
        return false;
        }
     if (IsPrimaryDevice())
        AddPid(Channel->Tpid(), ptTeletext);

     esyslog("TurnOnLivePIDs 2");
     //SetStreamContentType(Channel);
     //ChangeStream(true, false);
#ifndef PLATFORM_STB71XX
     CHECK(ioctl(fd_audio, AUDIO_SET_MUTE, true)); // actually one would expect 'false' here, but according to Marco Schlüßler <marco@lordzodiac.de> this works
                                                   // to avoid missing audio after replaying a DVD; with 'false' there is an audio disturbance when switching
                                                   // between two channels on the same transponder on DVB-S
     CHECK(ioctl(fd_audio, AUDIO_SET_AV_SYNC, true));
#endif
     }
  else if (StartTransferMode)
     cControl::Launch(new cTransferControl(this, Channel));

  // Hopefully setting the Video Streamtype here is correct.
  dsyslog("Channel->Vtype(): %d\n", Channel->Vtype());
  SetVideoStreamtype(Channel->Vtype());

  // Determining the audio type
  int atype = 4;   // MPEG1
  if (apid == 0) { // no regular pid
     apid = Channel->Dpid(0);
     atype = 6; // AC3
  }
  audioTrackType = atype;
  CHECK(ioctl(fd_audio, AUDIO_SET_ENCODING, audioTrackType));

  ChangeStream(true, false);

  return true;
}

void cDvbHdDevice::ChangeStream(bool Encoding, bool AudioOnly)
{
/* dirty quick hack */
  //if (!AudioOnly)
     CHECK(ioctl(fd_video, VIDEO_SELECT_SOURCE, VIDEO_SOURCE_DEMUX));
  CHECK(ioctl(fd_audio, AUDIO_SELECT_SOURCE, AUDIO_SOURCE_DEMUX));

  dsyslog("videoStreamType, audioTrackType: %d, %d\n", videoStreamType, audioTrackType);
  if (Encoding) {
     CHECK(ioctl(fd_video, VIDEO_SET_ENCODING, videoStreamType));
     CHECK(ioctl(fd_audio, AUDIO_SET_ENCODING, audioTrackType));
     }

  //if (!AudioOnly)
     CHECK(ioctl(fd_video, VIDEO_PLAY));
  CHECK(ioctl(fd_audio, AUDIO_PLAY));

  CHECK(ioctl(fd_audio, AUDIO_SET_MUTE, true)); // actually one would expect 'false' here, but according to Marco Schler <marco@lordzodiac.de> this works
                                                // to avoid missing audio after replaying a DVD; with 'false' there is an audio disturbance when switching
                                                // between two channels on the same transponder on DVB-S
  CHECK(ioctl(fd_audio, AUDIO_SET_AV_SYNC, true));
}

void cDvbHdDevice::StopAudio(void)
{
  if (fd_audio < 0) {
     fd_audio = DvbOpen(DEV_DVB_AUDIO, adapter, frontend, O_RDWR | O_NONBLOCK);
     }
  CHECK(ioctl(fd_audio, AUDIO_STOP, true));
}

void cDvbHdDevice::StopVideo(bool blank)
{
  if ( fd_video < 0 ) {
    fd_video = DvbOpen(DEV_DVB_VIDEO, adapter, frontend, O_RDWR | O_NONBLOCK);
    }
  CHECK(ioctl(fd_video, VIDEO_STOP, true));
  if(blank) CHECK(ioctl(fd_video, VIDEO_SET_BLANK, true));
}

int cDvbHdDevice::GetAudioChannelDevice(void)
{
  audio_status_t as;
  CHECK(ioctl(fd_audio, AUDIO_GET_STATUS, &as));
  return as.channel_select;
}

void cDvbHdDevice::SetAudioChannelDevice(int AudioChannel)
{
#ifdef PLATFORM_STB71XX
  dsyslog("cDvbHdDevice::%s:%d \n", __func__, __LINE__);
#endif
  CHECK(ioctl(fd_audio, AUDIO_CHANNEL_SELECT, AudioChannel));
}

void cDvbHdDevice::SetVolumeDevice(int Volume)
{
#ifdef PLATFORM_STB71XX
  dsyslog("cDvbHdDevice::%s:%d \n", __func__, __LINE__);
  int left;
  left = 0;
  left = (int)(63-(int)(Volume / 2.55 * 0.63));
  FILE *fd;
  if((fd = fopen("/proc/stb/avs/0/volume", "wb")) == NULL) {
     dsyslog("cannot open Device /proc/stb/avs/0/volume(%m)");
  return;
  }
  fprintf(fd, "%d", left);
  dsyslog("cDvbHdDevice_debug:Volume:%d:left:%d \n", Volume , left);
  fclose(fd);
#else
  if (digitalAudio)
     Volume = 0;
  audio_mixer_t am;
  // conversion for linear volume response:
  am.volume_left = am.volume_right = 2 * Volume - Volume * Volume / 255;
  CHECK(ioctl(fd_audio, AUDIO_SET_MIXER, &am));
#endif
}

void cDvbHdDevice::SetDigitalAudioDevice(bool On)
{
#ifdef PLATFORM_STB71XX
  dsyslog("cDvbHdDevice::%s:%d \n", __func__, __LINE__);
#endif
  if (digitalAudio != On) {
     if (digitalAudio)
        cCondWait::SleepMs(1000); // Wait until any leftover digital data has been flushed
     digitalAudio = On;
     SetVolumeDevice(On || IsMute() ? 0 : CurrentVolume());
     }
}

void cDvbHdDevice::SetAudioTrackDevice(eTrackType Type)
{
#ifdef PLATFORM_STB71XX
  dsyslog("cDvbHdDevice::%s:%d TrackType=%d\n", __func__, __LINE__, Type);
#endif
  const tTrackId *TrackId = GetTrack(Type);
  int tracktype = (IS_DOLBY_TRACK(Type)) ? AUDIO_STREAMTYPE_AC3 : AUDIO_STREAMTYPE_MPEG2;
  if (audioTrackType != tracktype) {
     audioTrackType = tracktype;
     ChangeStream(false, true);
     }
  else {
     ChangeStream(false, false);
     }
  if (TrackId && TrackId->id) {
     SetAudioBypass(false);
     if (IS_AUDIO_TRACK(Type) || (IS_DOLBY_TRACK(Type) && SetAudioBypass(true))) {
        if (pidHandles[ptAudio].pid && pidHandles[ptAudio].pid != TrackId->id) {
           DetachAll(pidHandles[ptAudio].pid);
           if (CamSlot())
              CamSlot()->SetPid(pidHandles[ptAudio].pid, false);
           pidHandles[ptAudio].pid = TrackId->id;
           SetPid(&pidHandles[ptAudio], ptAudio, true);
           if (CamSlot()) {
              CamSlot()->SetPid(pidHandles[ptAudio].pid, true);
              CamSlot()->StartDecrypting();
              }
           }
        }
     else if (IS_DOLBY_TRACK(Type)) {
        if (setTransferModeForDolbyDigital == 0)
           return;
        // Currently this works only in Transfer Mode
        ForceTransferMode();
        }
     }
}

bool cDvbHdDevice::CanReplay(void) const
{
  return cDevice::CanReplay();
}

bool cDvbHdDevice::SetPlayMode(ePlayMode PlayMode)
{
  if (PlayMode != pmExtern_THIS_SHOULD_BE_AVOIDED && fd_video < 0 && fd_audio < 0) {
     // reopen the devices
     fd_video = DvbOpen(DEV_DVB_VIDEO, adapter, frontend, O_RDWR | O_NONBLOCK);
     fd_audio = DvbOpen(DEV_DVB_AUDIO, adapter, frontend, O_RDWR | O_NONBLOCK);
     SetVideoFormat(Setup.VideoFormat);
     }

  switch (PlayMode) {
    case pmNone:
#ifdef PLATFORM_STB71XX
         dsyslog("cDvbHdDevice::%s:%d PlayMode=None\n", __func__, __LINE__);
#endif
         // special handling to return from PCM replay:
         CHECK(ioctl(fd_video, VIDEO_SET_BLANK, true));
         CHECK(ioctl(fd_video, VIDEO_SELECT_SOURCE, VIDEO_SOURCE_MEMORY));
         CHECK(ioctl(fd_video, VIDEO_PLAY));

         CHECK(ioctl(fd_video, VIDEO_STOP, true));
         CHECK(ioctl(fd_audio, AUDIO_STOP, true));
#ifndef PLATFORM_STB71XX
         CHECK(ioctl(fd_video, VIDEO_CLEAR_BUFFER));
         CHECK(ioctl(fd_audio, AUDIO_CLEAR_BUFFER));
#endif
         CHECK(ioctl(fd_video, VIDEO_SELECT_SOURCE, VIDEO_SOURCE_DEMUX));
         CHECK(ioctl(fd_audio, AUDIO_SELECT_SOURCE, AUDIO_SOURCE_DEMUX));
         CHECK(ioctl(fd_audio, AUDIO_SET_AV_SYNC, true));
         CHECK(ioctl(fd_audio, AUDIO_SET_MUTE, false));
         break;
    case pmAudioVideo:
    case pmAudioOnlyBlack:
#ifdef PLATFORM_STB71XX
         dsyslog("cDvbHdDevice::%s:%d PlayMode=AudioVideo_or_AudioOnlyBlack\n", __func__, __LINE__);
#endif
         if (playMode == pmNone)
            TurnOffLiveMode(true);
         CHECK(ioctl(fd_video, VIDEO_SET_BLANK, true));
         CHECK(ioctl(fd_audio, AUDIO_SELECT_SOURCE, AUDIO_SOURCE_MEMORY));
         CHECK(ioctl(fd_audio, AUDIO_SET_AV_SYNC, PlayMode == pmAudioVideo));
         CHECK(ioctl(fd_audio, AUDIO_PLAY));
         CHECK(ioctl(fd_video, VIDEO_SELECT_SOURCE, VIDEO_SOURCE_MEMORY));
         CHECK(ioctl(fd_video, VIDEO_PLAY));
         break;
    case pmAudioOnly:
#ifdef PLATFORM_STB71XX
         dsyslog("cDvbHdDevice::%s:%d PlayMode=AudioOnly\n", __func__, __LINE__);
#endif
         CHECK(ioctl(fd_video, VIDEO_SET_BLANK, true));
         CHECK(ioctl(fd_audio, AUDIO_STOP, true));
#ifndef PLATFORM_STB71XX
         CHECK(ioctl(fd_audio, AUDIO_CLEAR_BUFFER));
#endif
         CHECK(ioctl(fd_audio, AUDIO_SELECT_SOURCE, AUDIO_SOURCE_MEMORY));
         CHECK(ioctl(fd_audio, AUDIO_SET_AV_SYNC, false));
         CHECK(ioctl(fd_audio, AUDIO_PLAY));
         CHECK(ioctl(fd_video, VIDEO_SET_BLANK, false));
         break;
    case pmVideoOnly:
#ifdef PLATFORM_STB71XX
         dsyslog("cDvbHdDevice::%s:%d PlayMode=VideoOnly\n", __func__, __LINE__);
#endif
         CHECK(ioctl(fd_video, VIDEO_SET_BLANK, true));
         CHECK(ioctl(fd_video, VIDEO_STOP, true));
         CHECK(ioctl(fd_audio, AUDIO_SELECT_SOURCE, AUDIO_SOURCE_DEMUX));
         CHECK(ioctl(fd_audio, AUDIO_SET_AV_SYNC, false));
         CHECK(ioctl(fd_audio, AUDIO_PLAY));
#ifndef PLATFORM_STB71XX
         CHECK(ioctl(fd_audio, AUDIO_CLEAR_BUFFER));
#endif
         CHECK(ioctl(fd_video, VIDEO_SELECT_SOURCE, VIDEO_SOURCE_MEMORY));
         CHECK(ioctl(fd_video, VIDEO_PLAY));
         break;
    case pmExtern_THIS_SHOULD_BE_AVOIDED:
#ifdef PLATFORM_STB71XX
         dsyslog("cDvbHdDevice::%s:%d PlayMode=Extern\n", __func__, __LINE__);
#endif
         close(fd_video);
         close(fd_audio);
         fd_video = fd_audio = -1;
         break;
    default: esyslog("ERROR: unknown playmode %d", PlayMode);
    }
  playMode = PlayMode;
  return true;
}

int64_t cDvbHdDevice::GetSTC(void)
{
  if (fd_stc >= 0) {
     struct dmx_stc stc;
     stc.num = 0;
     if (ioctl(fd_stc, DMX_GET_STC, &stc) == -1) {
        esyslog("ERROR: stc %d: %m", CardIndex() + 1);
        return -1;
        }
     return stc.stc / stc.base;
     }
  return -1;
}

void cDvbHdDevice::TrickSpeed(int Speed)
{
  if (fd_video >= 0)
     CHECK(ioctl(fd_video, VIDEO_SLOWMOTION, Speed));
}

void cDvbHdDevice::Clear(void)
{
  if (fd_video >= 0)
     CHECK(ioctl(fd_video, VIDEO_CLEAR_BUFFER));
  if (fd_audio >= 0)
     CHECK(ioctl(fd_audio, AUDIO_CLEAR_BUFFER));
  cDevice::Clear();
}

void cDvbHdDevice::Play(void)
{
#ifdef PLATFORM_STB71XX
  dsyslog("cDvbHdDevice::%s:%d\n", __func__, __LINE__);
#endif
  if (playMode == pmAudioOnly || playMode == pmAudioOnlyBlack) {
     if (fd_audio >= 0)
        CHECK(ioctl(fd_audio, AUDIO_CONTINUE));
     }
  else {
     if (fd_audio >= 0) {
        CHECK(ioctl(fd_audio, AUDIO_SET_AV_SYNC, true));
        CHECK(ioctl(fd_audio, AUDIO_CONTINUE));
        }
     if (fd_video >= 0)
        CHECK(ioctl(fd_video, VIDEO_CONTINUE));
     }
  cDevice::Play();
}

void cDvbHdDevice::Freeze(void)
{
  if (playMode == pmAudioOnly || playMode == pmAudioOnlyBlack) {
     if (fd_audio >= 0)
        CHECK(ioctl(fd_audio, AUDIO_PAUSE));
     }
  else {
     if (fd_audio >= 0) {
        CHECK(ioctl(fd_audio, AUDIO_SET_AV_SYNC, false));
        CHECK(ioctl(fd_audio, AUDIO_PAUSE));
        }
     if (fd_video >= 0)
        CHECK(ioctl(fd_video, VIDEO_FREEZE));
     }
  cDevice::Freeze();
}

void cDvbHdDevice::Mute(void)
{
  if (fd_audio >= 0) {
     CHECK(ioctl(fd_audio, AUDIO_SET_AV_SYNC, false));
     CHECK(ioctl(fd_audio, AUDIO_SET_MUTE, true));
     }
  cDevice::Mute();
}

void cDvbHdDevice::StillPicture(const uchar *Data, int Length)
{
  if (!Data || Length < TS_SIZE)
     return;
  if (Data[0] == 0x47) {
     // TS data
     cDevice::StillPicture(Data, Length);
     }
  else if (Data[0] == 0x00 && Data[1] == 0x00 && Data[2] == 0x01 && (Data[3] & 0xF0) == 0xE0) {
     // PES data
     char *buf = MALLOC(char, Length);
     if (!buf)
        return;
     int i = 0;
     int blen = 0;
     while (i < Length - 6) {
           if (Data[i] == 0x00 && Data[i + 1] == 0x00 && Data[i + 2] == 0x01) {
              int len = Data[i + 4] * 256 + Data[i + 5];
              if ((Data[i + 3] & 0xF0) == 0xE0) { // video packet
                 // skip PES header
                 int offs = i + 6;
                 // skip header extension
                 if ((Data[i + 6] & 0xC0) == 0x80) {
                    // MPEG-2 PES header
                    if (Data[i + 8] >= Length)
                       break;
                    offs += 3;
                    offs += Data[i + 8];
                    len -= 3;
                    len -= Data[i + 8];
                    if (len < 0 || offs + len > Length)
                       break;
                    }
                 else {
                    // MPEG-1 PES header
                    while (offs < Length && len > 0 && Data[offs] == 0xFF) {
                          offs++;
                          len--;
                          }
                    if (offs <= Length - 2 && len >= 2 && (Data[offs] & 0xC0) == 0x40) {
                       offs += 2;
                       len -= 2;
                       }
                    if (offs <= Length - 5 && len >= 5 && (Data[offs] & 0xF0) == 0x20) {
                       offs += 5;
                       len -= 5;
                       }
                    else if (offs <= Length - 10 && len >= 10 && (Data[offs] & 0xF0) == 0x30) {
                       offs += 10;
                       len -= 10;
                       }
                    else if (offs < Length && len > 0) {
                       offs++;
                       len--;
                       }
                    }
                 if (blen + len > Length) // invalid PES length field
                    break;
                 memcpy(&buf[blen], &Data[offs], len);
                 i = offs + len;
                 blen += len;
                 }
              else if (Data[i + 3] >= 0xBD && Data[i + 3] <= 0xDF) // other PES packets
                 i += len + 6;
              else
                 i++;
              }
           else
              i++;
           }
     video_still_picture sp = { buf, blen };
     CHECK(ioctl(fd_video, VIDEO_STILLPICTURE, &sp));
     free(buf);
     }
  else {
     // non-PES data
     video_still_picture sp = { (char *)Data, Length };
     CHECK(ioctl(fd_video, VIDEO_STILLPICTURE, &sp));
     }
}

bool cDvbHdDevice::Poll(cPoller &Poller, int TimeoutMs)
{
  Poller.Add((playMode == pmAudioOnly || playMode == pmAudioOnlyBlack) ? fd_audio : fd_video, true);
  return Poller.Poll(TimeoutMs);
}

bool cDvbHdDevice::Flush(int TimeoutMs)
{
  //TODO actually this function should wait until all buffered data has been processed by the card, but how?
  return true;
}

int cDvbHdDevice::PlayVideo(const uchar *Data, int Length)
{
  return WriteAllOrNothing(fd_video, Data, Length, 1000, 10);
}

int cDvbHdDevice::PlayAudio(const uchar *Data, int Length, uchar Id)
{
  return WriteAllOrNothing(fd_audio, Data, Length, 1000, 10);
}

int cDvbHdDevice::PlayTsVideo(const uchar *Data, int Length)
{
  return WriteAllOrNothing(fd_video, Data, Length, 1000, 10);
}

int cDvbHdDevice::PlayTsAudio(const uchar *Data, int Length)
{
  return WriteAllOrNothing(fd_audio, Data, Length, 1000, 10);
}

DVB::cDvbCmdIf *cDvbHdDevice::GetDvbCmdHandler(void)
{
  //TODO why not just keep a pointer?
//esyslog("GetDvbCmdHandler: devVideo Index=%d Offset=%d", devVideoIndex, devVideoOffset);
esyslog("GetDvbCmdHandler: devDvbOffset Offset=%d", devDvbOffset);
  if (devDvbOffset >= 0) {
     cDvbHdDevice *device = (cDvbHdDevice *)GetDevice(devDvbOffset);
     if (device)
        return device->mDvbCmdIf;
     }
  return NULL;
}

// --- cDvbHdDeviceProbe ---------------------------------------------------

bool cDvbHdDeviceProbe::Probe(int Adapter, int Frontend)
{
#ifdef PLATFORM_STB71XX
  // It seems that we dont have these entries, so fall back
  // TODO: This should be changed to be target independend
  if (Adapter > 0 || Frontend > 0 /* FIXME: On DualTuner boxes we got two Fronteds */)
     return false;

  dsyslog("creating cDvbHdDevice %d/%d", Adapter, Frontend);
  new cDvbHdDevice(Adapter, Frontend);

  return true;
#else
  static uint32_t SubsystemIds[] = {
    0x110A0000, // Fujitsu Siemens DVB-C
    0x13C20000, // Technotrend/Hauppauge WinTV DVB-S rev1.X or Fujitsu Siemens DVB-C
    0x13C20001, // Technotrend/Hauppauge WinTV DVB-T rev1.X
    0x13C20002, // Technotrend/Hauppauge WinTV DVB-C rev2.X
    0x13C20003, // Technotrend/Hauppauge WinTV Nexus-S rev2.X
    0x13C20004, // Galaxis DVB-S rev1.3
    0x13C20006, // Fujitsu Siemens DVB-S rev1.6
    0x13C20008, // Technotrend/Hauppauge DVB-T
    0x13C2000A, // Technotrend/Hauppauge WinTV Nexus-CA rev1.X
    0x13C2000E, // Technotrend/Hauppauge WinTV Nexus-S rev2.3
    0x13C21002, // Technotrend/Hauppauge WinTV DVB-S rev1.3 SE
    0x00000000
    };
  cString FileName;
  cReadLine ReadLine;
  FILE *f = NULL;
  uint32_t SubsystemId = 0;
  FileName = cString::sprintf("/sys/class/dvb/dvb%d.frontend%d/device/subsystem_vendor", Adapter, Frontend);
  if ((f = fopen(FileName, "r")) != NULL) {
     if (char *s = ReadLine.Read(f))
        SubsystemId = strtoul(s, NULL, 0) << 16;
     fclose(f);
     }
  FileName = cString::sprintf("/sys/class/dvb/dvb%d.frontend%d/device/subsystem_device", Adapter, Frontend);
  if ((f = fopen(FileName, "r")) != NULL) {
     if (char *s = ReadLine.Read(f))
        SubsystemId |= strtoul(s, NULL, 0);
     fclose(f);
     }
  for (uint32_t *sid = SubsystemIds; *sid; sid++) {
      if (*sid == SubsystemId) {
         dsyslog("creating cDvbHdDevice");
         new cDvbHdDevice(Adapter, Frontend);
         return true;
         }
      }
#endif
  return false; 
}
