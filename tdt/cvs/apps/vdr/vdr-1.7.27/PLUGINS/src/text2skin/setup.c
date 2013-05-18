//								-*- c++ -*-

#include "setup.h"

cText2SkinSetup Text2SkinSetup;

// --- cText2SkinSetup --------------------------------------------------------

cText2SkinSetup::cText2SkinSetup(void):
	MenuScrollbar(false),
	MarqueeLeftRight(true),
	MarqueeReset(false),
	ShowAux(true),
	StripAux(true),
	CheckTimerConflict(false),
	MaxCacheFill(25)
{
}

bool cText2SkinSetup::SetupParse(const char *Name, const char *Value) {
	if      (strcmp(Name, "MenuScrollbar") == 0) MenuScrollbar = atoi(Value);
	else if (strcmp(Name, "MarqueeLeftRight") == 0) MarqueeLeftRight = atoi(Value);
	else if (strcmp(Name, "MarqueeReset") == 0) MarqueeReset = atoi(Value);
	else if (strcmp(Name, "ShowAux") == 0) ShowAux = atoi(Value);
	else if (strcmp(Name, "StripAux") == 0) StripAux = atoi(Value);
	else if (strcmp(Name, "CheckTimerConflict") == 0) CheckTimerConflict = atoi(Value);
	else if (strcmp(Name, "MaxCacheFill") == 0) MaxCacheFill = atoi(Value);
	else return false;
	return true;
}

