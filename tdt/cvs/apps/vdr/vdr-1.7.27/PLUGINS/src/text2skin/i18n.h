//								-*- c++ -*-

#ifndef VDR_TEXT2SKIN_I18N_H
#define VDR_TEXT2SKIN_I18N_H

#include "common.h"
#include "file.h"
#include <vdr/i18n.h>

class cText2SkinI18n {
private:
	std::string  mIdentity;

public:
	cText2SkinI18n(const char *Skin);
	std::string Translate(const std::string &Text) { return I18nTranslate(Text.c_str(), mIdentity.c_str()); }
};

#endif // VDR_TEXT2SKIN_I18N_H

