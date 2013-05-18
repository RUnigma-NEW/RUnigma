/****************************************************************************
 * DESCRIPTION:
 *             Creates VDR Menus
 *
 * $Id: setupmenu.cpp,v 1.21 2006/03/05 09:47:26 ralf Exp $
 *
 * Contact:    ranga@vdrtools.de
 *
 * Copyright (C) 2005 by Ralf Dotzert
 ****************************************************************************/

#include <stdlib.h>
#include <vdr/menuitems.h>
#if APIVERSNUM >= 10501
#include <vdr/shutdown.h>
#endif
#include <string>

#include "setupmenu.h"
#include "config.h"
#include "plugins.h"
#include "debug.h"
#include "i18n.h"
#include "setupsetup.h"
#include "setupsystemmenu.h"



//#################################################################################################
cSetupPluginMenu::cSetupPluginMenu(Config  *config) : cOsdMenu(tr("Plugins activate / deactivate"), 42)
{
  _moveMode = false;
  _sortMode = false;

  if (config != NULL && config->GetPlugins() != NULL) {
     _config  = config;
     _plugins = config->GetPlugins();
     Set();
    }
}

cSetupPluginMenu::~cSetupPluginMenu()
{
}

/**
 * 
 */
void cSetupPluginMenu::Set( )
{

  int current = Current();

  Clear();

  for (int i=0; i<_plugins->GetNr(); i++) {
      if (_plugins->Get(i)->GetProtect()) {
         char *title = NULL;
         asprintf(&title, "%s:\t%s", _plugins->Get(i)->GetInfo(),tr("protected")); 
         Add(new cOsdItem (title));
         free(title);
         }
      else
         Add(new cMenuEditBoolItem(_plugins->Get(i)->GetInfo(), _plugins->Get(i)->GetActiveRef(), trVDR("no"), trVDR("yes")));
      }

  if (current == -1 && _plugins->GetNr() > 0)
     current = 0;
  SetCurrent(Get(current));

  if (_moveMode)
     SetHelp(tr("PageUp"),  tr("PageDown"), tr("Before"), tr("After"));
  else
     SetHelp(tr("PageUp"),  tr("PageDown"), trVDR("Parameters"), tr("Move"));

  setHelp();
  Display();
}

eOSState cSetupPluginMenu::ProcessKey( eKeys Key )
{
  bool HadSubMenu = HasSubMenu();
  eOSState state = cOsdMenu::ProcessKey(Key);
  if (HasSubMenu() || HadSubMenu)
     return state;

  switch (Key) {
    case kOk :
                   _config->SaveFile();
                   return osBack;
                   break;

    case kRed:
                   PageUp();
                   Set();
                   break;

    case kGreen:
                   PageDown();
                   Set();
                   break;
    case kYellow:
                   if (_moveMode) {
                      _plugins->MovePlugin(_startIndex, Current(), Plugins::BEFORE);
                      _moveMode = !_moveMode;
                      Set();
                      }
                   else { // Edit Parameter
                      char *tmp=NULL;
                      asprintf(&tmp, "%s-%s", trVDR("Plugin"), trVDR("Parameters"));
                      return(AddSubMenu(new cSetupPluginParameter(tmp, _plugins->Get(Current()))));
                      free(tmp);
                      }
                   break;
    case kBlue:
                   if (_moveMode) {
                      _plugins->MovePlugin(_startIndex, Current(), Plugins::BEHIND);
                      }
                   else {
                      _startIndex = Current();
                      }
                   _moveMode = !_moveMode;
                   Set();
                   break;
    case kDown:
    case kUp:
                   setHelp();
                   break;
    default:       break;
    }
  return state;
}

void cSetupPluginMenu::setHelp()
{
  char *tmp = NULL;
  int  current = Current();
  if (current >= 0 ) {
     asprintf(&tmp, "%s: %s", trVDR("Plugin"), _plugins->Get(current)->GetName());
     SetStatus(tmp);
     free(tmp);
     }
}


//#################################################################################################
//  Edit Plugin Parameter
//################################################################################################

cSetupPluginParameter::cSetupPluginParameter( const char *title, Plugin * plugin ) : cOsdMenu(title, 25)
{
  _plugin = plugin;
  _edit   = false;
  Set();
}

cSetupPluginParameter::~ cSetupPluginParameter( )
{
}

void cSetupPluginParameter::Set( )
{
  char *tmp = NULL;
  const char *param = _plugin->GetParameter();
  Clear();
  if (param == NULL)
     _editParameter[0] = '\0';
  else {
     strncpy(_editParameter, param, sizeof(_editParameter));
     _editParameter[sizeof(_editParameter)] = '\0';
     }
  asprintf(&tmp, "%s-%s", trVDR("Plugin"), trVDR("Parameters"));
  Add(new cMenuEditStrItem(tmp, _editParameter, sizeof(_editParameter), trVDR(FileNameChars)));

  asprintf(&tmp, "%s: %s", trVDR("Plugin"), _plugin->GetName());
  SetStatus(tmp);
  free(tmp);
  Display();
}

eOSState cSetupPluginParameter::ProcessKey( eKeys Key )
{

  eOSState state = cOsdMenu::ProcessKey(Key);

  switch (Key) {
    case kOk:
              if (!_edit)
                 return osBack;
              else {
                 _plugin->SetParameter(_editParameter);
                 _edit=false;
                 }
              break;
    case kRight:
              _edit = true;
              break;
    default:
              break;
    }
  return state;
}


//#################################################################################################
//  cSetupVdrMenu
//################################################################################################

cSetupVdrMenu::cSetupVdrMenu(const char *title): cOsdMenu(title, 25)
{
  _startIndex           = 0;
  _createEditNodeIndex  = 0;

  // Load Menu Configuration
  char *menuXML = NULL;
#if VDRVERSNUM < 10507
  asprintf(&menuXML, "%s/setup/vdr-menu.%i.xml", cPlugin::ConfigDirectory(), Setup.OSDLanguage);
#else
  asprintf(&menuXML, "%s/setup/vdr-menu.%s.xml", cPlugin::ConfigDirectory(), Setup.OSDLanguage);
#endif
  if (access(menuXML, 06) == -1)
     asprintf(&menuXML, "%s/setup/vdr-menu.xml", cPlugin::ConfigDirectory());
  isyslog("setup: loading %s", menuXML);
  _vdrSubMenu.LoadXml(menuXML);
  free(menuXML);

  _menuState  = UNDEFINED;
  Set();
}

cSetupVdrMenu::~ cSetupVdrMenu( )
{
}

void cSetupVdrMenu::Set( )
{
  int current = Current();
  cSubMenuNode *node = NULL;
  int nr = _vdrSubMenu.GetNrOfNodes();

  Clear();

  switch (_menuState) {
    case UNDEFINED:
                SetTitle(tr("Menu Edit"));
    case MOVE:
                for (int i=0; i<nr; i++) {
                    if ((node = _vdrSubMenu.GetAbsNode(i)) != NULL) {
                       char *tmp = createMenuName(node);
                       Add(new cOsdItem(tmp));
                       free(tmp);
                       }
                    }
                current = _createEditNodeIndex;
                if (current >= nr)
                   current = nr-1;
                break;
    case EDIT:
                if (_vdrSubMenu.GetAbsNode(_createEditNodeIndex)->GetType() == cSubMenuNode::PLUGIN) {
/*
                   char *name = NULL;
                   asprintf(&name, "%s:\t%s", trVDR("Name"), _vdrSubMenu.GetAbsNode(_createEditNodeIndex)->GetName());
                   Add(new cOsdItem(name));
                   free(name);
*/
                   if (_vdrSubMenu.GetAbsNode(_createEditNodeIndex)->GetCustomTitle() != NULL)
                      strncpy(_editTitle, _vdrSubMenu.GetAbsNode(_createEditNodeIndex)->GetCustomTitle(), sizeof(_editTitle));
                   else
                      strncpy(_editTitle, "", sizeof(_editTitle));
                   _editTitle[sizeof(_editTitle)] = '\0';
                   Add(new cMenuEditStrItem(tr("Title"), _editTitle, sizeof(_editTitle), trVDR(FileNameChars)));

                   char *menutitle = NULL;
                   asprintf(&menutitle, "%s '%s'", tr("Edit Plugin"), _vdrSubMenu.GetAbsNode(_createEditNodeIndex)->GetName());
                   SetTitle(menutitle);
                   free(menutitle);
                   }
                else if (_vdrSubMenu.GetAbsNode(_createEditNodeIndex)->GetType() == cSubMenuNode::COMMAND ||
                         _vdrSubMenu.GetAbsNode(_createEditNodeIndex)->GetType() == cSubMenuNode::THREAD) {
                   strncpy(_editTitle, _vdrSubMenu.GetAbsNode(_createEditNodeIndex)->GetName(), sizeof(_editTitle));
                   _editTitle[sizeof(_editTitle)] = '\0';
                   Add(new cMenuEditStrItem(trVDR("Name"), _editTitle, sizeof(_editTitle), trVDR(FileNameChars)));

                   strncpy(_editCommand, _vdrSubMenu.GetAbsNode(_createEditNodeIndex)->GetCommand(), sizeof(_editCommand));
                   _editTitle[sizeof(_editCommand)] = '\0';
                   Add(new cMenuEditStrItem(tr("Command"), _editCommand, sizeof(_editCommand), trVDR(FileNameChars)));

                   if (_vdrSubMenu.GetAbsNode(_createEditNodeIndex)->CommandConfirm())
                      _hasToConfirm = 1;
                   else
                      _hasToConfirm = 0;
                   Add(new cMenuEditBoolItem(tr("Has to confirm"), &_hasToConfirm));

                   if (_vdrSubMenu.GetAbsNode(_createEditNodeIndex)->GetType() == cSubMenuNode::THREAD)
                      _isThread = 1;
                   else
                      _isThread = 0;
                   Add(new cMenuEditBoolItem(tr("Execute as thread"), &_isThread));

                   SetTitle(tr("Edit Command"));
                   }
                else { // MENU
                   strncpy(_editTitle, _vdrSubMenu.GetAbsNode(_createEditNodeIndex)->GetName(), sizeof(_editTitle));
                   _editTitle[sizeof(_editTitle)] = '\0';
                   Add(new cMenuEditStrItem(trVDR("Name"), _editTitle, sizeof(_editTitle), trVDR(FileNameChars)));

                   SetTitle(tr("Edit Menu"));
                   }

                setHelp();
                break;
    case CREATE:
                strncpy(_editTitle, "", sizeof(_editTitle));
                Add(new cMenuEditStrItem(trVDR("Name"), _editTitle, sizeof(_editTitle), trVDR(FileNameChars)));
                SetTitle(tr("Create Menu"));
                setHelp();
                break;
    case CREATECMD:
                strncpy(_editTitle, "", sizeof(_editTitle));
                Add(new cMenuEditStrItem(trVDR("Name"), _editTitle, sizeof(_editTitle), trVDR(FileNameChars)));
                strncpy(_editCommand, "", sizeof(_editCommand));
                Add(new cMenuEditStrItem(tr("Command"), _editCommand, sizeof(_editCommand), trVDR(FileNameChars)));

                _hasToConfirm = 1;
                Add(new cMenuEditBoolItem(tr("Has to confirm"), &_hasToConfirm));

                _isThread = 0;
                Add(new cMenuEditBoolItem(tr("Execute as thread"), &_isThread));
                SetTitle(tr("Create Command"));
                setHelp();
                break;
    default:
                break;
    }

  SetCurrent(Get(current));
  setHelp();
  Display();
}




eOSState cSetupVdrMenu::ProcessKey( eKeys Key )
{
    cSubMenuNode *node = NULL;
    eOSState state = cOsdMenu::ProcessKey(Key);
    if (HasSubMenu())
       return state;
//    isyslog("setup: %d", state); // osUnknown=0, osContinue=1, osBack=16
    if (state == osUnknown || state == osBack) {
       switch (Key) {
         case k0:
                       if (_menuState == UNDEFINED) {
                          _menuState = CREATECMD;
                          _createEditNodeIndex = Current();
                          Set();
                          }
                       break;
         case kRed:
                      if (_menuState == CREATE) {
                          _menuState = CREATECMD;
                          Set();
                          }
                       else if (_menuState == CREATECMD) {
                          _menuState = CREATE;
                          Set();
                          }
                       else if (_menuState == UNDEFINED) {
                          _menuState = CREATE;
                          _createEditNodeIndex = Current();
                          Set();
                          }
                       break;
         case kGreen:
                       if (_menuState == MOVE) {
                          if ((node = _vdrSubMenu.GetAbsNode(Current())) != NULL && node->GetType() == cSubMenuNode::MENU) {
                             _vdrSubMenu.MoveMenu(_startIndex, Current(), cSubMenu::INTO);
                             _menuState = UNDEFINED;
                             _createEditNodeIndex = Current();
                             SetStatus(NULL);
                             Set();
                             }
                          }
                       else if (_menuState == UNDEFINED) {
                          if ((node = _vdrSubMenu.GetAbsNode(Current())) != NULL && (node->GetType() == cSubMenuNode::MENU ||
                                                                                     node->GetType() == cSubMenuNode::PLUGIN ||
                                                                                     node->GetType() == cSubMenuNode::COMMAND ||
                                                                                     node->GetType() == cSubMenuNode::THREAD)) {
                             _menuState = EDIT;
                             _createEditNodeIndex = Current();
                             Set();
                             }
                          }
                       setHelp();
                       break;
         case kYellow:
                       if (_menuState == MOVE) {
                          _vdrSubMenu.MoveMenu(_startIndex, Current(), cSubMenu::BEFORE);
                          _menuState = UNDEFINED;
                          _createEditNodeIndex = Current();
                          SetStatus(NULL);
                          Set();
                          }
                       else if (_menuState == UNDEFINED) {
                          if (Interface->Confirm(tr("Delete Menu?"))) {
                             _createEditNodeIndex = Current();
                             _vdrSubMenu.DeleteMenu(_createEditNodeIndex);
                             _menuState = UNDEFINED;
                             Set();
                             }
                          }
                       break;
         case kBlue:
                       if (_menuState == MOVE) {
                          _vdrSubMenu.MoveMenu(_startIndex, Current(), cSubMenu::BEHIND);
                          _createEditNodeIndex = Current();
                          _menuState = UNDEFINED;
                          SetStatus(NULL);
                          }
                       else if (_menuState == UNDEFINED) {
                          _startIndex = Current();
                          _createEditNodeIndex = _startIndex;
                          _menuState = MOVE;
                          SetStatus(tr("Up/Dn for new location - color key to move"));
                          }
                       Set();
                       break;

         case kOk:
                       switch (_menuState) {
                         case UNDEFINED:
                                    // Save Menus to file and exit submenu
                                     state = osBack;
                                    _vdrSubMenu.SetMenuSuffix(setupSetup._menuSuffix);
                                    _vdrSubMenu.SaveXml();
                                    break;
                         case CREATE:
                                    _vdrSubMenu.CreateMenu(_createEditNodeIndex, _editTitle);
                                    _menuState = UNDEFINED;
                                    Set();
                                    break;
                         case CREATECMD:
                                    if (_isThread == 1)
                                       _vdrSubMenu.CreateThread(_createEditNodeIndex, _editTitle, _editCommand, _hasToConfirm);
                                    else
                                       _vdrSubMenu.CreateCommand(_createEditNodeIndex, _editTitle, _editCommand, _hasToConfirm);
                                    _menuState = UNDEFINED;
                                    Set();
                                    break;
                         case EDIT:
                                    if (_vdrSubMenu.GetAbsNode(_createEditNodeIndex)->GetType() == cSubMenuNode::PLUGIN)
                                       _vdrSubMenu.GetAbsNode(_createEditNodeIndex)->SetCustomTitle(_editTitle);
                                    else if (_vdrSubMenu.GetAbsNode(_createEditNodeIndex)->GetType() == cSubMenuNode::COMMAND ||
                                             _vdrSubMenu.GetAbsNode(_createEditNodeIndex)->GetType() == cSubMenuNode::THREAD) {
                                       _vdrSubMenu.GetAbsNode(_createEditNodeIndex)->SetName(_editTitle);
                                       if (_isThread == 1)
                                          _vdrSubMenu.GetAbsNode(_createEditNodeIndex)->SetType(cSubMenuNode::THREAD);
                                       else
                                          _vdrSubMenu.GetAbsNode(_createEditNodeIndex)->SetType(cSubMenuNode::COMMAND);
                                       _vdrSubMenu.GetAbsNode(_createEditNodeIndex)->SetCommand(_editCommand);
                                       _vdrSubMenu.GetAbsNode(_createEditNodeIndex)->SetCommandConfirm(_hasToConfirm);
                                       }
                                    else
                                       _vdrSubMenu.GetAbsNode(_createEditNodeIndex)->SetName(_editTitle);
                                    _menuState = UNDEFINED;
                                    Set();
                                    break;

                         default:
                                    break;
                         }
                       break;
         case kBack:
                       state = osBack;
                       if (_menuState == CREATE || _menuState == CREATECMD || _menuState == EDIT) {
                          _menuState = UNDEFINED;
                          Set();
                          state = osContinue;
                          }
                       break;
         case kDown:
         case kUp:
         case kRight:
         case kLeft:
                       if (_menuState == MOVE)
                          setHelp();
                       break;
         default:
                       break;
         }
      }
   else if (state == osContinue) {
       switch (Key) {
         case kBack:
                      setHelp();
                      break;
         case kDown:
         case kUp:
         case kRight:
         case kLeft:
                       if (_menuState == MOVE)
                          setHelp();
                       break;
         default:
                       break;
         }
      }

   return state;
}

// --------------- Private Methods ---------------------
void cSetupVdrMenu::setHelp( )
{
  cSubMenuNode *node = NULL;

   if (_menuState == CREATE)
     SetHelp(tr("Command"), NULL, NULL, NULL);
  else if (_menuState == CREATECMD)
     SetHelp(tr("Menu"), NULL, NULL, NULL);
  else if (_menuState == EDIT)
     SetHelp(NULL, NULL, NULL, NULL);
  else if (_menuState == MOVE) {
     if ((node= _vdrSubMenu.GetAbsNode(Current())) != NULL && node->GetType() == cSubMenuNode::MENU)
        SetHelp(NULL, tr("Into"), tr("Before"), tr("After"));
     else
        SetHelp(NULL, NULL, tr("Before"), tr("After"));
     }
  else
     SetHelp(tr("Create"), tr("Edit"), tr("Delete"), tr("Move"));
}

char * cSetupVdrMenu::createMenuName( cSubMenuNode *node )
{
  char *prefix = NULL;
  char *tmp = NULL;
  int   level = node->GetLevel();

  // Set Prefix
  prefix = (char*) malloc(1);
  prefix[0] = '\0';
  for (int i=0; i<level; i++)
      asprintf(&prefix, "|   %s", prefix);

  cSubMenuNode::Type type = node->GetType();

  if (type == cSubMenuNode::MENU)
     asprintf(&tmp, "%s+%s", prefix, node->GetName());
  else if (type == cSubMenuNode::SYSTEM)
     asprintf(&tmp, "%s %s", prefix, trVDR(node->GetName()));
  else if (type == cSubMenuNode::PLUGIN)
     if (node->GetCustomTitle() != NULL && strcmp(node->GetCustomTitle(), "") != 0)
        asprintf(&tmp, "%s %s  (%s)", prefix, node->GetCustomTitle(), node->GetName());
     else
        asprintf(&tmp, "%s %s  (%s)", prefix, node->GetPluginMainMenuEntry(), node->GetName());
  else
     asprintf(&tmp, "%s %s",  prefix, node->GetName());

  free(prefix);

  return(tmp);
}

//#################################################################################################//  cSetupGenericMenu
//################################################################################################
cSetupGenericMenu::cSetupGenericMenu(const char *title, MenuNode *node, Config  *config)  : cSetupMenu()
{
  _node     = node;
  _editItem = false;
  _currLine = 0;
  _config   = config;

  SetTitle(title);
  SetCols(25);

  if (_node != NULL)
     Set();
}


cSetupGenericMenu::~cSetupGenericMenu()
{
}

const char * cSetupGenericMenu::nohk(const char *str)
{
  static std::string tmp;
  tmp = setupSetup._entryPrefix;

  if (strlen(setupSetup._entryPrefix) == 0 || setupSetup._entryPrefix[0] == ' ')
     tmp = "      ";
  else {
     tmp =  "  ";
     tmp += setupSetup._entryPrefix;
     tmp += "  ";
     }

  tmp += str;
  return(tmp.c_str());
}

/**
 * 
 */
void cSetupGenericMenu::Set( )
{
  int current = Current();
  Clear();
  SetHasHotkeys();
  for (int i=0; i< _node->GetNr(); i++) {
      MenuNode::Type type = _node->GetNode(i)->GetType();

     if (type == MenuNode::ENTRY) {
        MenuEntry *e = _node->GetNode(i)->GetMenuEntry();

        switch (e->GetType()) {
          case Util::BOOL:
             Add(new cMenuEditBoolItem(nohk(e->GetName()), e->GetValueBoolRef(), trVDR("no"), trVDR("yes")));
             break;
          case Util::NUMBER:
             Add(new cMenuEditIntItem(nohk(e->GetName()), e->GetValueNumberRef(), 0, 999999999));
             break;
          case Util::TEXT:
             Add(new cMenuEditStrItem(nohk((char*)e->GetName()), (char*)e->GetValue(), e->GetValueTextMaxLen(), " aäbcdefghijklmnoöpqrsßtuüvwxyz0123456789-.#~_"));
             break;
          case Util::NUMBER_TEXT:
             Add(new cMenuEditStrItem(nohk((char*)e->GetName()), (char*)e->GetValue(), e->GetValueTextMaxLen(), "0123456789"));
             break;
          case Util::IP:
             Add(new cMenuEditStrItem(nohk((char*)e->GetName()), (char*)e->GetValueIp(), e->GetValueIpMaxLen(), ".0123456789"));
             break;
          case Util::HEX:
             Add(new cMenuEditStrItem(nohk((char*)e->GetName()), (char*)e->GetValue(), e->GetValueTextMaxLen(), "0123456789ABCDEF:"));
             break;
          case Util::SELECTION:
             {
               if (e->GetNrOfSelectionValues() == 0) {
                  char *txt = NULL;
                  asprintf(&txt, "%s:\t%s", nohk(e->GetName()), tr("missing channels/xxx.conf"));
                  Add(new cOsdItem (txt));
                  free(txt);
                  }
               else
                  Add(new cMenuEditStraItem(nohk(e->GetName()), e->GetReferenceSelection(),
                                                                e->GetNrOfSelectionValues(),
                                                                e->GetSelectionValues()) );
             }
             break;
          default:
             break;
          }
        }
     else if (type == MenuNode::MENUSYSTEM) {
        char *tmp = NULL;
        asprintf(&tmp, "%s", _node->GetNode(i)->GetMenu()->GetSystem());
        if ((strcmp(tmp, "ActPlugins") == 0) || (strcmp(tmp, "VDRMenu") == 0))
           asprintf(&tmp, "%s%s", tr(_node->GetNode(i)->GetName()), setupSetup._menuSuffix);
        else
           asprintf(&tmp, "%s%s", trVDR(_node->GetNode(i)->GetName()), setupSetup._menuSuffix);
        Add(new cOsdItem(hk(tmp)));
        free(tmp);
        }
     else {
        char *tmp = NULL;
        asprintf(&tmp, "%s%s", _node->GetNode(i)->GetName(), setupSetup._menuSuffix);
        Add(new cOsdItem(hk(tmp)));
        free(tmp);
        }
     }
  SetCurrent(Get(current));
  Display();
}

eOSState cSetupGenericMenu::ProcessKey( eKeys Key )
{
  bool HadSubMenu = HasSubMenu();
  eOSState state = cOsdMenu::ProcessKey(Key);

  if (HasSubMenu() || HadSubMenu)
     return state;

  MenuNode *n = _node->GetNode(Current());

  switch (Key) {
    case kOk :
                    if (n != NULL && (n->GetType() == MenuNode::MENU || n->GetType() == MenuNode::MENUSYSTEM)) {
                       if (n->GetType() == MenuNode::MENU)
                          return(AddSubMenu(new cSetupGenericMenu(n->GetName(), n,  _config)));
                       else {  // Menu "system"
                          cOsdMenu *menu = cSetupSystemMenu::GetSystemMenu(n->GetMenu()->GetSystem(), _config);
                          if (menu != NULL)
                             return(AddSubMenu(menu));
                          }
                       }
                    else {
                       if (!_editItem) { //close submenu only if we do not have edited an textitem
                          state = osBack;
                          _config->SaveFile(); // Write New Configurationfile

                          for (int i=0;  i < _node->GetNr(); i++) {
                              n = _node->GetNode(i);
                              if ( n->GetType() == MenuNode::ENTRY)
                                 ExecuteCommand(n->GetMenuEntry()->GetCommand());
                              }
                          ExecuteCommand(_node->GetMenu()->GetCommand());
                          }

                       _editItem = false;
                       }
                    break;
     case kRed:
                    {
                       char *help = NULL;
                       if (n->GetType() == MenuNode::ENTRY)
                          help = GetLongHelp(n->GetMenuEntry()->GetHelp2());
                       else
                          help = GetLongHelp(n->GetMenu()->GetHelp2());

                       if (help != NULL) {
                          eOSState newState = AddSubMenu(new cMenuText(tr("Help"), help, fontFix));
                          free(help);
                          return(newState);
                          }
                       }
                    break;
     case kGreen:
                    break;
     case kYellow:
                    break;
     case kBlue:
                    break;
     case kRight:
                    if (n!= NULL &&
                        n->GetType() == MenuNode::ENTRY &&
                       (n->GetMenuEntry()->GetType() == Util::TEXT ||
                        n->GetMenuEntry()->GetType() == Util::NUMBER_TEXT ||
                        n->GetMenuEntry()->GetType() == Util::HEX ||
                        n->GetMenuEntry()->GetType() == Util::NUMBER)
                        )
                        _editItem = true;
                    break;

     default:
                    if (n!= NULL) {
                       if (n->GetType() == MenuNode::ENTRY) {
                          SetStatus(n->GetMenuEntry()->GetHelp1());
                          if (n->GetMenuEntry()->GetHelp2() != NULL)
                             SetHelp(tr("Help"), NULL, NULL, NULL);
                          else
                             SetHelp(NULL, NULL, NULL, NULL);
                          }
                       else {
                          SetStatus(n->GetMenu()->GetHelp1());
                          if (n->GetMenu()->GetHelp2() != NULL)
                             SetHelp(tr("Help"), NULL, NULL, NULL);
                          else
                             SetHelp(NULL, NULL, NULL, NULL);
                          }
                       }
                    else
                       SetStatus(NULL);
                    break;
     }
   return state;
}

void cSetupGenericMenu::ExecuteCommand( const char * cmd )
{
  char *tmp=NULL;
  int   status=0;

  if (cmd != NULL) {
     asprintf(&tmp, "%s: %s", tr("Execute"), cmd);
     SetStatus(tmp);
     free(tmp);
     status=system(cmd);
     if (status == -1)
        DEBUG3("%s: fork of command %s failed\n", DBG_PREFIX, cmd);
     else {
       if (WEXITSTATUS(status) != 0)
          DEBUG4("%s: executing of command %s returned=%d\n", DBG_PREFIX, cmd, WEXITSTATUS(status));
       }
     }
}




//#############################################################################################
// cSetupMenu
//#############################################################################################


cSetupMenu::cSetupMenu() : cOsdMenu(tr("Setup"))
{
  char *configFile=NULL;

  SetCols(20);
#if VDRVERSNUM < 10507
  asprintf(&configFile, "%s/setup/vdr-setup.%i.xml", cPlugin::ConfigDirectory(), Setup.OSDLanguage);
#else
  asprintf(&configFile, "%s/setup/vdr-setup.%s.xml", cPlugin::ConfigDirectory(), Setup.OSDLanguage);
#endif
  if (access(configFile, 06) == -1)
     asprintf(&configFile, "%s/setup/vdr-setup.xml", cPlugin::ConfigDirectory());
  isyslog("setup: loading %s", configFile);

  _config    = new Config( configFile );
  free(configFile);

  _number    = 0;
  _error     = false;
  _childLock = false;
  _childLockEntered = NULL;
  _childLockEnteredHidden = NULL;

  if (_config != NULL && _config->LoadFile() == true) {
     if (_config->GetChildLock() != NULL && strcmp(_config->GetChildLock(), "0000") != 0) {
        _childLock = true;
        _childLockEntered          = Util::Strdupnew(_config->GetChildLock());
        _childLockEntered[0]       = '\0';
        _childLockEnteredHidden    = Util::Strdupnew(_config->GetChildLock());
        _childLockEnteredHidden[0] = '\0';
        SetAskChildCode();
        }
     else
        Set();
     }
  else {
     SetStatus(tr("Error in configuration files"));
     _error = true;
     }
}


cSetupMenu::~cSetupMenu()
{
  delete _config;
  delete []_childLockEntered;
  delete [] _childLockEnteredHidden;
}

/**
 * 
 */
void cSetupMenu::Set( )
{
  int current  = Current();
  Menus     *m = _config->GetMenus();
  MenuNode  *n = NULL;

  Clear();
  SetHasHotkeys();

  // Customized Setup Menus
  for (int i=0; i< m->GetNr(); i++) {
      n = m->GetMenuNode(i);
      char *tmp = NULL;
      asprintf(&tmp, "%s%s", n->GetName(), setupSetup._menuSuffix);
      Add(new cOsdItem (hk(tmp)));
      free(tmp);
      }
  SetCurrent(Get(current));
  SetHelp(NULL, NULL, trVDR("Restart"), tr("Reboot"));

  Display();
}

/**
 * Ask the user for the secret Child Code
 */
void cSetupMenu::SetAskChildCode( )
{
  Clear();
  char *tmp = NULL;
  asprintf(&tmp, "%s%s", tr("Enter Pin: "), _childLockEnteredHidden);
  Add(new cOsdItem (tmp));
  free(tmp);
  Display();
}




/**
 * Procss Key
 * @param Key 
 * @return 
 */
eOSState cSetupMenu::ProcessKey( eKeys Key )
{
  if (_childLock)
     return(GetCodeProcessKey(Key));
  else
     return(StandardProcessKey(Key));
}

/**
 * Standard Key Processing
 * @param Key 
 * @return 
 */
eOSState cSetupMenu::StandardProcessKey( eKeys Key )
{
    bool HadSubMenu = HasSubMenu();

    eOSState state = cOsdMenu::ProcessKey(Key);

    if (HasSubMenu() || HadSubMenu)
       return(state);

    int current = Current();
    MenuNode *node = _config->GetMenus()->GetMenuNode(current);
    switch (Key) {
      case kOk :
                      if (node != NULL) {
                         if (node->GetType() == MenuNode::MENU) {
                            return AddSubMenu(new cSetupGenericMenu(node->GetName(), node, _config));
                            }
                         else if (node->GetType() == MenuNode::MENUSYSTEM) {
                            cOsdMenu * menu =cSetupSystemMenu::GetSystemMenu (hk(node->GetMenu()->GetSystem()), _config);
                            if (menu != NULL)
                               AddSubMenu(menu);
                            }
                         SetCurrent(Get(current));
                         return(osContinue);
                         }
                      break;
      case kRed:
                      {
                         char *help = NULL;
                         if (node->GetType() == MenuNode::ENTRY)
                            help = GetLongHelp(node->GetMenuEntry()->GetHelp2());
                         else
                            help = GetLongHelp(node->GetMenu()->GetHelp2());

                         if (help != NULL) {
                            eOSState newState = AddSubMenu(new cMenuText(tr("Help"), help, fontFix));
                            free(help);
                            return(newState);
                            }

                         }
                      break;
      case kYellow:
                      if (Interface->Confirm(cRecordControls::Active() ? trVDR("Recording - restart anyway?") : trVDR("Really restart?"))) {
#if APIVERSNUM >= 10501
                         ShutdownHandler.RequestEmergencyExit();
#else
                         cThread::EmergencyExit(true);
#endif
                         return osEnd;
                         }
                      return osContinue;
                      break;
      case kBlue:
                      if (Interface->Confirm(cRecordControls::Active() ? trVDR("Recording - restart anyway?") : tr("Really reboot?"))) {
#if APIVERSNUM >= 10501
                         ShutdownHandler.RequestEmergencyExit();
#else
                         cThread::EmergencyExit(true);
#endif
                         system(_config->GetBootLinux());
                         }
                      return osContinue;
                      break;

      case kNone:
                      break;

      default:
                      if (node!= NULL) {
                         if (node->GetType() == MenuNode::ENTRY) {
                            SetStatus(node->GetMenuEntry()->GetHelp1());
                            if (node->GetMenuEntry()->GetHelp2() != NULL)
                               SetHelp(tr("Help"), NULL, trVDR("Restart"), tr("Reboot"));
                            else
                               SetHelp(NULL, NULL, trVDR("Restart"), tr("Reboot"));
                            }
                         else {
                            SetStatus( node->GetMenu()->GetHelp1());
                            if (node->GetMenu()->GetHelp2() != NULL)
                               SetHelp(tr("Help"), NULL, trVDR("Restart"), tr("Reboot"));
                            else
                               SetHelp(NULL, NULL, trVDR("Restart"), tr("Reboot"));
                            }
                         }
                      else
                         SetStatus(NULL);
                      break;
      }
    return state;

}



/**
 * Key Processing for fetching Secret Code
 * @param Key 
 * @return 
 */
eOSState cSetupMenu::GetCodeProcessKey( eKeys Key )
{
  int num;
  eOSState state = cOsdMenu::ProcessKey(Key);

  switch (Key) {
    case k0 ... k9:
                num = Key - k0;
                sprintf(_childLockEntered, "%s%d", _childLockEntered, num);
                sprintf(_childLockEnteredHidden, "%s*", _childLockEnteredHidden);
                SetAskChildCode();
                if (strlen(_childLockEntered) == strlen(_config->GetChildLock())) {
                   if (strcmp(_childLockEntered,_config->GetChildLock()) == 0) {
                      _childLock=false;
                      Set();
                      }
                   else {
                      _childLockEntered[0] = '\0';
                      _childLockEnteredHidden[0] = '\0';
                      SetStatus(tr("Invalid Pin!"));
                      }
                   }
                else {
                   SetStatus(NULL);
                   }
                break;


    case kBack:
                return osBack;
                break;
    case kNone:
                break;
    default:
                SetStatus(NULL);
                break;
    }
  return state;
}

//
char * cSetupMenu::GetLongHelp( const char * help )
{
  char *helpfile = NULL;
  char *result = NULL;

  if (help != NULL) {
     asprintf(&helpfile, "%s/setup/help/%s", cPlugin::ConfigDirectory(), help);
     FILE *fp = fopen(helpfile, "r");
     if (fp != NULL) {
        fseek(fp, 0L, SEEK_END);

        long len = ftell(fp);
        fseek(fp, 0L, SEEK_SET);
        int byteRead = 0;
        if ((result = (char*) malloc(len+1)) == NULL)
           DEBUG3("%s: can not allocate buffer for Helpfile: %s\n", DBG_PREFIX, helpfile );
        else
        if ((byteRead = fread(result, 1, len, fp)) == -1)
           DEBUG3("%s: can not read file: Helpfile: %s\n", DBG_PREFIX, helpfile);
        else
           result[byteRead] = '\0';
        fclose(fp);
        }
     else
        DEBUG4("%s: can not open Helpfile: %s, %s\n", DBG_PREFIX, helpfile, strerror(errno));
     free(helpfile);
     }
  return(result);
}




