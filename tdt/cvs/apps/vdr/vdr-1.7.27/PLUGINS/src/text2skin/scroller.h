//								-*- c++ -*-

#ifndef VDR_TEXT2SKIN_SCROLLER_H
#define VDR_TEXT2SKIN_SCROLLER_H

#include "common.h"
#include <vdr/font.h>

class cText2SkinScreen;

class cText2SkinScroller {
private:
	cText2SkinScreen *mScreen;
	const cFont      *mFont;
	int               mLeft;
	int               mTop;
	int               mWidth;
	int               mHeight;
	int               mOffset;
	int               mShown;
	tColor            mColorFg;
	tColor            mColorBg;
	cTextWrapper      mWrapper;

public:
	cText2SkinScroller(cText2SkinScreen *Screen, int Left, int Top, int Width, int Height,
	                   const char *Text, const cFont *Font, tColor ColorFg, tColor ColorBg);

	void Set(cText2SkinScreen *Screen, int Left, int Top, int Width, int Height, const char *Text,
	         const cFont *Font, tColor ColorFg, tColor ColorBg);
	void DrawText(void);
	void Reset(void);
	int Left(void) { return mLeft; }
	int Top(void) { return mTop; }
	int Width(void) { return mWidth; }
	int Height(void) { return mHeight; }
	int Total(void) { return mWrapper.Lines(); }
	int Offset(void) { return mOffset; }
	int Shown(void) { return mShown; }
	bool CanScrollUp(void) { return mOffset > 0; }
	bool CanScrollDown(void) { return mOffset + mShown < Total(); }
	bool CanScroll(void) { return CanScrollUp() || CanScrollDown(); }
	void Scroll(bool Up, bool Page);
};

#endif // VDR_TEXT2SKIN_SCROLLER_H
