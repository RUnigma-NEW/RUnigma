/*
 * See the README file for copyright information and how to reach the author.
 */

#include <sys/statvfs.h>
#include <string>
#include <vdr/videodir.h>
#include <vdr/remote.h>
#include <vdr/menu.h>
#include <vdr/interface.h>
#include "mymenurecordings.h"
#include "tools.h"

using namespace std;

// --- myMenuRenameRecording --------------------------------------------------
myMenuRenameRecording::myMenuRenameRecording(cRecording *Recording,const char *DirBase,const char *DirName):cOsdMenu(tr("Rename"),12)
{
 isdir=false;
 recording=Recording;
 dirbase=DirBase?ExchangeChars(strdup(DirBase),true):NULL;
 dirname=DirName?ExchangeChars(strdup(DirName),true):NULL;
 strn0cpy(name,"",sizeof(name));
 strn0cpy(path,"",sizeof(path));

 if(recording)
 {
  char *p=(char*)strrchr(recording->Name(),'~'); //TODO
  if(p)
  {
   strn0cpy(name,++p,sizeof(name));
   strn0cpy(path,recording->Name(),sizeof(path));

   p=strrchr(path,'~');
   if(p)
    *p=0;
  }
  else
   strn0cpy(name,recording->Name(),sizeof(name));
 }
 else
 {
  isdir=true;
  strn0cpy(name,DirName,sizeof(name));
  if(DirBase)
   strn0cpy(path,DirBase,sizeof(path));
 }
 Add(new cMenuEditStrItem(trVDR("Name"),name,sizeof(name),trVDR(FileNameChars)));
 cRemote::Put(kRight);
}

myMenuRenameRecording::~myMenuRenameRecording()
{
 free(dirbase);
 free(dirname);
}

eOSState myMenuRenameRecording::ProcessKey(eKeys Key)
{
  eOSState state=cOsdMenu::ProcessKey(Key);
  if(state==osContinue)
  {
    if(Key==kOk)
    {
      char *oldname=NULL;
      char *newname=NULL;
      char *tmppath=path[0]?ExchangeChars(strdup(path),true):NULL;
      char *tmpname=name[0]?ExchangeChars(strdup(name),true):NULL;

      if(strchr(name,'.')==name||!strlen(name))
      {
        Skins.Message(mtError,tr("Invalid filename!"));
        cRemote::Put(kRight);
        return osContinue;
      }

      if(isdir)
      {
        if(-1==asprintf(&oldname,"%s%s%s/%s",VideoDirectory,tmppath?"/":"",dirbase?dirbase:"",dirname))
          oldname=NULL;
      }
      else
        oldname=strdup(recording->FileName());

      if(oldname)
      {
        if(-1==asprintf(&newname,"%s%s%s/%s%s",VideoDirectory,tmppath?"/":"",tmppath?tmppath:"",tmpname,isdir?"":strrchr(recording->FileName(),'/')))
          newname=NULL;

        if(newname)
        {
          if(!MakeDirs(newname,true))
            Skins.Message(mtError,tr("Creating directories failed!"));
          else
          {
            if(MoveRename(oldname,newname,isdir?NULL:recording,false))
              state=osBack;
            else
            {
              cRemote::Put(kRight);
              state=osContinue;
            }
          }
          free(newname);
        }
        free(oldname);
      }
      free(tmppath);
      free(tmpname);
    }
    if(Key==kBack)
      return osBack;
  }
  return state;
}

// --- myMenuNewName ----------------------------------------------------------
bool myMenuMoveRecording::clearall=false;
char newname[128]; //TODO

class myMenuNewName:public cOsdMenu
{
 private:
  char name[128];
 public:
  myMenuNewName();
  virtual eOSState ProcessKey(eKeys Key);
};

myMenuNewName::myMenuNewName():cOsdMenu(tr("New folder"),12)
{
 strn0cpy(name,tr("New folder"),sizeof(name));
 Add(new cMenuEditStrItem(trVDR("Name"),name,sizeof(name),trVDR(FileNameChars)));
 cRemote::Put(kRight);
}

eOSState myMenuNewName::ProcessKey(eKeys Key)
{
 eOSState state=cOsdMenu::ProcessKey(Key);

 if(state==osContinue)
 {
  if(Key==kOk)
  {
   if(strchr(name,'.')==name||!strlen(name))
   {
    Skins.Message(mtError,tr("Invalid filename!"));
    cRemote::Put(kRight);
    state=osContinue;
   }
   else
   {
    strn0cpy(newname,name,sizeof(newname));
    state=osBack;
   }
  }
  if(Key==kBack)
   state=osBack;
 }

 return state;
}

// --- myMenuMoveRecordingItem ------------------------------------------------
class myMenuMoveRecordingItem:public cOsdItem
{
 private:
  int level;
  char *title;
 public:
  myMenuMoveRecordingItem(const char *Title,int Level);
  myMenuMoveRecordingItem(cRecording *Recording,int Level);
  int Level(){return level;}
  void SetLevel(int _Level){level=_Level;}
};

myMenuMoveRecordingItem::myMenuMoveRecordingItem(const char *Title,int _Level)
{
 level=_Level;
 title=strdup(Title);
 SetText(title);
}

myMenuMoveRecordingItem::myMenuMoveRecordingItem(cRecording *Recording,int _Level)
{
 level=0;

 const char *s=Recording->Name();
 while(*++s)
 {
  if(*s=='~')
   level++;
 }
 if(_Level<level)
 {
  s=Recording->Name();
  const char *p=s;
  while(*++s)
  {
   if(*s=='~')
   {
    if(_Level--)
     p=s+1;
    else
     break;
   }
  }
  title=MALLOC(char,s-p+1);
  strn0cpy(title,p,s-p+1);
  SetText(title);
 }
 else
  SetText("");
}

// --- myMenuMoveRecording ----------------------------------------------------
myMenuMoveRecording::myMenuMoveRecording(cRecording *Recording,const char *DirBase,const char *DirName,const char *Base,int Level):cOsdMenu(Base?Base:"")
{
 dirbase=DirBase?strdup(DirBase):NULL;
 dirname=DirName?strdup(DirName):NULL;
 strn0cpy(newname,"",sizeof(newname));
 recording=Recording;
 base=Base?strdup(Base):NULL;

 level=Level;
 Set();
 SetHelp(tr("Button$Cancel"),NULL,tr("Button$Create"),tr("Button$Move"));
}

myMenuMoveRecording::~myMenuMoveRecording()
{
 free(base);
 free(dirbase);
 free(dirname);
}

void myMenuMoveRecording::Set()
{
  if(level==0)
    Add(new myMenuMoveRecordingItem(tr("[base dir]"),0));

  cThreadLock RecordingsLock(&Recordings);
  Recordings.Sort();

  char *lastitemtext=NULL;
  myMenuMoveRecordingItem *lastitem=NULL;
  for(cRecording *_recording=Recordings.First();_recording;_recording=Recordings.Next(_recording))
  {
    if(!base||(strstr(_recording->Name(),base)==_recording->Name()&&_recording->Name()[strlen(base)]=='~'))
    {
      myMenuMoveRecordingItem *item=new myMenuMoveRecordingItem(_recording,level);
      if(*item->Text())
      {
        if(lastitemtext&&!strcmp(lastitemtext,item->Text())) // same text
        {
          if(lastitem&&lastitem->Level()<item->Level()) // if level of the previous item is lower, set it to the new value
            lastitem->SetLevel(item->Level());

          delete item;
        }
        else
        {
          Add(item); // different text means a new item to add
          lastitem=item;
          free(lastitemtext);
          lastitemtext=strdup(lastitem->Text());
        }
      }
      else
        delete item;
    }
  }
  free(lastitemtext);
}

eOSState myMenuMoveRecording::Open()
{
 myMenuMoveRecordingItem *item=(myMenuMoveRecordingItem*)Get(Current());
 if(item)
 {
  if(item->Level()>level)
  {
   const char *t=item->Text();
   char buffer[MaxFileName];
   if(base)
   {
    snprintf(buffer,sizeof(buffer),"%s~%s",base,t);
    t=buffer;
   }
   return AddSubMenu(new myMenuMoveRecording(recording,dirbase,dirname,t,level+1));
  }
 }
 return osContinue;
}

eOSState myMenuMoveRecording::MoveRec()
{
  char *oldname=NULL;
  char *_newname=NULL;
  char *dir=NULL;
  char *tmpdirbase=dirbase?ExchangeChars(strdup(dirbase),true):NULL;
  char *tmpdirname=dirname?ExchangeChars(strdup(dirname),true):NULL;

  eOSState state=osContinue;

  if(dirname)
  {
    if(-1==asprintf(&oldname,"%s%s%s/%s",VideoDirectory,dirbase?"/":"",tmpdirbase?tmpdirbase:"",tmpdirname))
      oldname=NULL;
  }
  else
    oldname=strdup(recording->FileName());

  if(oldname)
  {
    myMenuMoveRecordingItem *item=(myMenuMoveRecordingItem*)Get(Current());
    if(item)
    {
      if(strcmp(tr("[base dir]"),item->Text()))
      {
        if(dirname)
        {
          if(-1==asprintf(&dir,"%s%s%s",base?base:"",base?"~":"",item->Text()))
            dir=NULL;
        }
        else  // needed for move recording menu
        {
          const char *p=strrchr(recording->Name(),'~');
          if(-1==asprintf(&dir,"%s%s%s~%s",base?base:"",base?"~":"",item->Text(),p?p+1:recording->Name()))
            dir=NULL;
        }
      }
      else
      {
        if(!dirname)
        {
          const char *p=strrchr(recording->Name(),'~');
          if(-1==asprintf(&dir,"%s",p?++p:recording->Name()))
            dir=NULL;
        }
      }
    }
    else
    {
      if(dirname)
      {
        if(-1==asprintf(&dir,"%s",base))
          dir=NULL;
      }
      else
      {
        const char *p=strrchr(recording->Name(),'~');
        if(-1==asprintf(&dir,"%s~%s",base,p?p:recording->Name()))
          dir=NULL;
      }
    }
    if(dir)
      dir=ExchangeChars(dir,true);

    if(-1==asprintf(&_newname,"%s%s%s%s",VideoDirectory,dir?"/":"",dir?dir:"",strrchr(dirname?oldname:recording->FileName(),'/')))
      _newname=NULL;

    if(_newname)
    {
      // getting existing part of target path
      string path=_newname;
      string::size_type pos=string::npos;
      do
        pos=path.rfind('/',pos)-1;
      while(access(path.substr(0,pos+1).c_str(),R_OK));

      path=path.substr(0,pos+1);

      struct stat stat1,stat2;
      stat(oldname,&stat1);
      stat(path.c_str(),&stat2);
      // are source and target at the same filesystem?
      if(stat1.st_dev==stat2.st_dev)
      {
        if(MoveRename(oldname,_newname,dirname?NULL:recording,true))
        {
          clearall=true;
          state=osBack;
        }
      }
      else
      {
        struct statvfs fsstat;
        if(!statvfs(path.c_str(),&fsstat))
        {
          int freemb=int((double)fsstat.f_bavail/(1024.0*1024.0/fsstat.f_bsize));
          int recmb=0;

          // moving a single recording
          if(recording)
          {
            recmb=DirSizeMB(recording->FileName());
            if(freemb-recmb > 0  || Interface->Confirm(tr("Target filesystem filled - try anyway?")))
            {
              MoveCutterThread->AddToMoveList(oldname,_newname);
              clearall=true;
              state=osBack;
            }
          }
          // moving a directory
          else
          {
            string buf=oldname;
            buf+="/";
            if(!buf.compare(0,buf.length(),_newname))
              Skins.Message(mtError,tr("Moving into own sub-directory not allowed!"));
            else
            {
              cThreadLock RecordingsLock(&Recordings);
              for(cRecording *rec=Recordings.First();rec;rec=Recordings.Next(rec))
              {
                if(!strncmp(oldname,rec->FileName(),strlen(oldname)))
                  recmb+=DirSizeMB(rec->FileName());
              }

              if(freemb-recmb > 0  || Interface->Confirm(tr("Target filesystem filled - try anyway?")))
              {
                for(cRecording *rec=Recordings.First();rec;rec=Recordings.Next(rec))
                {
                  if(!strncmp(oldname,rec->FileName(),strlen(oldname)))
                  {
                    char *_buf=ExchangeChars(strdup(oldname+strlen(VideoDirectory)+1),false);

                    if(strcmp(rec->Name(),_buf))
                    {
                      free(_buf);
                      if(-1==asprintf(&_buf,"%s%s",_newname,rec->FileName()+strlen(oldname)))
                        _buf=NULL;

                      if(_buf)
                        MoveCutterThread->AddToMoveList(rec->FileName(),_buf);
                    }
                    free(_buf);
                    _buf=NULL;
                  }
                }
                clearall=true;
                state=osBack;
              }
            }
          }
        }
        else
        {
          Skins.Message(mtError,tr("Can't get filesystem information"));
          esyslog("[extrecmenu] %s",strerror(errno));
        }
      }
      free(_newname);
    }
    free(oldname);
  }
  free(dir);
  free(tmpdirbase);
  free(tmpdirname);

  return state;
}

eOSState myMenuMoveRecording::Create()
{
  return AddSubMenu(new myMenuNewName);
}

eOSState myMenuMoveRecording::ProcessKey(eKeys Key)
{
  eOSState state=cOsdMenu::ProcessKey(Key);

  if(state==osUnknown)
  {
    switch(Key)
    {
      case kRed: clearall=true;break;
      case kYellow: return Create();
      case kBlue: return MoveRec();
      case kOk: return Open();
      default: break;
    }
  }

  if(newname[0]!=0)
  {
    Add(new myMenuMoveRecordingItem(newname,level+2));
    Display();
    strn0cpy(newname,"",sizeof(newname));
  }

  if(clearall)
    return osBack;

  return state;
}

// --- myMenuRecordingDetails -------------------------------------------------
myMenuRecordingDetails::myMenuRecordingDetails(cRecording *Recording):cOsdMenu(tr("Details"),12)
{
  recording=Recording;
#if VDRVERSNUM > 10720
  priority = recording->Priority();
  lifetime = recording->Lifetime();
#else
  priority = recording->priority;
  lifetime = recording->lifetime;
#endif

  Add(new cMenuEditIntItem(trVDR("Priority"),&priority,0,MAXPRIORITY));
  Add(new cMenuEditIntItem(trVDR("Lifetime"),&lifetime,0,MAXLIFETIME));
}

eOSState myMenuRecordingDetails::ProcessKey(eKeys Key)
{
  eOSState state=cOsdMenu::ProcessKey(Key);
  if(state==osUnknown)
  {
    if(Key==kOk)
    {
#if VDRVERSNUM > 10720
      int old_priority = recording->Priority();
      int old_lifetime = recording->Lifetime();
#else
      int old_priority = recording->priority;
      int old_lifetime = recording->lifetime;
#endif
      if((priority!=old_priority)||(lifetime!=old_lifetime))
      {
#if VDRVERSNUM > 10713
        if(recording->IsPesRecording())
#endif
        {
          char *oldname=strdup(recording->FileName());
          char *_newname=strdup(recording->FileName());

          sprintf(_newname+strlen(_newname)-9,"%02d.%02d.rec",priority,lifetime);

          if(MoveRename(oldname,_newname,recording,false))
            state=osBack;
          else
            state=osContinue;

          free(oldname);
          free(_newname);
        }
#if VDRVERSNUM > 10713
        else
        {
          cString buffer = cString::sprintf("P %d\nL %d", priority, lifetime);
          if(ModifyInfo(recording,*buffer))
          {
            cString fileName = recording->FileName();
            Recordings.Del(recording);
            Recordings.AddByName(*fileName);
            state=osBack;
          }
          else
            state=osContinue;
        }
#endif
      }
      else
        state=osBack;
    }
  }
  return state;
}

#define INFOFILE_PES "info.vdr"
#define INFOFILE_TS "info"
bool myMenuRecordingDetails::ModifyInfo(cRecording *Recording, const char *Info)
{ //This has been taken from remotetimers-0.1.5, written by Frank Schmirler <vdrdev@schmirler.de>

#if VDRVERSNUM > 10713
  cString InfoFileName=cString::sprintf(Recording->IsPesRecording()?"%s/"INFOFILE_PES:"%s/"INFOFILE_TS,Recording->FileName());
	FILE *f = fopen(InfoFileName, "a");
	if (f)
	{
		fprintf(f, "%s\n", Info);
		fclose(f);
		// Casting const away is nasty, but what the heck?
		// The Recordings thread is locked and the object is going to be deleted anyway.
		if (((cRecordingInfo *)Recording->Info())->Read() && Recording->WriteInfo())
			return true;
		esyslog("[extrecmenu] Failed to update '%s'", *InfoFileName);
	}
	else
		esyslog("[extrecmenu] writing to '%s' failed: %m", *InfoFileName);
#else
  cString InfoFileName=cString::sprintf("%s/"INFOFILE_PES,Recording->FileName());
  // check for write access as cRecording::WriteInfo() always returns true
  if(access(InfoFileName,W_OK)==0)
  {
    FILE *f=fmemopen((void *)Info,strlen(Info)*sizeof(char),"r");
    if(f)
    {
      // Casting const away is nasty, but what the heck?
      // The Recordings thread is locked and the object is going to be deleted anyway.
      if(((cRecordingInfo *)Recording->Info())->Read(f)&&Recording->WriteInfo())
        return true;
      esyslog("[extrecmenu] error in info string '%s'",Info);
    }
    else
      esyslog("[extrecmenu] error in fmemopen: %m");
  }
  else
    esyslog("[extrecmenu] '%s' not writeable: %m",*InfoFileName);
#endif
  return false;
}
