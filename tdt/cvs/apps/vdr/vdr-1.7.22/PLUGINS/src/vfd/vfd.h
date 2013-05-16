/*
 * 
 */

#ifndef VDR_VFD_H
#define VDR_VFD_H

#include <vdr/plugin.h>
	
class cVFDStatus;
class cSysTime;

class cPluginVFD : public cPlugin {
private:
	static const char *VERSION;
	static const char *DESCRIPTION;

	cVFDStatus *m_Status;
	cSysTime *m_SysTime;

public:
  cPluginVFD(void);
  virtual ~cPluginVFD();
  virtual const char *Version(void) { return VERSION; }
  virtual const char *Description(void) { return DESCRIPTION; }
  virtual bool Initialize(void);
  virtual bool Start(void);
  virtual void Housekeeping(void);
  virtual cMenuSetupPage *SetupMenu(void);
  virtual bool SetupParse(const char *Name, const char *Value);
};

#endif // VDR_VFD_H
