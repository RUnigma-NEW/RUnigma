/*
 * dvbufs9xx.c: A plugin for the Video Disk Recorder
 *
 * See the README file for copyright information and how to reach the author.
 *
 * $Id: dvbufs9xx.c $
 */

#include <vdr/plugin.h>
#include "dvbhddevice.h"
#include "setup.h"

static const char *VERSION        = "0.2.1";
static const char *DESCRIPTION    = "Duckbox DVB device";

class cPluginDvbDevice : public cPlugin {
private:
  cDvbHdDeviceProbe *probe;
public:
  cPluginDvbDevice(void);
  virtual ~cPluginDvbDevice();
  virtual const char *Version(void) { return VERSION; }
  virtual const char *Description(void) { return DESCRIPTION; }
  virtual cMenuSetupPage *SetupMenu(void);
  virtual bool SetupParse(const char *Name, const char *Value);
  };

cPluginDvbDevice::cPluginDvbDevice(void)
{
  probe = new cDvbHdDeviceProbe;
}

cPluginDvbDevice::~cPluginDvbDevice()
{
  delete probe;
}

cMenuSetupPage *cPluginDvbDevice::SetupMenu(void)
{
  return new cDvbSetupPage(cDvbHdDevice::GetDvbCmdHandler());
}

bool cPluginDvbDevice::SetupParse(const char *Name, const char *Value)
{
  return gDvbSetup.SetupParse(Name, Value);
}

VDRPLUGINCREATOR(cPluginDvbDevice); // Don't touch this!
