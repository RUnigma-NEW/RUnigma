//								-*- c++ -*-

#ifndef VDR_TEXT2SKIN_LOADER_H
#define VDR_TEXT2SKIN_LOADER_H

#include "common.h"
#include <vdr/skins.h>

class cxSkin;
class cText2SkinI18n;
class cText2SkinTheme;

class cText2SkinLoader: public cSkin {
private:
	cxSkin          *mData;
	cText2SkinI18n  *mI18n;
	cText2SkinTheme *mTheme;
	std::string      mDescription;

public:
	static void Start(void);
	static void Load(const char *Skin);

	cText2SkinLoader(cxSkin *Data, cText2SkinI18n *I18n, cText2SkinTheme *Theme,
			const std::string &Skin, const std::string &Description);
	~cText2SkinLoader();

	virtual const char *Description(void) { return mDescription.c_str(); }
	virtual cSkinDisplayChannel *DisplayChannel(bool WithInfo);
	virtual cSkinDisplayMenu *DisplayMenu(void);
	virtual cSkinDisplayReplay *DisplayReplay(bool ModeOnly);
	virtual cSkinDisplayVolume *DisplayVolume(void);
	virtual cSkinDisplayTracks *DisplayTracks(const char *Title, int NumTracks,
	                                          const char * const *Tracks);
	virtual cSkinDisplayMessage *DisplayMessage(void);

	cxSkin          *Data(void) const { return mData; }
	cText2SkinI18n  *I18n(void) const { return mI18n; }
	cText2SkinTheme *Theme(void) const { return mTheme; }
};

#endif // VDR_TEXT2SKIN_LOADER_H
