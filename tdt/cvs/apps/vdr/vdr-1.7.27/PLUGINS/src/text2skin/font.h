//								-*- c++ -*-

#ifndef VDR_TEXT2SKIN_FONT_H
#define VDR_TEXT2SKIN_FONT_H

#include "common.h"
#include <map>
#include <string>
#include <vdr/font.h>

using std::map;
using std::string;


class cText2SkinFontCache
{
private:
	typedef map<string,cFont*> cache_map;

public:
	cText2SkinFontCache();
	~cText2SkinFontCache();

	bool Load(string Name, string CacheName, int Size);
	const cFont* GetFont(string CacheName);
	void Clear();

private:
	cache_map 		_cache;
};

class cText2SkinFont {
private:
	static cText2SkinFontCache mFontCache;
	// disallow direct construction
	cText2SkinFont(void);
	virtual ~cText2SkinFont();

public:
	static const cFont *Load(const string &Name, int Size);
};

#endif // VDR_TEXT2SKIN_FONT_H
