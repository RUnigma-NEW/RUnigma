/*
 * dvbhddevice.h: The DVB HD device interface
 *
 * See the README file for copyright information and how to reach the author.
 *
 * $Id: dvbhddevice.h $
 */

#ifndef __DVBHDDEVICE_H
#define __DVBHDDEVICE_H

#include "dvbcmd.h"
#include "vdr/dvbdevice.h"
#include "vdr/dvbspu.h"

typedef enum _eVideoDisplay {
     vdPanAndScan,
     vdLetterBox,
     vdBestFit,
     vdNonLinear
} eVideoDisplay;

/// The cDvbHdDevice implements a DVB device which can be accessed through the Linux DVB driver API.

class cDvbHdDevice : public cDvbDevice {
private:
  int fd_osd, fd_audio, fd_video, fd_stc;
protected:
  virtual void MakePrimaryDevice(bool On);
public:
  cDvbHdDevice(int Adapter, int Frontend);
  virtual ~cDvbHdDevice();
  virtual bool HasDecoder(void) const;

// SPU facilities

private:
  cDvbSpuDecoder *spuDecoder;
public:
  virtual cSpuDecoder *GetSpuDecoder(void);

// Channel facilities

private:
  void TurnOffLiveMode(bool LiveView);
protected:
  virtual bool SetChannelDevice(const cChannel *Channel, bool LiveView);

// PID handle facilities

private:
  bool SetAudioBypass(bool On);
protected:
  virtual bool SetPid(cPidHandle *Handle, int Type, bool On);

// Image Grab facilities

private:
  static int devVideoOffset;
  int devVideoIndex;
public:
  virtual uchar *GrabImage(int &Size, bool Jpeg = true, int Quality = -1, int SizeX = -1, int SizeY = -1);

// Video format facilities

#ifdef PLATFORM_STB71XX
private:
  int videoStreamType;
  int audioTrackType;
#endif
public:
#ifdef xPLATFORM_STB71XX
  virtual void SetResolution(int Resolution);
#endif
  virtual void SetVideoDisplayFormat(eVideoDisplayFormat VideoDisplayFormat);
  virtual void SetVideoFormat(bool VideoFormat16_9);
  //virtual void SetTvFormat(bool TvFormat16_9);
  //virtual void SetVideoDisplay(eVideoDisplay VideoDisplay);
  virtual eVideoSystem GetVideoSystem(void);
  virtual void GetVideoSize(int &Width, int &Height, double &VideoAspect);
  virtual void GetOsdSize(int &Width, int &Height, double &PixelAspect);
#ifdef PLATFORM_STB71XX
  virtual void SetVideoStreamtype(int VideoStreamtype);

  virtual void ChangeStream(bool Encoding=false, bool AudioOnly=false);
  virtual void StopVideo(bool blank = true);
  virtual void StopAudio(void);
#endif

// Track facilities

protected:
  virtual void SetAudioTrackDevice(eTrackType Type);

// Audio facilities

private:
  bool digitalAudio;
protected:
  virtual int GetAudioChannelDevice(void);
  virtual void SetAudioChannelDevice(int AudioChannel);
  virtual void SetVolumeDevice(int Volume);
  virtual void SetDigitalAudioDevice(bool On);

// Player facilities

protected:
  ePlayMode playMode;
  virtual bool CanReplay(void) const;
  virtual bool SetPlayMode(ePlayMode PlayMode);
  virtual int PlayVideo(const uchar *Data, int Length);
  virtual int PlayAudio(const uchar *Data, int Length, uchar Id);
  virtual int PlayTsVideo(const uchar *Data, int Length);
  virtual int PlayTsAudio(const uchar *Data, int Length);
public:
  virtual int64_t GetSTC(void);
  virtual void TrickSpeed(int Speed);
  virtual void Clear(void);
  virtual void Play(void);
  virtual void Freeze(void);
  virtual void Mute(void);
  virtual void StillPicture(const uchar *Data, int Length);
  virtual bool Poll(cPoller &Poller, int TimeoutMs = 0);
  virtual bool Flush(int TimeoutMs = 0);

// HDFF specific things

public:
  static DVB::cDvbCmdIf *GetDvbCmdHandler(void);
private:
  static int devDvbOffset;//TODO
  bool isDvbPrimary;//TODO implicit!
  DVB::cDvbCmdIf *mDvbCmdIf;
};

class cDvbHdDeviceProbe : public cDvbDeviceProbe {
public:
  virtual bool Probe(int Adapter, int Frontend);
  };

#endif //__DVBHDDEVICE_H
