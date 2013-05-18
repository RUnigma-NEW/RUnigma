//								-*- c++ -*-

#ifndef VDR_TEXT2SKIN_DISPLAY_H
#define VDR_TEXT2SKIN_DISPLAY_H

#include "xml/object.h"
#include <string>
#include <map>

class cxSkin;

class cxDisplay {
	friend bool xStartElem(const std::string &name, std::map<std::string,std::string> &attrs);
	friend bool xEndElem(const std::string &name);

public:
	enum eType {
		channelInfo,
		channelSmall,
		volume,
		audioTracks,
		message,
		replayInfo,
		replaySmall,
		menu,
#define __COUNT_DISPLAY__ (menu + 1)
	};

private:
	cxSkin   *mSkin;
	eType     mType;
	txWindow  mWindows[MAXOSDAREAS];
	int       mNumWindows;
	int       mNumMarquees;
	cxObjects mObjects;
	cxRefresh mRefreshDefault;

public:
	cxDisplay(cxSkin *Parent);

	static const std::string &GetType(eType Type);
	bool ParseType(const std::string &Text);

	eType            Type(void)       const { return mType; }
	const txWindow  *Windows(void)    const { return mWindows; }
	int              NumWindows(void) const { return mNumWindows; }
	cxSkin          *Skin(void)       const { return mSkin; }

	uint             Objects(void)    const { return mObjects.size(); }
	cxObject        *GetObject(int n) const { return mObjects[n]; }
};

class cxDisplays: public std::map<cxDisplay::eType,cxDisplay*> {
public:
	cxDisplays(void);
	~cxDisplays();
};

#endif // VDR_TEXT2SKIN_DISPLAY_H
