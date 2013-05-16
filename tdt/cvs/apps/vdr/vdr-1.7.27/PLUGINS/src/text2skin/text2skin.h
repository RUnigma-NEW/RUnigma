//								-*- c++ -*-

#ifndef VDR_TEXT2SKIN_H
#define VDR_TEXT2SKIN_H

#include "common.h"
#include <vdr/plugin.h>

class cText2SkinPlugin : public cPlugin {
private:
	static const char *VERSION;
	static const char *SKINVERSION;
	static const char *DESCRIPTION;

public:
	static const char *SkinVersion(void) { return SKINVERSION; }

  cText2SkinPlugin(void);
  virtual ~cText2SkinPlugin();
  virtual const char *Version(void) { return VERSION; }
  virtual const char *Description(void) { return tr(DESCRIPTION); }
  virtual const char **SVDRPHelpPages(void);
  virtual cString SVDRPCommand(const char *Command, const char *Option, int &ReplyCode);
  virtual bool Start(void);
  virtual cMenuSetupPage *SetupMenu(void);
  virtual bool SetupParse(const char *Name, const char *Value);
};

#endif // VDR_TEXT2SKIN_H
