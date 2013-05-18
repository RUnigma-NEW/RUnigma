//								-*- c++ -*-

#ifndef VDR_TEXT2SKIN_XML_SKIN_H
#define VDR_TEXT2SKIN_XML_SKIN_H

#include "xml/display.h"
#include <vdr/osd.h>
#include <map>
#include <string>

// --- cxSkin -----------------------------------------------------------------

class cText2SkinI18n;
class cText2SkinTheme;

class cxVersion {
public:
	cxVersion(int ma = 0, int min = 0);
	bool Parse(const std::string &Text);
	int Major(void) const { return mMajor; }
	int Minor(void) const { return mMinor; }
	bool Require(int ma, int min) const {
		return mMajor > ma ? true : (mMajor == ma ? mMinor >= min : false);
	}
	bool Limit(int ma, int min) const {
		return mMajor < ma ? true : (mMajor == ma ? mMinor <= min : false);
	}
	bool operator==(const cxVersion &v) const {
		return mMajor == v.mMajor && mMinor == v.mMinor;
	}
	bool operator>=(const cxVersion &v) const {
		return Require(v.mMajor, v.mMinor);
	}
	bool operator>=(const char *c) const {
		cxVersion v;
		if (!v.Parse(c))
			return false;
		return Require(v.mMajor, v.mMinor);
	}
	bool operator<=(const cxVersion &v) const {
		return Limit(v.mMajor, v.mMinor);
	}
	bool operator<=(const char *c) const {
		cxVersion v;
		if (!v.Parse(c))
			return false;
		return Limit(v.mMajor, v.mMinor);
	}

private:
	int mMajor;
	int mMinor;
};

class cxSkin {
	friend bool xStartElem(const std::string &name, std::map<std::string,std::string> &attrs);
	friend bool xEndElem(const std::string &name);

	/* Skin Editor */
	friend class VSkinnerView;

public:
	enum eScreenBase {
		relative,
		absolute,
		abs1280x720,
		abs1920x1080,
#define __COUNT_BASE__ (abs1920x1080 + 1)
	};

private:
	eScreenBase      mBase;
	txPoint          mBaseOffset;
	txSize           mBaseSize;
	std::string      mName;
	std::string      mTitle;
	cxVersion        mVersion;

	cxDisplays       mDisplays;

	cText2SkinI18n  *mI18n; // TODO: should move here completely
	cText2SkinTheme *mTheme;

public:
	cxSkin(const std::string &Name, cText2SkinI18n *I18n, cText2SkinTheme *Theme);

	cxDisplay *Get(cxDisplay::eType Type);

	bool ParseBase(const std::string &Text);
	void SetBase(eScreenBase Base = (eScreenBase)-1);

	eScreenBase        Base(void)       const { return mBase; }
	const txPoint     &BaseOffset(void) const { return mBaseOffset; }
	const txSize      &BaseSize(void)   const { return mBaseSize; }
	const std::string &Name(void)       const { return mName; }
	const std::string &Title(void)      const { return mTitle; }
	const cxVersion   &Version(void)    const { return mVersion; }

	// functions for object classes to obtain dynamic item information
	std::string        Translate(const std::string &Text);
};

inline cxDisplay *cxSkin::Get(cxDisplay::eType Type) {
	if (mDisplays.find(Type) != mDisplays.end())
		return mDisplays[Type];
	return NULL;
}

#endif // VDR_TEXT2SKIN_XML_SKIN_H
