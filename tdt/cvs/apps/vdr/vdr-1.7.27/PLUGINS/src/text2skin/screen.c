//								-*- c++ -*-

#include "screen.h"

cText2SkinScreen::cText2SkinScreen(bool OffScreen):
	mOsd(OffScreen ? NULL : cOsdProvider::NewOsd(0, 0)),
	mScreen(OffScreen ? new cBitmap(720, 576, 8) : NULL),
	mOffScreen(OffScreen)
{
}

cText2SkinScreen::~cText2SkinScreen()
{
	delete mScreen;
	delete mOsd;
}

eOsdError cText2SkinScreen::SetAreas(const tArea *Areas, int NumAreas)
{
	if (!mOffScreen) {
		eOsdError result = mOsd->CanHandleAreas(Areas, NumAreas);
		if (result == oeOk)
			mOsd->SetAreas(Areas, NumAreas);
		else
			return result;
	}

	mNumRegions = NumAreas;
#ifndef DIRECTBLIT
	for (int i = 0; i < mNumRegions; ++i)
		mRegions[i] = new cBitmap(Areas[i].Width(), Areas[i].Height(), Areas[i].bpp, Areas[i].x1, Areas[i].y1);
#endif
	Clear();

	return oeOk;
}

void cText2SkinScreen::Clear(void)
{
	for (int i = 0; i < mNumRegions; ++i) {
#ifndef DIRECTBLIT
		mRegions[i]->Reset();
		mRegions[i]->Clean();
		mRegions[i]->DrawRectangle(mRegions[i]->X0(), mRegions[i]->Y0(), mRegions[i]->X0() + mRegions[i]->Width() - 1, mRegions[i]->Y0() + mRegions[i]->Height() - 1, clrTransparent);
#else
		cBitmap *b = mOsd->GetBitmap(i);
		if (b) {
			b->Reset();
			b->Clean();
			b->DrawRectangle(b->X0(), b->Y0(), b->X0() + b->Width() - 1, b->Y0() + b->Height() - 1, clrTransparent);
		}
#endif
	}
}

void cText2SkinScreen::DrawBitmap(int x, int y, const cBitmap &Bitmap, const tColor *ColorMask)
{
#ifndef DIRECTBLIT
	for (int i = 0; i < mNumRegions; ++i)
		DrawBitmapOverlay(*mRegions[i], x, y, (cBitmap&)Bitmap, ColorMask);
		//mRegions[i]->DrawBitmap(x, y, Bitmap);
#else
	// mOsd->DrawBitmap(x, y, Bitmap, ColorFg, ColorBg);
	cBitmap *bm = NULL;
	for (int i = 0; (bm = mOsd->GetBitmap(i)) != NULL; ++i)
		DrawBitmapOverlay(*bm, x, y, (cBitmap&)Bitmap, ColorMask);
#endif
}

void cText2SkinScreen::DrawRectangle(int x1, int y1, int x2, int y2, tColor Color)
{
#ifndef DIRECTBLIT
	for (int i = 0; i < mNumRegions; ++i)
		mRegions[i]->DrawRectangle(x1, y1, x2, y2, Color);
#else
	mOsd->DrawRectangle(x1, y1, x2, y2, Color);
#endif
}

void cText2SkinScreen::DrawText(int x, int y, const char *s, tColor ColorFg, tColor ColorBg,
                                const cFont *Font, int Width, int Height, int Alignment)
{
#ifndef DIRECTBLIT
	for (int i = 0; i < mNumRegions; ++i)
		mRegions[i]->DrawText(x, y, s, ColorFg, ColorBg, Font, Width, Height, Alignment);
#else
	mOsd->DrawText(x, y, s, ColorFg, ColorBg, Font, Width, Height, Alignment);
#endif
}

void cText2SkinScreen::DrawEllipse(int x1, int y1, int x2, int y2, tColor Color, int Quadrants)
{
#ifndef DIRECTBLIT
	for (int i = 0; i < mNumRegions; ++i)
		mRegions[i]->DrawEllipse(x1, y1, x2, y2, Color, Quadrants);
#else
	mOsd->DrawEllipse(x1, y1, x2, y2, Color, Quadrants);
#endif
}

void cText2SkinScreen::DrawSlope(int x1, int y1, int x2, int y2, tColor Color, int Type)
{
#ifndef DIRECTBLIT
	for (int i = 0; i < mNumRegions; ++i)
		mRegions[i]->DrawSlope(x1, y1, x2, y2, Color, Type);
#else
	mOsd->DrawSlope(x1, y1, x2, y2, Color, Type);
#endif
}

void cText2SkinScreen::Flush(void)
{
	for (int i = 0; i < mNumRegions; ++i) {
		if (mOffScreen)
			mScreen->DrawBitmap(mRegions[i]->X0(), mRegions[i]->Y0(), *mRegions[i]);
#ifndef DIRECTBLIT
		else
			mOsd->DrawBitmap(mRegions[i]->X0(), mRegions[i]->Y0(), *mRegions[i]);
#endif
	}
#ifdef BENCH
	int x1 = 0, y1 = 0, x2 = 0, y2 = 0;
	cBitmap *bm;
	for (int j = 0; (bm = mOsd->GetBitmap(j)) != NULL; j++)
		if (bm->Dirty(x1, y1, x2, y2))
			fprintf(stderr, "Flush dirty screen area %2i: x1=%3i x2=%3i y1=%3i y2=%3i\n",
				j, x1, x2, y1, y2);
#endif
	if (!mOffScreen)
		mOsd->Flush();
}

void cText2SkinScreen::DrawBitmapOverlay(cBitmap &Dest, int x, int y, cBitmap &Bitmap,
                                         const tColor *ColorMask)
{
	const tIndex *bitmap = Dest.Data(0, 0);
	if (bitmap && Bitmap.Data(0, 0) && Dest.Intersects(x, y, x + Bitmap.Width() - 1, y + Bitmap.Height() - 1)) {
		if (Dest.Covers(x, y, x + Bitmap.Width() - 1, y + Bitmap.Height() - 1))
			Dest.Reset();
		x -= Dest.X0();
		y -= Dest.Y0();
		tIndex Indexes[MAXNUMCOLORS];
		Dest.Take(Bitmap, &Indexes);
		tIndex Mask, *indexMask = NULL;
		if (ColorMask != NULL) {
			Mask = Dest.Index(*ColorMask);
			indexMask = &Mask;
		}
		for (int ix = 0; ix < Bitmap.Width(); ix++) {
			for (int iy = 0; iy < Bitmap.Height(); iy++) {
				if (indexMask == NULL || *indexMask != Indexes[int(*Bitmap.Data(ix, iy))])
					Dest.SetIndex(x + ix, y + iy, Indexes[int(*Bitmap.Data(ix, iy))]);
			}
		}
	}
}
