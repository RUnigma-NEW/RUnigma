//								-*- c++ -*-

#ifndef VDR_TEXT2SKIN_THEME_H
#define VDR_TEXT2SKIN_THEME_H

#include "common.h"
#include "file.h"
#include <map>
#include <vdr/themes.h>

class cText2SkinTheme: public cText2SkinFile {
private:
	typedef std::map<std::string,int> tThemeMap;

	cTheme    mTheme;
	tThemeMap mMap;

protected:
	bool Parse(const char *Text);

public:
	cText2SkinTheme(const char *Skin);
	virtual ~cText2SkinTheme();

	cTheme *Theme(void) { return &mTheme; }
	tColor Color(const std::string &Name);
};

inline tColor cText2SkinTheme::Color(const std::string &Name) {
	tThemeMap::iterator it = mMap.find(Name);
	if (it != mMap.end())
		return mTheme.Color((*it).second);
	else
		return 0x00000000;
}

#endif // VDR_TEXT2SKIN_THEME_H
