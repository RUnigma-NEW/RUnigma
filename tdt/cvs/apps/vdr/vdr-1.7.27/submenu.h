#ifdef USE_SETUP
/****************************************************************************
 * DESCRIPTION:
 *             Submenu
 *
 * $Id: vdr-1.3.44-Setup-0.3.0.diff,v 1.1 2006/03/04 09:58:47 ralf Exp $
 *
 * Contact:    ranga@teddycats.de
 *
 * Copyright (C) 2004, 2005 by Ralf Dotzert
 *
 * modified for the VDR Extensions Patch by zulu @vdr-portal
 *
 ****************************************************************************/

#ifndef SUBMENU_H
#define SUBMENU_H

#include "thread.h"
#include "tools.h"
#include "tinystr.h"

class cSubMenuNode;
class cSubMenuNodes;
class cSubMenu;


class cSubMenuNodes : public cList<cSubMenuNode> {};

// execute cmd thread
class cExecCmdThread : public cThread {
private:
  char *ExecCmd;
protected:
  virtual void Action(void) {
     if (system(ExecCmd) == 0)
        esyslog("%s - finished", ExecCmd);
     delete(this);
     };
public:
  cExecCmdThread(char *cmd) {
     asprintf(&ExecCmd, "%s", cmd);
     }
  cExecCmdThread(const char *cmd) {
     asprintf(&ExecCmd, "%s", cmd);
     }
  ~cExecCmdThread() {
     free(ExecCmd);
     };
  };

//################################################################################
//# SubMenuNode
//################################################################################
class cSubMenuNode : public cListObject {
public:
  enum Type { UNDEFINED, SYSTEM, COMMAND, THREAD, PLUGIN, MENU };
  cSubMenuNode(TiXmlElement * xml, int level, cSubMenuNodes *currentMenu, cSubMenuNodes *parentMenu);
  cSubMenuNode(cSubMenuNodes *currentMenu, cSubMenuNodes *parentMenu);
  ~cSubMenuNode();
  bool SaveXml(TiXmlElement * root);
  static cSubMenuNode::Type IsType(const char *name);
  void  SetType(const char *name);
  void  SetType(enum Type type);
  void  SetPlugin();
  cSubMenuNode::Type GetType();
  const char *GetTypeAsString();
  void SetCommand(const char *command);
  bool CommandConfirm();
  void SetCommandConfirm(int val);
  const char *GetCommand();
  void SetCustomTitle(const char *title);
  const char *GetCustomTitle();
  void SetName(const char *name);
  const char*GetName();
  int  GetLevel();
  void SetLevel(int level);
  int  GetPluginIndex();
  void SetPluginIndex(int index);
  void SetPluginMainMenuEntry(const char *mainMenuEntry);
  const char *GetPluginMainMenuEntry();
  cSubMenuNodes *GetParentMenu();
  void SetParentMenu(cSubMenuNodes *parent);
  cSubMenuNodes *GetCurrentMenu();
  void SetCurrentMenu(cSubMenuNodes *current);
  cSubMenuNodes *GetSubMenus();
  bool HasSubMenus();
  void Print(int index = 0);
private:
  Type _type;
  int _level;
  // Plugin Variables
  int _pluginIndex;
  const char *_pluginMainMenuEntry;
  // common
  const char *_name;
  const char *_command;
  bool _commandConfirm;
  const char *_title;
  cSubMenuNodes _subMenus;
  cSubMenuNodes *_parentMenu;
  cSubMenuNodes *_currentMenu;
  void init();
  };


//################################################################################
//# SubMenu Class
//################################################################################
class cSubMenu {
public:
  cSubMenu();
  ~cSubMenu();
  enum Where { BEFORE, BEHIND, INTO};
  bool LoadXml(char *fname);
  bool SaveXml(char *fname);
  bool SaveXml();
  cSubMenuNodes *GetMenuTree();
  bool Up(int *ParentIndex);
  bool Down(int index);
  int  GetNrOfNodes();
  cSubMenuNode* GetAbsNode(int index);
  cSubMenuNode* GetNode(int index);
  void PrintMenuTree();
  bool IsPluginInMenu(const char *name);
  void AddPlugin(const char *name);
  void CreateCommand(int index, const char *name, const char *execute, int confirm);
  void CreateThread(int index, const char *name, const char *execute, int confirm);
  const char *ExecuteCommand(const char *command);
  void MoveMenu(int index, int toindex, enum Where);
  void CreateMenu(int index, const char *menuTitle);
  void DeleteMenu(int index);
  char *GetMenuSuffix() { return _menuSuffix; }
  void SetMenuSuffix(char *suffix) { if (_menuSuffix) free(_menuSuffix); asprintf(&_menuSuffix, suffix); }
  bool isTopMenu() { return (_currentParentMenuTree == NULL); }
  const char *GetParentMenuTitel();
private:
  cSubMenuNodes _menuTree;
  cSubMenuNodes *_currentMenuTree;
  cSubMenuNodes *_currentParentMenuTree;
  char *_fname;
  char *_commandResult;
  int _nrNodes;
  cSubMenuNode **_nodeArray;
  char *_menuSuffix;
  int countNodes(cSubMenuNodes *tree);
  void tree2Array(cSubMenuNodes *tree, int &index);
  void addMissingPlugins();
  void reloadNodeArray();
  void removeUndefinedNodes();
  };

#endif //__SUBMENU_H
#endif /* SETUP */
