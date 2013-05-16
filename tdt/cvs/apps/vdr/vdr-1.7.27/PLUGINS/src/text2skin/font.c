//								-*- c++ -*-

#include "font.h"
#include "render.h"
#include <vdr/tools.h>


cText2SkinFontCache::cText2SkinFontCache(void)
{
}

cText2SkinFontCache::~cText2SkinFontCache()
{
	Clear();
}

bool cText2SkinFontCache::Load(string Name, string CacheName, int Size)
{
	if ( _cache.find(CacheName) != _cache.end() )
		return true;
	cFont* newFont = cFont::CreateFont(Name.c_str(), Size);
	if ( newFont == NULL )
		return false;
	_cache[CacheName] = newFont;
	return true;
}

const cFont* cText2SkinFontCache::GetFont(string CacheName)
{
	if (CacheName == "Sml") return cFont::GetFont(fontSml);
	else if (CacheName == "Fix") return cFont::GetFont(fontFix);
	else if ( _cache.find(CacheName) != _cache.end() )
	{
		return _cache[CacheName];
	}
	return cFont::GetFont(fontOsd);
}

void cText2SkinFontCache::Clear()
{
	cache_map::iterator it = _cache.begin();
	for (; it != _cache.end(); ++it)
		delete((*it).second);
	_cache.clear();
}


cText2SkinFontCache cText2SkinFont::mFontCache;

cText2SkinFont::cText2SkinFont(void)
{
}

cText2SkinFont::~cText2SkinFont()
{
}

const cFont *cText2SkinFont::Load(const string &Name, int Size)
{
	if (Name == "Osd")
		return cFont::GetFont(fontOsd);
	else if (Name == "Fix")
		return cFont::GetFont(fontFix);
	else if (Name == "Sml")
		return cFont::GetFont(fontSml);

	const cFont *res = NULL;
	string cachename = string(cString::sprintf("%s_%d", Name.c_str(), Size));
	if (mFontCache.Load(Name, cachename, Size))
		res = mFontCache.GetFont(cachename);
	else
		esyslog("ERROR: Text2Skin: Couldn't load font %s@%d", Name.c_str(), Size);
	return res;
}
