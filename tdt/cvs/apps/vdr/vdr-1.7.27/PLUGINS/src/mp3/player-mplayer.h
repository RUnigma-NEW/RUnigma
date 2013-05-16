/*
 * MP3/MPlayer plugin to VDR (C++)
 *
 * (C) 2001-2006 Stefan Huelswitt <s.huelswitt@gmx.de>
 *
 * This code is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This code is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 * Or, point your browser to http://www.gnu.org/copyleft/gpl.html
 */

#ifndef ___DVB_MPLAYER_H
#define ___DVB_MPLAYER_H

#include <vdr/player.h>
#include <vdr/thread.h>
#include <vdr/status.h>

// ----------------------------------------------------------------

class cFileObj;
class cMPlayerResume;

// ----------------------------------------------------------------

class cMPlayerStatus : public cStatus, cMutex {
private:
  int volume;
  bool mute, changed;
protected:
  virtual void SetVolume(int Volume, bool Absolute);
public:
  cMPlayerStatus(void);
  bool GetVolume(int &Volume, bool &Mute);
  };

extern cMPlayerStatus *status;

// ----------------------------------------------------------------

class cMPlayerPlayer : public cPlayer, cThread {
private:
  cFileObj *file;
  int lasKnownItme, indexPos, spoolingPts;
  bool rewind;
  cMPlayerResume *resume;
  bool started, slave, run;
  int pid, inpipe[2], outpipe[2], pipefl;
  bool brokenPipe;
  enum ePlayMode { pmPlay, pmPaused };
  ePlayMode playMode;
  int index, total, saveIndex;
  int nextTime, nextPos;
  int mpVolume;
  bool mpMute;
  char *currentName;
  //
  bool Fork(void);
  void ClosePipe(void);
  void MPlayerControl(const char *format, ...);
  void SetMPlayerVolume(bool force);
protected:
  virtual void Activate(bool On);
  virtual void Action(void);
public:
  cMPlayerPlayer(const cFileObj *File, bool Rewind);
  virtual ~cMPlayerPlayer();
  bool Active(void);
  bool SlaveMode(void) const { return slave; }
  void Pause(void);
  void Play(void);
  void Goto(int Index,  bool still);
  void SkipSeconds(int secs);
  void KeyCmd(const char *cmd);
  char *GetCurrentName(void);
  virtual bool GetIndex(int &Current, int &Total, bool SnapToIFrame);
  virtual bool GetReplayMode(bool &Play, bool &Forward, int &Speed);
  };

extern int MPlayerAid;
extern const char *globalResumeDir;

#endif //___DVB_MPLAYER_H
