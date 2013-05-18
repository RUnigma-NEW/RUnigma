//								-*- c++ -*-

#include "xml/display.h"

static const std::string DisplayNames[] =
	{ "channelInfo", "channelSmall", "volume", "audioTracks", "message", "replayInfo",
      "replaySmall", "menu" };

cxDisplay::cxDisplay(cxSkin *parent):
		mSkin(parent),
		mType((eType)__COUNT_DISPLAY__),
		mNumWindows(0),
		mRefreshDefault(NULL)
{
}

bool cxDisplay::ParseType(const std::string &Text)
{
	for (int i = 0; i < (int)__COUNT_DISPLAY__; ++i) {
		if (DisplayNames[i].length() > 0 && DisplayNames[i] == Text) {
			mType = (eType)i;
			return true;
		}
	}
	return false;
}

const std::string &cxDisplay::GetType(eType Type)
{
	return DisplayNames[Type];
}

cxDisplays::cxDisplays(void)
{
}

cxDisplays::~cxDisplays()
{
	iterator it = begin();
	while (it != end())
		(delete (*it).second, ++it);
}
