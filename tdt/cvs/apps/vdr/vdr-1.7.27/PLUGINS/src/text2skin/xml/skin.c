//								-*- c++ -*-

#include "xml/skin.h"
#include "i18n.h"
#include <vdr/tools.h>
#include <vdr/config.h>

const std::string ScreenBases[] = { "relative", "absolute", "abs1280x720", "abs1920x1080" };

cxVersion::cxVersion(int ma, int min):
		mMajor(ma),
		mMinor(min)
{
}

bool cxVersion::Parse(const std::string &Text)
{
	int dot = Text.find(".");
	std::string ma(Text, 0, dot), min(Text, dot + 1);
	char *e = NULL;
	const char *t = NULL;
	long l = 0;

	t = ma.c_str();
	l = strtol(t, &e, 10);
	if (e == t || *e != '\0')
		return false;
	else
		mMajor = l;

	t = min.c_str();
	l = strtol(t, &e, 10);
	if (e == t || *e != '\0')
		return false;
	else
		mMinor = l;

	return true;
}

cxSkin::cxSkin(const std::string &Name, cText2SkinI18n *I18n, cText2SkinTheme *Theme):
		mName(Name),
		mI18n(I18n),
		mTheme(Theme)
{
}

void cxSkin::SetBase(eScreenBase Base)
{
	if (Base != (eScreenBase)-1)
		mBase = Base;

	switch (mBase) {
	case relative:
		mBaseOffset = txPoint(Setup.OSDLeft, Setup.OSDTop);
		mBaseSize   = txSize(Setup.OSDWidth, Setup.OSDHeight);
		break;

	case absolute:
		mBaseOffset = txPoint(0, 0);
		mBaseSize   = txSize(720, 576); //XXX
		break;

	case abs1280x720:
		mBaseOffset = txPoint(0, 0);
		mBaseSize   = txSize(1280, 720); //XXX
		break;

	case abs1920x1080:
		mBaseOffset = txPoint(0, 0);
		mBaseSize   = txSize(1920, 1080); //XXX
		break;

	default:
		break;
	}
}

bool cxSkin::ParseBase(const std::string &Text)
{
	int i;
	for (i = 0; i < (int)__COUNT_BASE__; ++i) {
		if (ScreenBases[i] == Text)
			break;
	}
	if (i < (int)__COUNT_BASE__) {
		SetBase((eScreenBase)i);
		return true;
	}
	return false;
}

std::string cxSkin::Translate(const std::string &Text)
{
	if (mI18n != NULL)
		return mI18n->Translate(Text);
	return Text;
}
