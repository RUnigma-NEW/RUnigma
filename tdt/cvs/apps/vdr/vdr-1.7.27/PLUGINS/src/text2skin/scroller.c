//								-*- c++ -*-

#include "scroller.h"
#include "screen.h"
#include <vdr/tools.h>

cText2SkinScroller::cText2SkinScroller(cText2SkinScreen *Screen, int Left, int Top, int Width,
                                       int Height, const char *Text, const cFont *Font,
                                       tColor ColorFg, tColor ColorBg)
{
  Set(Screen, Left, Top, Width, Height, Text, Font, ColorFg, ColorBg);
}

void cText2SkinScroller::Set(cText2SkinScreen *Screen, int Left, int Top, int Width, int Height, const char *Text, const cFont *Font, tColor ColorFg, tColor ColorBg)
{
  mScreen  = Screen;
  mLeft    = Left;
  mTop     = Top;
  mWidth   = Width;
  mHeight  = Height;
  mFont    = Font;
  mColorFg = ColorFg;
  mColorBg = ColorBg;
  mOffset  = 0;
	// sets mHeight to the actually used mHeight, which may be less than Height
  mWrapper.Set(Text, Font, Width);
  mShown   = min(Total(), mHeight / mFont->Height());
  mHeight  = mShown * mFont->Height();
  DrawText();
}

void cText2SkinScroller::Reset(void) {
  mScreen = NULL; // just makes sure it won't draw anything
}

void cText2SkinScroller::DrawText(void) {
  if (mScreen) {
    for (int i = 0; i < mShown; i++)
      mScreen->DrawText(mLeft, mTop + i * mFont->Height(), mWrapper.GetLine(mOffset + i), mColorFg, mColorBg, mFont, mWidth);
  }
}

void cText2SkinScroller::Scroll(bool Up, bool Page)
{
  if (Up) {
    if (CanScrollUp()) {
      mOffset -= Page ? mShown : 1;
      if (mOffset < 0)
        mOffset = 0;
    }
  } else {
    if (CanScrollDown()) {
      mOffset += Page ? mShown : 1;
      if (mOffset + mShown > Total())
        mOffset = Total() - mShown;
    }
  }
}
