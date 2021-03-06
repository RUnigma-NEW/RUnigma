/****************************************************************************
 * DESCRIPTION: 
 *             Setup a Plugin vor VDR
 *
 * $Id: setup.cpp,v 1.18 2006/03/06 19:16:41 ralf Exp $
 *
 * Contact:    ranga@vdrtools.de
 *
 * Copyright (C) 2004 by Ralf Dotzert 
 ****************************************************************************/

#include <vdr/plugin.h>
#include "setupmenu.h"
#include "setupsetup.h"
#include "i18n.h"

static const char *VERSION        = "0.3.1-zulu-edition";
static const char *DESCRIPTION    = trNOOP("VDR-Setup Extension");
static const char *MAINMENUENTRY  = trNOOP("Setup");

//holds setup configuration
cSetupSetup  setupSetup;

class cPluginSetup : public cPlugin {
private:
  // Add any member variables or functions you may need here.
public:
  cPluginSetup(void);
  virtual ~cPluginSetup();
  virtual const char *Version(void) { return VERSION; }
  virtual const char *Description(void) { return tr(DESCRIPTION); }
  virtual const char *CommandLineHelp(void);
  virtual bool ProcessArgs(int argc, char *argv[]);
  virtual bool Initialize(void);
  virtual bool Start(void);
  virtual void Housekeeping(void);
  virtual const char *MainMenuEntry(void) { return setupSetup.DirectMenu ? tr("Menu Edit") : tr(MAINMENUENTRY); }
  virtual cOsdObject *MainMenuAction(void);
  virtual cMenuSetupPage *SetupMenu(void);
  virtual bool SetupParse(const char *Name, const char *Value);
  };

cPluginSetup::cPluginSetup(void)
{
  // Initialize any member variables here.
  // DON'T DO ANYTHING ELSE THAT MAY HAVE SIDE EFFECTS, REQUIRE GLOBAL
  // VDR OBJECTS TO EXIST OR PRODUCE ANY OUTPUT!
}

cPluginSetup::~cPluginSetup()
{
  // Clean up after yourself!
 
}

const char *cPluginSetup::CommandLineHelp(void)
{
  // Return a string that describes all known command line options.
  return NULL;
}

bool cPluginSetup::ProcessArgs(int argc, char *argv[])
{
  // Implement command line argument processing here if applicable.
  return true;
}

bool cPluginSetup::Initialize(void)
{
  // Initialize any background activities the plugin shall perform.
#if VDRVERSNUM < 10507
  RegisterI18n(Phrases);
#endif
  return true;
}

bool cPluginSetup::Start(void)
{
  // Start any background activities the plugin shall perform.
  return true;
}

void cPluginSetup::Housekeeping(void)
{
  // Perform any cleanup or other regular tasks.
}

cOsdObject *cPluginSetup::MainMenuAction(void)
{
  // Perform the action when selected from the main VDR menu.
  if (setupSetup.DirectMenu)
     return new cSetupVdrMenu(tr("Menu Edit"));
  return (new cSetupMenu());
}

cMenuSetupPage *cPluginSetup::SetupMenu(void)
{
  // Return a setup menu in case the plugin supports one.
  return new cSetupSetupPage;

}

bool cPluginSetup::SetupParse(const char *Name, const char *Value)
{
  // Parse your own setup parameters and store their values.
  return setupSetup.SetupParse(Name, Value);
}



VDRPLUGINCREATOR(cPluginSetup); // Don't touch this!
