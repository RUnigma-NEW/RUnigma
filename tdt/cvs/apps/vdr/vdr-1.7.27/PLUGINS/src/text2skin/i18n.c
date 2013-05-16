//								-*- c++ -*-

#include "i18n.h"
#include <vdr/config.h>

cText2SkinI18n::cText2SkinI18n(const char *Skin) :
	mIdentity(std::string("vdr-"PLUGIN_NAME_I18N"-") + Skin)
{
	I18nRegister(mIdentity.substr(mIdentity.find('-') + 1).c_str());
}
