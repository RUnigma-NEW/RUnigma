//								-*- c++ -*-

#ifndef VDR_TEXT2SKIN_SCREEN_H
#define VDR_TEXT2SKIN_SCREEN_H

#include "common.h"
#include <vdr/osd.h>

// #undef DIRECTBLIT
#define DIRECTBLIT

class cText2SkinScreen {
	/* Skin Editor */
	friend class VSkinnerScreen;

private:
	cOsd    *mOsd;
	cBitmap *mScreen;
	cBitmap *mRegions[MAXOSDAREAS];
	bool     mOffScreen;
	int      mNumRegions;

protected:
	static void DrawBitmapOverlay(cBitmap &Dest, int x, int y, cBitmap &Bitmap,
	                              const tColor *ColorMask = NULL);

public:
	cText2SkinScreen(bool OffScreen = false);
	~cText2SkinScreen();

	eOsdError SetAreas(const tArea *Areas, int NumAreas);

	void Clear(void);
	void DrawBitmap(int x, int y, const cBitmap &Bitmap, const tColor *ColorMask = NULL);
	void DrawRectangle(int x1, int y1, int x2, int y2, tColor Color);
	void DrawText(int x, int y, const char *s, tColor ColorFg, tColor ColorBg, const cFont *Font,
	              int Width = 0, int Height = 0, int Alignment = taDefault);
	void DrawEllipse(int x1, int y1, int x2, int y2, tColor Color, int Quadrants = 0);
	void DrawSlope(int x1, int y1, int x2, int y2, tColor Color, int Type);

	void Flush(void);

	bool IsOpen(void) const { return mOsd != NULL; }
};

#endif // VDR_TEXT2SKIN_SCREEN_H
