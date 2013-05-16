/*
 * dvbhddevice.h: The DVB HD device interface
 *
 * See the README file for copyright information and how to reach the author.
 *
 * $Id: dvbhddevice.h $
 */

#ifndef __DVBHDDEVICE_H
#define __DVBHDDEVICE_H

#include "vdr/dvbdevice.h"
#include "vdr/dvbspu.h"

/// The cDvbHdDevice implements a DVB device which can be accessed through the Linux DVB driver API.

class cDvbHdDevice : public cDvbDevice {
private:
  int card;
  int fd_osd, fd_audio, fd_video, fd_stc, fd_ca;
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
  //bool SetAudioBypass(bool On);
protected:
  virtual bool SetPid(cPidHandle *Handle, int Type, bool On);

// Image Grab facilities

private:
  int devVideoIndex;
public:
  virtual uchar *GrabImage(int &Size, bool Jpeg = true, int Quality = -1, int SizeX = -1, int SizeY = -1);

// Video format facilities

private:
  int videoStreamType;
  int audioTrackType;
public:
  virtual void SetVideoSystem(int VideoSystem);
  virtual void SetVideoDisplayFormat(eVideoDisplayFormat VideoDisplayFormat);
  virtual void SetVideoFormat(bool VideoFormat16_9);
  virtual eVideoSystem GetVideoSystem(void);
  virtual void GetVideoSize(int &Width, int &Height, double &VideoAspect);
  virtual void GetOsdSize(int &Width, int &Height, double &PixelAspect);
  // ST sh4
  virtual bool SetVideoEncoding(int Vtype);
  virtual bool SetAudioEncoding(int Atype);
  virtual void ChangeAVEncoding(bool Video, bool Audio);

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
  };

class cDvbHdDeviceProbe : public cDvbDeviceProbe {
public:
  virtual bool Probe(int Adapter, int Frontend);
  };
 
#endif //__DVBHDDEVICE_H
