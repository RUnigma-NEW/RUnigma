/*
 * See the README file for copyright information and how to reach the author.
 *
 * This code is directly taken from VDR with some changes by me to work with this plugin
 */

#include <vdr/menu.h>
#include <vdr/config.h>
#include <vdr/interface.h>
#include "mymenucommands.h"

#if VDRVERSNUM > 10713
myMenuCommands::myMenuCommands(const char *_Title,cList<cNestedItem> *_Commands,const char *Parameters):cOsdMenu(_Title)
#else
myMenuCommands::myMenuCommands(const char *_Title,cCommands *_Commands, const char *Parameters):cOsdMenu(_Title)
#endif
{
 SetHasHotkeys();
 commands=_Commands;
#if VDRVERSNUM > 10713
 result=NULL;
 parameters=Parameters;
 for(cNestedItem *Command=commands->First();Command;Command=commands->Next(Command)) {
  const char *s=Command->Text();
  if(Command->SubItems())
   Add(new cOsdItem(hk(cString::sprintf("%s...", s))));
  else if(Parse(s))
   Add(new cOsdItem(hk(title)));
 }
#else
 parameters=Parameters?strdup(Parameters):NULL;
 for(cCommand *command=commands->First();command;command=commands->Next(command))
  Add(new cOsdItem(hk(command->Title())));
#endif
}

myMenuCommands::~myMenuCommands()
{
#if VDRVERSNUM > 10713
 free(result);
#else
 free(parameters);
#endif
}

#if VDRVERSNUM > 10713
bool myMenuCommands::Parse(const char *s)
{
 const char *p=strchr(s,':');
 if(p) {
  int l=p-s;
  if(l>0) {
   char t[l+1];
   stripspace(strn0cpy(t,s,l+1));
   l=strlen(t);
   if(l>1&&t[l-1]=='?') {
    t[l-1]=0;
    confirm=true;
   }
   else
    confirm=false;
   title=t;
   command=skipspace(p+1);
   return true;
  }
 }
 return false;
}
#endif

#if VDRVERSNUM > 10713
eOSState myMenuCommands::Execute()
{
 cNestedItem *Command=commands->Get(Current());
 if(Command) {
  if(Command->SubItems())
   return AddSubMenu(new myMenuCommands(Title(),Command->SubItems(),parameters));
  if(Parse(Command->Text())) {
   if(!confirm||Interface->Confirm(cString::sprintf("%s?",*title))) {
    Skins.Message(mtStatus,cString::sprintf("%s...",*title));
    free(result);
    result=NULL;
    cString cmdbuf;
    if(!isempty(parameters))
     cmdbuf=cString::sprintf("%s %s",*command,*parameters);
    const char *cmd=*cmdbuf?*cmdbuf:*command;
    dsyslog("executing command '%s'",cmd);
    cPipe p;
    if(p.Open(cmd,"r")) {
     int l=0;
     int c;
     while((c=fgetc(p))!=EOF) {
      if(l%20==0)
       result=(char *)realloc(result,l+21);
      result[l++]=char(c);
     }
     if(result)
      result[l]=0;
     p.Close();
    }
    else
     esyslog("ERROR: can't open pipe for command '%s'",cmd);
    Skins.Message(mtStatus,NULL);
    if(result)
     return AddSubMenu(new cMenuText(title,result,fontFix));
    return osEnd;
   }
  }
 }
 return osContinue;
}
#else
eOSState myMenuCommands::Execute()
{
 cCommand *command=commands->Get(Current());
 if(command)
 {
  char *buffer=NULL;
  bool confirmed=false;
#ifdef CMDSUBMENUVERSNUM
  if (command->hasChilds())
  {
   AddSubMenu(new myMenuCommands(command->Title(), command->getChilds(), parameters));
   return osContinue;
  }
#endif
  if(command->Confirm())
  {
   if(asprintf(&buffer,"%s?",command->Title())!=-1)
   {
    confirmed=Interface->Confirm(buffer);
    free(buffer);
   }
  } else {
    confirmed=true;
  }
  if(confirmed)
  {
   if(asprintf(&buffer, "%s...",command->Title())!=-1)
   {
    Skins.Message(mtStatus,buffer);
    free(buffer);
   }
   const char *Result=command->Execute(parameters);
   Skins.Message(mtStatus,NULL);
   if(Result)
    return AddSubMenu(new cMenuText(command->Title(),Result,fontFix));
   return osEnd;
  }
 }
 return osContinue;
}
#endif

eOSState myMenuCommands::ProcessKey(eKeys Key)
{
 eOSState state=cOsdMenu::ProcessKey(Key);

 if(state==osUnknown)
 {
  switch(Key)
  {
   case kRed:
   case kGreen:
   case kYellow:
   case kBlue: return osContinue;
   case kOk: return Execute();
   default: break;
  }
 }
 return state;
}
