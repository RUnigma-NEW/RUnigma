/*
 * MP3/MPlayer plugin to VDR (C++)
 *
 * (C) 2001-2009 Stefan Huelswitt <s.huelswitt@gmx.de>
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

#include <getopt.h>
#include <malloc.h>
#include <stdlib.h>
#include <ctype.h>

#include "common.h"

#include <vdr/plugin.h>
#include <vdr/player.h>
#include <vdr/status.h>
#include <vdr/font.h>
#include <vdr/osdbase.h>
#include <vdr/menuitems.h>
#include <vdr/skins.h>
#include <vdr/remote.h>
#include <vdr/menu.h>

#include "setup.h"
#include "setup-mplayer.h"
#include "menu.h"
#include "player-mplayer.h"
#include "data.h"
#include "data-src.h"
#include "i18n.h"
#include "version.h"
#include "service.h"

const char *sourcesSub=0;
cFileSources MPlaySources;

static const char *plugin_name=0;
const char *i18n_name=0;

// --- cMenuSetupMPlayer --------------------------------------------------------

class cMenuSetupMPlayer : public cMenuSetupPage {
private:
  cMPlayerSetup data;
  const char *res[3];
protected:
  virtual void Store(void);
public:
  cMenuSetupMPlayer(void);
  };

cMenuSetupMPlayer::cMenuSetupMPlayer(void)
{
  data=MPlayerSetup;
  SetSection(tr("MPlayer"));
  Add(new cMenuEditBoolItem(tr("Setup.MPlayer$Control mode"),  &data.SlaveMode, tr("Traditional"), tr("Slave")));
  res[0]=tr("disabled");
  res[1]=tr("global only");
  res[2]=tr("local first");
  Add(new cMenuEditStraItem(tr("Setup.MPlayer$Resume mode"),   &data.ResumeMode, 3, res));
  Add(new cMenuEditBoolItem(tr("Hide mainmenu entry"),         &data.HideMainMenu));
  for(int i=0; i<10; i++) {
    char name[32];
    snprintf(name,sizeof(name),"%s %d",tr("Setup.MPlayer$Slave command key"),i);
    static const char allowed[] = { "abcdefghijklmnopqrstuvwxyz0123456789!\"�$%&/()=?{}[]\\+*~#',;.:-_<>|@�`^�" };
    Add(new cMenuEditStrItem(name, data.KeyCmd[i],MAX_KEYCMD,allowed));
    }
}

void cMenuSetupMPlayer::Store(void)
{
  MPlayerSetup=data;
  SetupStore("ControlMode", MPlayerSetup.SlaveMode);
  SetupStore("HideMainMenu",MPlayerSetup.HideMainMenu);
  SetupStore("ResumeMode",  MPlayerSetup.ResumeMode);
  for(int i=0; i<10; i++) {
    char name[16];
    snprintf(name,sizeof(name),"KeyCmd%d",i);
    SetupStore(name,MPlayerSetup.KeyCmd[i]);
    }
}

// --- cMPlayerControl ---------------------------------------------------------

class cMPlayerControl : public cControl {
private:
  cFileObj *current;
  cPlayList *plist;
  static cFileObj *file;
  static bool rewind;
  cMPlayerPlayer *player;
  cSkinDisplayReplay *display;
  bool visible, modeOnly;
  time_t timeoutShow;
  int lastCurrent, lastTotal;
  char *lastReplayMsg;
  //
  bool jumpactive, jumphide, jumpmode;
  int timeSearchTime, timeSearchPos;
  //
  void Stop(void);
  void ShowTimed(int Seconds=0);
  void ShowProgress(void);
  void ShowMode(void);
  void ShowTitle(void);
  void Jump(void);
  void JumpProcess(eKeys Key);
  void JumpDisplay(void);
  void gotoChapter(int g);
public:
  cMPlayerControl(void);
  virtual ~cMPlayerControl();
  virtual eOSState ProcessKey(eKeys Key);
  virtual void Show(void) { ShowTimed(); }
  virtual void Hide(void);
  static void SetFile(const cFileObj *File, bool Rewind);
  };

cFileObj *cMPlayerControl::file=0;
bool cMPlayerControl::rewind=false;

cMPlayerControl::cMPlayerControl(void)
:cControl(player=new cMPlayerPlayer(file,rewind))
{
  visible=modeOnly=jumpactive=false;
  lastReplayMsg=0;
  display=0;
  ShowTitle();
}

cMPlayerControl::~cMPlayerControl()
{
  Stop();
  cStatus::MsgReplaying(this,0,0,false);
  free(lastReplayMsg);
}

void cMPlayerControl::SetFile(const cFileObj *File, bool Rewind)
{
  delete file;
  file=File ? new cFileObj(File) : 0;
  rewind=Rewind;
}

void cMPlayerControl::Stop(void)
{
  delete player; player=0;
}

void cMPlayerControl::ShowTimed(int Seconds)
{
  if(modeOnly) Hide();
  if(!visible) {
    ShowProgress();
    timeoutShow = Seconds>0 ? time(0)+Seconds : 0;
    }
}

void cMPlayerControl::Hide(void)
{
  if(visible) {
    delete display; display=0;
    visible=modeOnly=false;
#if APIVERSNUM >= 10500
    SetNeedsFastResponse(false);
#else
    needsFastResponse=false;
#endif
    }
}

void cMPlayerControl::ShowTitle(void)
{
  const char *path=0;
  bool release=true;
  if(player) path=player->GetCurrentName();
  if(!path) {
    path=file->FullPath();
    release=false;
    }
  if(path) {
    const char *name=rindex(path,'/');
    if(name) name++; else name=path;
    if(!lastReplayMsg || strcmp(lastReplayMsg,path)) {
      cStatus::MsgReplaying(this,name,path,true);
      free(lastReplayMsg);
      lastReplayMsg=strdup(path);
      }
    if(visible) {
      if(display) display->SetTitle(name);
      }
    }
  if(release) free((void *)path);
}

void cMPlayerControl::ShowProgress(void)
{
  int Current, Total;

  if(GetIndex(Current,Total) && Total>0) {
    bool flush=false;
    if(!visible) {
      display=Skins.Current()->DisplayReplay(false);
      visible=true; modeOnly=false;
#if APIVERSNUM >= 10500
      SetNeedsFastResponse(true);
#else
      needsFastResponse=true;
#endif
      lastCurrent=lastTotal=-1;
      flush=true;
      }

    if(abs(Current-lastCurrent)>12) {
      if(Total>0) display->SetProgress(Current, Total);
      display->SetCurrent(IndexToHMSF(Current));
      display->SetTotal(IndexToHMSF(Total));
      bool Play, Forward;
      int Speed;
      if(GetReplayMode(Play,Forward,Speed)) 
        display->SetMode(Play, Forward, Speed);
      ShowTitle();
      flush=true;
      lastCurrent=Current; lastTotal=Total;
      }
    if(flush) 
      Skins.Flush();
    ShowMode();
    }
}

void cMPlayerControl::ShowMode(void)
{
  if(Setup.ShowReplayMode && !jumpactive) {
    bool Play, Forward;
    int Speed;
    if(GetReplayMode(Play, Forward, Speed)) {
       bool NormalPlay = (Play && Speed == -1);

       if(!visible) {
         if(NormalPlay) return;
         display = Skins.Current()->DisplayReplay(true);
         visible=modeOnly=true;
         }

       if(modeOnly && !timeoutShow && NormalPlay) timeoutShow=time(0)+SELECTHIDE_TIMEOUT;

       display->SetMode(Play, Forward, Speed);
       }
    }
}

void cMPlayerControl::JumpDisplay(void)
{
  char buf[64];
  strcpy(buf, tr("Jump: "));
  int len = strlen(buf);
  char h10 = '0' + (timeSearchTime >> 24);
  char h1  = '0' + ((timeSearchTime & 0x00FF0000) >> 16);
  char m10 = '0' + ((timeSearchTime & 0x0000FF00) >> 8);
  char m1  = '0' + (timeSearchTime & 0x000000FF);
  char ch10 = timeSearchPos > 3 ? h10 : '-';
  char ch1  = timeSearchPos > 2 ? h1  : '-';
  char cm10 = timeSearchPos > 1 ? m10 : '-';
  char cm1  = timeSearchPos > 0 ? m1  : '-';
  sprintf(buf + len, "%c%c:%c%c", ch10, ch1, cm10, cm1);
  display->SetJump(buf); 
}

void cMPlayerControl::JumpProcess(eKeys Key)
{
  const int n=Key-k0;
  int Seconds = (timeSearchTime >> 24) * 36000 + ((timeSearchTime & 0x00FF0000) >> 16) * 3600 + ((timeSearchTime & 0x0000FF00) >> 8) * 600 + (timeSearchTime & 0x000000FF) * 60;
  int Current = int(round(lastCurrent / FramesPerSecond()));
  int Total = int(round(lastTotal / FramesPerSecond()));
  switch (Key) {
    case k0 ... k9:
         if (timeSearchPos < 4) {
            timeSearchTime <<= 8;
            timeSearchTime |= Key - k0;
            timeSearchPos++;
            JumpDisplay();
            }
         break;
    case kBlue:
      jumpmode=!jumpmode;
      JumpDisplay();
      break;
    case kPlay:
    case kOk:
      player->Goto(Seconds,false);
      jumpactive=false;
      break;
    case kFastRew:
    case kFastFwd:
    case kLeft:
    case kRight:
      break;
    default:
      jumpactive=false;
      break;
    }

  if(!jumpactive) {
    if(jumphide) Hide();
    else 
      display->SetJump(0);
    }
}

void cMPlayerControl::Jump(void)
{
  jumphide=jumpmode=false;
  timeSearchTime = timeSearchPos = 0;
  if(!visible) {
    ShowTimed(); if(!visible) return;
    jumphide=true;
    }
  JumpDisplay();
  jumpactive=true;
}

void cMPlayerControl::gotoChapter(int g){
    fprintf(stderr, "gotoCHAPTER");
	plist=new cPlayList;
    
	cFileObj *item=plist->First();
    while(item) {
      fprintf(stderr, "plist %s", item->Name());
      item=plist->Next(item);
      }


/* 	cFileObj *CurrentList =  cMenuBrowse::GetSelected();
	fprintf(stderr, "currentlist %s", CurrentList->Name());
	 */

/* 	fprintf(stderr,"klick-right\n");
	cFileObj *CurrentList = CurrentPlayLists.GetSelected();
	fprintf(stderr, "currentlist %s", CurrentList->Name()); */
/* 	
	cFileObj *xitem=CurrentList->First();
	fprintf(stderr, "xitem->Name() %s \n", xitem->Name());
	
	while(xitem) {
		fprintf(stderr, "xitem->Name() %s \n", xitem->Name());
		xitem=CurrentList->Next(xitem); 
	} 
*/
}
	

eOSState cMPlayerControl::ProcessKey(eKeys Key)
{
  if(!player->Active()) { Hide(); Stop(); return osEnd; }

  if(!player->SlaveMode()) {
    if(Key==kBlue) { Hide(); Stop(); return osEnd; }
    }
  else {
    if(visible) {
      if(timeoutShow && time(0)>timeoutShow) {
        Hide(); ShowMode();
        timeoutShow = 0;
        }
      else {
        if(modeOnly) ShowMode();
        else ShowProgress();
        }
      }
    else ShowTitle();

    if(jumpactive && Key != kNone) {
      JumpProcess(Key);
      return osContinue;
      }

    bool DoShowMode = true;
    switch (Key) {
      case kPlay:
      case kUp:      player->Play(); break;

      case kPause:
      case kDown:    player->Pause(); break;

      case kFastRew|k_Repeat:
      case kFastRew:
      case kLeft|k_Repeat:
      case kLeft:    player->SkipSeconds(-10); break;

      case kFastFwd|k_Repeat:
      case kFastFwd:
      case kRight|k_Repeat:
      case kRight:  {
		gotoChapter(1);

	
	
	
	  //player->SkipSeconds(10);  
	  break;
	  
	  }

      case kRed:     Jump(); break;

      case kGreen|k_Repeat:                      // temporary use
      case kGreen:   player->SkipSeconds(-60); break;
      case kYellow|k_Repeat:
      case kYellow:  player->SkipSeconds(60); break;
  //    case kGreen|k_Repeat:                      // reserved for future use
  //    case kGreen:   player->SkipPrev(); break;
  //    case kYellow|k_Repeat:
  //    case kYellow:  player->SkipNext(); break;

      case kBack:
                     Hide();
                     cRemote::CallPlugin(plugin_name);
                     return osBack;
      case kStop:
      case kBlue:    Hide(); Stop(); return osEnd;

      default:
        DoShowMode = false;
        switch(Key) {
          case kOk: if(visible && !modeOnly) { Hide(); DoShowMode=true; }
                    else ShowTimed();
                    break;
          case kAudio:
                    player->KeyCmd("switch_audio");
                    break;
          case kNext:
                    player->KeyCmd("seek_chapter +1");
                    break;
          case kPrev:
                    player->KeyCmd("seek_chapter -1");
                    break;
          case k0:
          case k1:
          case k2:
          case k3:
          case k4:
          case k5:
          case k6:
          case k7:
          case k8:
          case k9:  {
                    const char *cmd=MPlayerSetup.KeyCmd[Key-k0];
                    if(cmd[0]) player->KeyCmd(cmd);
                    }
                    break;
          default:  break;
          }
        break;
      }

    if(DoShowMode) ShowMode();
    }
  return osContinue;
}

// --- cMenuMPlayAid -----------------------------------------------------------

class cMenuMPlayAid : public cOsdMenu {
public:
  cMenuMPlayAid(void);
  virtual eOSState ProcessKey(eKeys Key);
  };

cMenuMPlayAid::cMenuMPlayAid(void)
:cOsdMenu(tr("MPlayer Audio ID"),20)
{
  Add(new cMenuEditIntItem(tr("Audiostream ID"),&MPlayerAid,-1,255));
  Display();
}

eOSState cMenuMPlayAid::ProcessKey(eKeys Key)
{
  eOSState state=cOsdMenu::ProcessKey(Key);
  if(state==osUnknown) {
    switch(Key) {
      case kOk: state=osBack; break;
      default:  break;
      }
    }
  return state;
}

// --- cMenuMPlayBrowse ---------------------------------------------------------

class cMenuMPlayBrowse : public cMenuBrowse {
private:
  bool sourcing, aidedit;
  eOSState Source(bool second);
  eOSState Summary(void);
protected:
  virtual void SetButtons(void);
public:
  cMenuMPlayBrowse(void);
  virtual eOSState ProcessKey(eKeys Key);
  };

static const char *excl_sum[] = { ".*","*.summary","*.txt","*.nfo",0 };

cMenuMPlayBrowse::cMenuMPlayBrowse(void)
:cMenuBrowse(MPlaySources.GetSource(),false,false,tr("MPlayer browser"),excl_sum)
{
  sourcing=aidedit=false;
  SetButtons();
}

void cMenuMPlayBrowse::SetButtons(void)
{
  static char blue[12];
  snprintf(blue,sizeof(blue),MPlayerAid>=0 ? "AID:%d" : "AID:def",MPlayerAid);
  SetHelp(trVDR("Button$Play"), MPlayerSetup.ResumeMode ? trVDR("Button$Rewind"):0, tr("Source"), blue);
  Display();
}

eOSState cMenuMPlayBrowse::Source(bool second)
{
  if(HasSubMenu()) return osContinue;

  if(!second) {
    sourcing=true;
    return AddSubMenu(new cMenuSource(&MPlaySources,tr("MPlayer source")));
    }
  sourcing=false;
  cFileSource *src=cMenuSource::GetSelected();
  if(src) {
    MPlaySources.SetSource(src);
    SetSource(src);
    NewDir(0);
    }
  return osContinue;
}

eOSState cMenuMPlayBrowse::Summary(void)
{
  cFileObj *item=CurrentItem();
  if(item && item->Type()==otFile) {
    static const char *exts[] = { ".summary",".txt",".nfo",0 };
    for(int i=0; exts[i]; i++) {
      char buff[4096];
      strn0cpy(buff,item->FullPath(),sizeof(buff)-20);
      char *e=&buff[strlen(buff)];
      strn0cpy(e,exts[i],20);
      int fd=open(buff,O_RDONLY);
      *e=0;
      if(fd<0 && (e=rindex(buff,'.'))) {
        strn0cpy(e,exts[i],20);
        fd=open(buff,O_RDONLY);
        }
      if(fd>=0) {
        int r=read(fd,buff,sizeof(buff)-1);
        close(fd);
        if(r>0) {
          buff[r]=0;
          return AddSubMenu(new cMenuText(tr("Summary"),buff));
          }
        }
      }
    }
  return osContinue;
}

eOSState cMenuMPlayBrowse::ProcessKey(eKeys Key)
{
  eOSState state=cOsdMenu::ProcessKey(Key);
  if(state==osContinue && !HasSubMenu()) {
    if(sourcing) return Source(true);
    if(aidedit) { aidedit=false; SetButtons(); }
    }
  bool rew=false;
  if(state==osUnknown) {
    switch(Key) {
      case kGreen:
        {
        cFileObj *item=CurrentItem();
        if(item && item->Type()==otFile) {
          lastselect=new cFileObj(item);
          state=osBack;
          rew=true;
          } 
        else state=osContinue;
        break;
        }
      case kYellow:
        state=Source(false);
        break;
      case kBlue:
        aidedit=true;
        state=AddSubMenu(new cMenuMPlayAid);
        break;
      case k0:
        state=Summary();
        break;
      default:
        break;
      }
    }
  if(state==osUnknown) state=cMenuBrowse::ProcessStdKey(Key,state);
  if(state==osBack && lastselect) {
    cMPlayerControl::SetFile(lastselect,rew);
    cControl::Launch(new cMPlayerControl);
    return osEnd;
    }
  return state;
}

// --- cPluginMPlayer ----------------------------------------------------------

static const char *DESCRIPTION    = trNOOP("Media replay via MPlayer");
static const char *MAINMENUENTRY  = "MPlayer";

class cPluginMPlayer : public cPlugin {
private:
  bool ExternalPlay(const char *path, bool test);
public:
  cPluginMPlayer(void);
  virtual ~cPluginMPlayer();
  virtual const char *Version(void) { return PluginVersion; }
  virtual const char *Description(void) { return tr(DESCRIPTION); }
  virtual const char *CommandLineHelp(void);
  virtual bool ProcessArgs(int argc, char *argv[]);
  virtual bool Initialize(void);
  virtual const char *MainMenuEntry(void);
  virtual cOsdMenu *MainMenuAction(void);
  virtual cMenuSetupPage *SetupMenu(void);
  virtual bool SetupParse(const char *Name, const char *Value);
  virtual bool Service(const char *Id, void *Data);
  virtual const char **SVDRPHelpPages(void);
  virtual cString SVDRPCommand(const char *Command, const char *Option, int &ReplyCode);
  };

cPluginMPlayer::cPluginMPlayer(void)
{
  // Initialize any member variables here.
  // DON'T DO ANYTHING ELSE THAT MAY HAVE SIDE EFFECTS, REQUIRE GLOBAL
  // VDR OBJECTS TO EXIST OR PRODUCE ANY OUTPUT!
  status=0;
}

cPluginMPlayer::~cPluginMPlayer()
{
  delete status;
}

const char *cPluginMPlayer::CommandLineHelp(void)
{
  static char *help_str=0;
  
  free(help_str);    //                                     for easier orientation, this is column 80|
  help_str=aprintf(  "  -m CMD,   --mount=CMD    use CMD to mount/unmount/eject mp3 sources\n"
                     "                           (default: %s)\n"
                     "  -M CMD,   --mplayer=CMD  use CMD when calling MPlayer\n"
                     "                           (default: %s)\n"
                     "  -S SUB,   --sources=SUB  search sources config in SUB subdirectory\n"
                     "                           (default: %s)\n"
                     "  -R DIR,   --resume=DIR   store global resume file in DIR\n"
                     "                           (default: %s)\n",
                     mountscript,
                     MPlayerCmd,
                     sourcesSub ? sourcesSub:"none",
                     globalResumeDir ? globalResumeDir:"video dir"
                     );
  return help_str;
}

bool cPluginMPlayer::ProcessArgs(int argc, char *argv[])
{
  static struct option long_options[] = {
      { "mount",    required_argument, NULL, 'm' },
      { "mplayer",  required_argument, NULL, 'M' },
      { "sources",  required_argument, NULL, 'S' },
      { "resume",   required_argument, NULL, 'R' },
      { NULL }
    };

  int c, option_index = 0;
  while((c=getopt_long(argc,argv,"m:M:S:R:",long_options,&option_index))!=-1) {
    switch (c) {
      case 'm': mountscript=optarg; break;
      case 'M': MPlayerCmd=optarg; break;
      case 'S': sourcesSub=optarg; break;
      case 'R': globalResumeDir=optarg; break;
      default:  return false;
      }
    }
  return true;
}

bool cPluginMPlayer::Initialize(void)
{
  if(!CheckVDRVersion(1,4,5,"mplayer")) return false;
  plugin_name="mplayer";
  i18n_name="vdr-mplayer";
  MPlaySources.Load(AddDirectory("/usr/local/share/vdr/plugins","mplayersources.conf"));
  if(MPlaySources.Count()<1) {
    esyslog("ERROR: you must have defined at least one source in mplayersources.conf");
    fprintf(stderr,"No source(s) defined in mplayersources.conf\n");
    return false;
    }
  if(!(status=new cMPlayerStatus)) return false;
  return true;
}

const char *cPluginMPlayer::MainMenuEntry(void)
{
  return MPlayerSetup.HideMainMenu ? 0 : tr(MAINMENUENTRY);
}

cOsdMenu *cPluginMPlayer::MainMenuAction(void)
{
  return new cMenuMPlayBrowse;
}

cMenuSetupPage *cPluginMPlayer::SetupMenu(void)
{
  return new cMenuSetupMPlayer;
}

bool cPluginMPlayer::SetupParse(const char *Name, const char *Value)
{
  if(      !strcasecmp(Name, "ControlMode"))  MPlayerSetup.SlaveMode    = atoi(Value);
  else if (!strcasecmp(Name, "HideMainMenu")) MPlayerSetup.HideMainMenu = atoi(Value);
  else if (!strcasecmp(Name, "ResumeMode"))   MPlayerSetup.ResumeMode   = atoi(Value);
  else if (!strncasecmp(Name,"KeyCmd", 6) && strlen(Name)==7 && isdigit(Name[6]))
    strn0cpy(MPlayerSetup.KeyCmd[Name[6]-'0'],Value,sizeof(MPlayerSetup.KeyCmd[0]));
  else return false;
  return true;
}

bool cPluginMPlayer::ExternalPlay(const char *path, bool test)
{
  char real[PATH_MAX+1];
  if(realpath(path,real)) {
    cFileSource *src=MPlaySources.FindSource(real);
    if(src) {
      cFileObj *item=new cFileObj(src,0,0,otFile);
      if(item) {
        item->SplitAndSet(real);
        if(item->GuessType()) {
          if(item->Exists()) {
            if(!test) {
              cMPlayerControl::SetFile(item,true);
              cControl::Launch(new cMPlayerControl);
              cControl::Attach();
              }
            delete item;
            return true;
            }
          else dsyslog("MPlayer service: cannot play '%s'",path);
          }
        else dsyslog("MPlayer service: GuessType() failed for '%s'",path);
        delete item;
        }
      }
    else dsyslog("MPlayer service: cannot find source for '%s', real '%s'",path,real);
    }
  else if(errno!=ENOENT && errno!=ENOTDIR)
    esyslog("ERROR: realpath: %s: %s",path,strerror(errno));
  return false;
}

bool cPluginMPlayer::Service(const char *Id, void *Data)
{
  if(!strcasecmp(Id,"MPlayer-Play-v1")) {
    if(Data) {
      struct MPlayerServiceData *msd=(struct MPlayerServiceData *)Data;
      msd->result=ExternalPlay(msd->data.filename,false);
      }
    return true;
    }
  else if(!strcasecmp(Id,"MPlayer-Test-v1")) {
    if(Data) {
      struct MPlayerServiceData *msd=(struct MPlayerServiceData *)Data;
      msd->result=ExternalPlay(msd->data.filename,true);
      }
    return true;
    }
  return false;
}

const char **cPluginMPlayer::SVDRPHelpPages(void)
{
  static const char *HelpPages[] = {
    "PLAY <filename>\n"
    "    Triggers playback of file 'filename'.",
    "TEST <filename>\n"
    "    Tests is playback of file 'filename' is possible.",
    NULL
    };
  return HelpPages;
}

cString cPluginMPlayer::SVDRPCommand(const char *Command, const char *Option, int &ReplyCode)
{
  if(!strcasecmp(Command,"PLAY")) {
    if(*Option) {
      if(ExternalPlay(Option,false)) return "Playback triggered";
      else { ReplyCode=550; return "Playback failed"; }
      }
    else { ReplyCode=501; return "Missing filename"; }
    }
  else if(!strcasecmp(Command,"TEST")) {
    if(*Option) {
      if(ExternalPlay(Option,true)) return "Playback possible";
      else { ReplyCode=550; return "Playback not possible"; }
      }
    else { ReplyCode=501; return "Missing filename"; }
    }
  return NULL;
}

VDRPLUGINCREATOR(cPluginMPlayer); // Don't touch this!
