//								-*- c++ -*-

#ifndef VDR_TEXT2SKIN_SETUP_H
#define VDR_TEXT2SKIN_SETUP_H

#include "common.h"

class cText2SkinSetup {
public:
	cText2SkinSetup(void);

	bool SetupParse(const char *Name, const char *Value);

	int MenuScrollbar;
	int MarqueeLeftRight;
	int MarqueeReset;
	int ShowAux;
	int StripAux;
	int CheckTimerConflict;
	int MaxCacheFill;
};

extern cText2SkinSetup Text2SkinSetup;

#endif // VDR_TEXT2SKIN_SETUP_H
