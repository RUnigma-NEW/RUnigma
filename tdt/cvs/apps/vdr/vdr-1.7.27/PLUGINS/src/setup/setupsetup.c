/****************************************************************************
 * DESCRIPTION:
 *             Setup Dialog
 *
 * $Id: setupsetup.cpp,v 1.2 2005/10/12 13:44:14 ralf Exp $
 *
 * Contact:    ranga@vdrtools.de
 *
 * Copyright (C) 2005 by Ralf Dotzert
 ****************************************************************************/


#include <vdr/menu.h>
#include <vdr/submenu.h>
#include <string.h>

#include "setupsetup.h"
#include "debug.h"

//*****************************************************************************
// Setup Configuration
//*****************************************************************************

cSetupSetup::cSetupSetup()
{
  // Set Default Values
  strcpy(_menuSuffix, " ...");
  strcpy(_entryPrefix, "-");
  DirectMenu = 0;
  ReturnValue = 1;
}

bool cSetupSetup::SetupParse( const char *Name, const char *Value )
{
  if      (!strcasecmp(Name, "DirectMenu"))  DirectMenu = atoi(Value);
  else if (!strcasecmp(Name, "MenuSuffix"))  snprintf(_menuSuffix, sizeof(_menuSuffix), Value);
  else if (!strcasecmp(Name, "EntryPrefix")) snprintf(_entryPrefix, sizeof(_entryPrefix), Value);
  else if (!strcasecmp(Name, "ReturnValue")) ReturnValue = atoi(Value);
  else return false;

  return true;
}

//*****************************************************************************
// Setup Page
//*****************************************************************************
cSetupSetupPage::cSetupSetupPage( )
{
  ReturnValues[0]="true/false";
  ReturnValues[1]="on/off";
  ReturnValues[2]="yes/no";
  ReturnValues[3]="1/0";

  Add(new cMenuEditBoolItem(tr("setupSetup$Main menu entry"), &setupSetup.DirectMenu, tr("Setup"), tr("Menu Edit")));
  Add(new cMenuEditStrItem(tr("setupSetup$Menu suffix"),       setupSetup._menuSuffix, sizeof(setupSetup._menuSuffix),  trVDR(FileNameChars)));
  Add(new cMenuEditStrItem(tr("setupSetup$Entry prefix"),      setupSetup._entryPrefix, sizeof(setupSetup._entryPrefix), trVDR(FileNameChars)));
  Add(new cMenuEditStraItem(tr("setupSetup$Return value"),    &setupSetup.ReturnValue, 4, ReturnValues));
}


void cSetupSetupPage::Store( void )
{
  SetupStore("DirectMenu",      setupSetup.DirectMenu);
  SetupStore("MenuSuffix",      setupSetup._menuSuffix);
  SetupStore("EntryPrefix",     setupSetup._entryPrefix);
  SetupStore("ReturnValue",     setupSetup.ReturnValue);
}

eOSState cSetupSetupPage::ProcessKey( eKeys Key )
{
  cSubMenu vdrSubMenu;
  char *menuXML = NULL;
  eOSState state = cOsdMenu::ProcessKey(Key);
  if (state == osUnknown) {
     switch (Key) {
        case kOk: // Load Menu Configuration
#if VDRVERSNUM < 10507
                 asprintf(&menuXML, "%s/setup/vdr-menu.%i.xml", cPlugin::ConfigDirectory(), Setup.OSDLanguage);
#else
                 asprintf(&menuXML, "%s/setup/vdr-menu.%s.xml", cPlugin::ConfigDirectory(), Setup.OSDLanguage);
#endif
                 if (access(menuXML, 06) == -1)
                    asprintf(&menuXML, "%s/setup/vdr-menu.xml", cPlugin::ConfigDirectory());
                 if (vdrSubMenu.LoadXml(menuXML)) {
                    isyslog("setup: saved setup to %s", menuXML);
                    vdrSubMenu.SetMenuSuffix(setupSetup._menuSuffix);
                    vdrSubMenu.SaveXml();
                    }
                 free(menuXML);
                 Store();
                 return osBack;
        default: break;
        }
     state = osContinue;
     }
  return state;
}


