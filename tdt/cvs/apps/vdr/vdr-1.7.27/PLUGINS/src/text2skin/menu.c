//								-*- c++ -*-

#include "menu.h"
#include "bitmap.h"

// --- cText2SkinSetupPage ----------------------------------------------------

cText2SkinSetupPage::cText2SkinSetupPage(void):
	mData(Text2SkinSetup)
{
	Setup();
}

void cText2SkinSetupPage::Setup(void) {
	int current = Current();
	Clear();

	Add(new cMenuEditBoolItem(tr("Show scrollbar in the menus"), &mData.MenuScrollbar));
	Add(new cMenuEditBoolItem(tr("Scrolling behaviour"), &mData.MarqueeLeftRight, tr("to the left"), tr("left and right")));
	Add(new cMenuEditBoolItem(tr("Reset Marquee for new item"), &mData.MarqueeReset));
	Add(new cMenuEditBoolItem(tr("Show auxiliary infos of recordings"), &mData.ShowAux));
	if (mData.ShowAux)
		// TRANSLATORS: note the leading blanks!
		Add(new cMenuEditBoolItem(tr("   Extract known tags"), &mData.StripAux));
	Add(new cMenuEditBoolItem(tr("Use 'epgsearch' to check timer-conflicts"), &mData.CheckTimerConflict));
	Add(new cOsdItem(tr("Flush image cache"), osUser1));
	Add(new cMenuEditIntItem(tr("Max. image cache size"), &mData.MaxCacheFill, 1));

	SetCurrent(Get(current));
	Display();
}

cText2SkinSetupPage::~cText2SkinSetupPage() {
}

void cText2SkinSetupPage::Store(void) {
	SetupStore("MenuScrollbar", mData.MenuScrollbar);
	SetupStore("MarqueeLeftRight", mData.MarqueeLeftRight);
	SetupStore("MarqueeReset", mData.MarqueeReset);
	SetupStore("ShowAux", mData.ShowAux);
	SetupStore("StripAux", mData.StripAux);
	SetupStore("CheckTimerConflict", mData.CheckTimerConflict);
	SetupStore("MaxCacheFill", mData.MaxCacheFill);
	Text2SkinSetup = mData;
}

eOSState cText2SkinSetupPage::ProcessKey(eKeys Key) {
	int oldShowAux = mData.ShowAux;

	eOSState state = cMenuSetupPage::ProcessKey(Key);
	if (state == osUser1) {
		Skins.Message(mtInfo, tr("Flushing image cache..."));
		cText2SkinBitmap::FlushCache();
		Skins.Message(mtInfo, NULL);
		return osContinue;
	}

	if (mData.ShowAux != oldShowAux)
		Setup();

	return state;
}

