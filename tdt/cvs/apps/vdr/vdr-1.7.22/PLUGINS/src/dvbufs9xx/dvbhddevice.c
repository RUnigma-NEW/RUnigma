/*
 * dvbhddevice.c: The DVB HD device interface
 *
 * See the README file for copyright information and how to reach the author.
 *
 * $Id: dvbhddevice.c $
 */

#include "dvbhddevice.h"
#include <errno.h>
#include <limits.h>
#include <linux/videodev2.h>
#include <linux/dvb/audio.h>
#include <linux/dvb/dmx.h>
#include <linux/dvb/video.h>
#include <linux/dvb/ca.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include "dvbfbosd.h"
#include "vdr/eitscan.h"
#include "vdr/transfer.h"
#include "vdr/dvbci.h"

// ST sh4
#include "stm_ioctls.h"


// --- cDvbHdDevice --------------------------------------------------------
cDvbHdDevice::cDvbHdDevice(int Adapter, int Frontend)
:cDvbDevice(Adapter, Frontend)
{
  spuDecoder = NULL;
  digitalAudio = false;
  playMode = pmNone;

  // Devices that are only present on cards with decoders:
  fd_osd      = DvbOpen(DEV_DVB_OSD,    adapter, frontend, O_RDWR);
  fd_video    = DvbOpen(DEV_DVB_VIDEO,  adapter, frontend, O_RDWR | O_NONBLOCK);
  fd_audio    = DvbOpen(DEV_DVB_AUDIO,  adapter, frontend, O_RDWR | O_NONBLOCK);
  fd_stc      = DvbOpen(DEV_DVB_DEMUX,  adapter, frontend, O_RDWR);
  fd_ca 	  = DvbOpen(DEV_DVB_CA, 	adapter, frontend, O_RDWR);

  if(Frontend == 0)
	card = DMX_SOURCE_FRONT0 + Adapter;
  else
	card = DMX_SOURCE_FRONT1 + Adapter;

  ioctl(fd_stc, DMX_SET_SOURCE, &card);
  ioctl(fd_ca, CA_SET_DESCR, 1);

  ciAdapter = cDvbCiAdapter::CreateCiAdapter(this, fd_ca);

  // Video system & format:
  SetVideoSystem(Setup.VideoSystem);
  SetVideoFormat(Setup.VideoFormat);
}

cDvbHdDevice::~cDvbHdDevice()
{
  delete spuDecoder;
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
  return true;
}

cSpuDecoder *cDvbHdDevice::GetSpuDecoder(void)
{
  if (!spuDecoder && IsPrimaryDevice())
     spuDecoder = new cDvbSpuDecoder();
  return spuDecoder;
}

uchar *cDvbHdDevice::GrabImage(int &Size, bool Jpeg, int Quality, int SizeX, int SizeY)
{
  //TODO
  return NULL;
}

void cDvbHdDevice::SetVideoSystem(int VideoSystem)
{
  cDevice::SetVideoSystem(VideoSystem);

  char *sResolution[] = {
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
  isyslog("[%s, %s, %d] Set VideoSystem (Resolution): %d (%s)\n", __FILE__, __func__, __LINE__, VideoSystem, sResolution[VideoSystem]);
  write(fd, sResolution[VideoSystem], strlen(sResolution[VideoSystem]));
  close(fd);
}

void cDvbHdDevice::SetVideoDisplayFormat(eVideoDisplayFormat VideoDisplayFormat)
{
  cDevice::SetVideoDisplayFormat(VideoDisplayFormat);

  char *sMode[] = {
     "panscan",
     "letterbox",
     "bestfit",    //centercutout?
     "non"
     };

  int fd = open("/proc/stb/video/policy", O_WRONLY);
  isyslog("[%s, %s, %d] Set VideoDisplayFormat (Videoformat): %d (%d)\n", __FILE__, __func__, __LINE__, VideoDisplayFormat, Setup.VideoFormat);
  write(fd, sMode[VideoDisplayFormat],strlen((const char*) sMode[VideoDisplayFormat]));
  close(fd);
}

void cDvbHdDevice::SetVideoFormat(bool VideoFormat16_9)
{
  char *sAspect[] = {
     "4:3", // VideoFormat16_9=0
     "16:9"
     };

  int fd = open("/proc/stb/video/aspect", O_WRONLY);
  write(fd, sAspect[VideoFormat16_9], strlen(sAspect[VideoFormat16_9]));
  close(fd);

  SetVideoDisplayFormat(eVideoDisplayFormat(Setup.VideoDisplayFormat));
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

void cDvbHdDevice::GetOsdSize(int &Width, int &Height, double &PixelAspect)
{

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

  cDevice::GetOsdSize(Width, Height, PixelAspect);
}

/*TODO obsolete?
bool cDvbHdDevice::SetAudioBypass(bool On)
{
  if (setTransferModeForDolbyDigital != 1)
     return false;
  return ioctl(fd_audio, AUDIO_SET_BYPASS_MODE, On) == 0;
} */

/** 
  * SetVideoStreamtype()
  * Calls the LinuxDvbApi to set the Video Encoding, MPEG2 or H264.
  * int VideoStreamtype: Represents the streamtype as defined in pat/pmt
  */
bool cDvbHdDevice::SetVideoEncoding(int Vtype)
{
  //dsyslog("cDvbHdDevice::%s:%d check VideoStreamType=%d\n", __func__, __LINE__, Vtype);
  const int MPEG1 = 1;
  const int MPEG2 = 2;
  const int H264 = 27;

  int streamtype = VIDEO_ENCODING_MPEG2;
  switch (Vtype) {
     case MPEG1: streamtype = VIDEO_ENCODING_MPEG1;
                 break;
     case MPEG2: streamtype = VIDEO_ENCODING_MPEG2;
                 break;
     case H264:  streamtype = VIDEO_ENCODING_H264;
                 break;
     }

  if (videoStreamType != streamtype) {
     videoStreamType = streamtype;
     dsyslog("cDvbHdDevice::%s:%d change VideoStreamType=%d\n", __func__, __LINE__, videoStreamType);
     CHECK(ioctl(fd_video, VIDEO_SET_STREAMTYPE, videoStreamType));
     return true;
     }
  return false;
}

bool cDvbHdDevice::SetAudioEncoding(int Atype)
{
  if (audioTrackType != Atype) {
     audioTrackType = Atype;
     dsyslog("cDvbHdDevice::%s:%d change AudioTrackType=%d\n", __func__, __LINE__, audioTrackType);
     return true;
     //ChangeAVEncoding(0, 1); todo switching ?
     }
  return false;
}

void cDvbHdDevice::ChangeAVEncoding(bool Video, bool Audio)
{
  CHECK(ioctl(fd_video, VIDEO_SET_ENCODING, videoStreamType));
  CHECK(ioctl(fd_audio, AUDIO_SET_ENCODING, audioTrackType));
  CHECK(ioctl(fd_video, VIDEO_PLAY));
  CHECK(ioctl(fd_audio, AUDIO_PLAY));
  CHECK(ioctl(fd_audio, AUDIO_SET_AV_SYNC, true));
}

//                                   ptAudio        ptVideo        ptPcr        ptTeletext        ptDolby        ptOther
static dmx_pes_type_t PesTypes[] = { DMX_PES_AUDIO, DMX_PES_VIDEO, DMX_PES_PCR, DMX_PES_TELETEXT, DMX_PES_OTHER, DMX_PES_OTHER };

bool cDvbHdDevice::SetPid(cPidHandle *Handle, int Type, bool On)
{
  dsyslog("cDvbHdDevice::%s:%d Type=%d %s -> %d\n", __func__, __LINE__, Type, On ? "On" : "Off", Handle->pid);

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
        /* ST sh4 ? */
/*	if (Type == ptVideo)
	   //SetVideoEncoding(Handle->streamType); */ // always 0 ?
        if (Type == ptAudio)
           SetAudioEncoding(AUDIO_ENCODING_MPEG1);
        else if (Type == ptDolby)
           SetAudioEncoding(AUDIO_ENCODING_AC3);
        /**/
        pesFilterParams.pid     = Handle->pid;
        pesFilterParams.input   = DMX_IN_FRONTEND;
        pesFilterParams.output  = (Type <= ptTeletext && Handle->used <= 1) ? DMX_OUT_DECODER : DMX_OUT_TS_TAP;
        pesFilterParams.pes_type= PesTypes[Type < ptOther ? Type : ptOther];
        pesFilterParams.flags   = DMX_IMMEDIATE_START;
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
  dsyslog("cDvbHdDevice::%s:%d - LiveView=%d\n", __func__, __LINE__, LiveView);

  if (LiveView) {
     CHECK(ioctl(fd_audio, AUDIO_STOP, true)); ////
     CHECK(ioctl(fd_video, VIDEO_STOP, true)); ////
     // Avoid noise while switching:
     CHECK(ioctl(fd_audio, AUDIO_SET_MUTE, true));
     //CHECK(ioctl(fd_video, VIDEO_SET_BLANK, true));
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
  dsyslog("cDvbHdDevice::%s:%d \n", __func__, __LINE__);

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
  dsyslog("cDvbHdDevice::%s:%d - TurnOnLivePIDs=%d\n", __func__, __LINE__, TurnOnLivePIDs);

  // Turn off live PIDs if necessary:
  if (TurnOffLivePIDs)
     TurnOffLiveMode(LiveView);

  // Set the tuner:
  if (!cDvbDevice::SetChannelDevice(Channel, LiveView))
     return false;

  // If this channel switch was requested by the EITScanner we don't wait for
  // a lock and don't set any live PIDs (the EITScanner will wait for the lock
  // by itself before setting any filters):

  if (EITScanner.UsesDevice(this)) //XXX
     return true;

  // PID settings:

  if (TurnOnLivePIDs) {
     //SetAudioBypass(false);
     if (!(AddPid(Channel->Ppid(), ptPcr) && AddPid(vpid, ptVideo) && AddPid(apid, ptAudio))) {
        esyslog("ERROR: failed to set PIDs for channel %d on device %d", Channel->Number(), CardIndex() + 1);
        return false;
        }
     if (IsPrimaryDevice())
        AddPid(Channel->Tpid(), ptTeletext);
     /* ST sh4 */
     SetVideoEncoding(Channel->Vtype());
     ChangeAVEncoding(1, 1);
     }
  else if (StartTransferMode)
     cControl::Launch(new cTransferControl(this, Channel));

  return true;
}

int cDvbHdDevice::GetAudioChannelDevice(void)
{
  audio_status_t as;
  CHECK(ioctl(fd_audio, AUDIO_GET_STATUS, &as));
  return as.channel_select;
}

void cDvbHdDevice::SetAudioChannelDevice(int AudioChannel)
{
  dsyslog("cDvbHdDevice::%s:%d \n", __func__, __LINE__);

  CHECK(ioctl(fd_audio, AUDIO_CHANNEL_SELECT, AudioChannel));
}

void cDvbHdDevice::SetVolumeDevice(int Volume)
{
  int left = (int)(63-(int)(Volume / 2.55 * 0.63));

  FILE *fd;
  if ((fd = fopen("/proc/stb/avs/0/volume", "wb")) == NULL) {
     dsyslog("cannot open Device /proc/stb/avs/0/volume(%m)");
     return;
     }
  fprintf(fd, "%d", left);
  dsyslog("cDvbHdDevice::%s:%d Volume=%d, left=%d\n", __func__, __LINE__, Volume, left);
  fclose(fd);
}

void cDvbHdDevice::SetDigitalAudioDevice(bool On)
{
  dsyslog("cDvbHdDevice::%s:%d %d\n", __func__, __LINE__,On);

  if(On == 1)
  {
    dsyslog("SetDigitalAudioDevice = ON\n");
    digitalAudio = 1;
    SetAudioEncoding(AUDIO_ENCODING_AC3);
    ChangeAVEncoding(0, 1);
  }
  if (digitalAudio != On) {
     if (digitalAudio)
        cCondWait::SleepMs(1000); // Wait until any leftover digital data has been flushed
     digitalAudio = On;
     SetVolumeDevice(On || IsMute() ? 0 : CurrentVolume());
     }
}

void cDvbHdDevice::SetAudioTrackDevice(eTrackType Type)
{
  dsyslog("cDvbHdDevice::%s:%d TrackType=%d\n", __func__, __LINE__, Type);

  const tTrackId *TrackId = GetTrack(Type);
  if (TrackId && TrackId->id) {
     //SetAudioBypass(false);
     if (IS_AUDIO_TRACK(Type)) { //|| (IS_DOLBY_TRACK(Type) && SetAudioBypass(true))) {
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
        SetAudioEncoding(AUDIO_ENCODING_MPEG1);
        }
     else if (IS_DOLBY_TRACK(Type)) {
        pidHandles[ptDolby].pid = TrackId->id;
        SetPid(&pidHandles[ptDolby], ptDolby, true);
        SetAudioEncoding(AUDIO_ENCODING_AC3);
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
         printf("cDvbHdDevice::%s:%d PlayMode=None\n", __func__, __LINE__);
         // special handling to return from PCM replay:
         CHECK(ioctl(fd_video, VIDEO_SET_BLANK, true));
         CHECK(ioctl(fd_video, VIDEO_SELECT_SOURCE, VIDEO_SOURCE_MEMORY));
         CHECK(ioctl(fd_video, VIDEO_PLAY));
         /**/
         CHECK(ioctl(fd_video, VIDEO_STOP, true));
         CHECK(ioctl(fd_audio, AUDIO_STOP, true));
         CHECK(ioctl(fd_video, VIDEO_SELECT_SOURCE, VIDEO_SOURCE_DEMUX));
         CHECK(ioctl(fd_audio, AUDIO_SELECT_SOURCE, AUDIO_SOURCE_DEMUX));
         CHECK(ioctl(fd_audio, AUDIO_SET_AV_SYNC, true));
         CHECK(ioctl(fd_audio, AUDIO_SET_MUTE, false));
         break;
    case pmAudioVideo:
    case pmAudioOnlyBlack:
         printf("cDvbHdDevice::%s:%d PlayMode=AudioVideo_or_AudioOnlyBlack\n", __func__, __LINE__);
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
         printf("cDvbHdDevice::%s:%d PlayMode=AudioOnly\n", __func__, __LINE__);
         CHECK(ioctl(fd_video, VIDEO_SET_BLANK, true));
         CHECK(ioctl(fd_audio, AUDIO_STOP, true));
         CHECK(ioctl(fd_audio, AUDIO_SELECT_SOURCE, AUDIO_SOURCE_MEMORY));
         CHECK(ioctl(fd_audio, AUDIO_SET_AV_SYNC, false));
         CHECK(ioctl(fd_audio, AUDIO_PLAY));
         CHECK(ioctl(fd_video, VIDEO_SET_BLANK, false));
         break;
    case pmVideoOnly:
         printf("cDvbHdDevice::%s:%d PlayMode=VideoOnly\n", __func__, __LINE__);
         CHECK(ioctl(fd_video, VIDEO_SET_BLANK, true));
         CHECK(ioctl(fd_video, VIDEO_STOP, true));
         CHECK(ioctl(fd_audio, AUDIO_SELECT_SOURCE, AUDIO_SOURCE_DEMUX));
         CHECK(ioctl(fd_audio, AUDIO_SET_AV_SYNC, false));
         CHECK(ioctl(fd_audio, AUDIO_PLAY));
         CHECK(ioctl(fd_video, VIDEO_SELECT_SOURCE, VIDEO_SOURCE_MEMORY));
         CHECK(ioctl(fd_video, VIDEO_PLAY));
         break;
    case pmExtern_THIS_SHOULD_BE_AVOIDED:
         printf("cDvbHdDevice::%s:%d PlayMode=Extern\n", __func__, __LINE__);
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
  if (fd_video >= 0) {
     uint64_t pts;
     if (ioctl(fd_video, VIDEO_GET_PTS, &pts) == -1) {
        esyslog("ERROR: pts %d: %m", CardIndex() + 1);
        return -1;
        }
     return pts;
     }

  if (fd_audio >= 0) {
     uint64_t pts;
     if (ioctl(fd_audio, AUDIO_GET_PTS, &pts) == -1) {
        esyslog("ERROR: pts %d: %m", CardIndex() + 1);
        return -1;
        }
     return pts;
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
  dsyslog("cDvbHdDevice::%s:%d\n", __func__, __LINE__);

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
  //return WriteAllOrNothing(fd_video, Data, Length, 1000, 10);

  /* ST sh4 */
  if (SetVideoEncoding(PatPmtParser()->Vtype())) {
     CHECK(ioctl(fd_video, VIDEO_STOP, true));
     CHECK(ioctl(fd_audio, AUDIO_STOP, true));
     ChangeAVEncoding(1, 0);
     }

  return cDevice::PlayTsVideo(Data, Length);
}

int cDvbHdDevice::PlayTsAudio(const uchar *Data, int Length)
{
  //return WriteAllOrNothing(fd_audio, Data, Length, 1000, 10);

  /* ST sh4 */
  return cDevice::PlayTsAudio(Data, Length);
}

// --- cDvbHdDeviceProbe ---------------------------------------------------

bool cDvbHdDeviceProbe::Probe(int Adapter, int Frontend)
{
  // It seems that we dont have these entries, so fall back
  // TODO: This should be changed to be target independend
  if (Adapter > 0 || Frontend > 0 /* FIXME: On DualTuner boxes we got two Fronteds */)
     return false;

  dsyslog("creating cDvbHdDevice %d/%d", Adapter, Frontend);
  new cDvbHdDevice(Adapter, Frontend);

  return true;
}
