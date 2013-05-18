/*
 * dvbufs9xx.c: A plugin for the Video Disk Recorder
 *
 * See the README file for copyright information and how to reach the author.
 *
 * $Id: dvbufs9xx.c $
 */

#include <vdr/plugin.h>
#include "dvbhddevice.h"

static const char *VERSION        = "0.0.3pre";
static const char *DESCRIPTION    = "Kathrein UFS9xx DVB device";

class cPluginDvbufs9xx : public cPlugin {
private:
  cDvbHdDeviceProbe *probe;
public:
  cPluginDvbufs9xx(void);
  virtual ~cPluginDvbufs9xx();
  virtual const char *Version(void) { return VERSION; }
  virtual const char *Description(void) { return DESCRIPTION; }
  };

cPluginDvbufs9xx::cPluginDvbufs9xx(void)
{
  probe = new cDvbHdDeviceProbe;
}

cPluginDvbufs9xx::~cPluginDvbufs9xx()
{
  delete probe;
}

VDRPLUGINCREATOR(cPluginDvbufs9xx); // Don't touch this!
