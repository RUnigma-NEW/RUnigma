//								-*- c++ -*-

#ifndef VDR_TEXT2SKIN_SKIN_H
#define VDR_TEXT2SKIN_SKIN_H

#include "common.h"
#include "render.h"
#include "xml/string.h"
#include <vdr/skins.h>
#include <string>

class cText2SkinData;
class cText2SkinI18n;
class cText2SkinLoader;

class cText2SkinDisplayChannel: public cSkinDisplayChannel, public cText2SkinRender {
private:
	cSkinDisplayChannel *mFallbackDisplay;

	const cChannel      *mChannel;
	int                  mNumber;
	const cEvent        *mPresent;
	const cEvent        *mFollowing;
	eMessageType         mType;
	std::string          mText;
	std::string          mButtonRed;
	std::string          mButtonGreen;
	std::string          mButtonYellow;
	std::string          mButtonBlue;

protected:
	virtual cxType GetTokenData(const txToken &Token);

public:
	cText2SkinDisplayChannel(cText2SkinLoader *Loader, bool WithInfo);
	virtual ~cText2SkinDisplayChannel();

	virtual void SetChannel(const cChannel *Channel, int Number);
	virtual void SetEvents(const cEvent *Present, const cEvent *Following);
	virtual void SetMessage(eMessageType Type, const char *Text);
	virtual void SetButtons(const char *Red, const char *Green, const char *Yellow, const char *Blue);

	virtual void Flush(void);
};

inline void cText2SkinDisplayChannel::Flush(void) {
	if (mFallbackDisplay != NULL)
		mFallbackDisplay->Flush();
	else
		cText2SkinRender::Flush();
}

class cText2SkinDisplayVolume: public cSkinDisplayVolume, public cText2SkinRender {
private:
	int  mCurrent;
	int  mTotal;
	bool mMute;

protected:
	virtual cxType GetTokenData(const txToken &Token);

public:
	cText2SkinDisplayVolume(cText2SkinLoader *Loader);
	virtual ~cText2SkinDisplayVolume();
	virtual void SetVolume(int Current, int Total, bool Mute);

	virtual void Flush(void) { cText2SkinRender::Flush(); }
};

class cText2SkinDisplayReplay: public cSkinDisplayReplay, public cText2SkinRender {
private:
	std::string   mTitle;
	bool          mStateInfo;
	bool          mPlay;
	bool          mForward;
	int           mSpeed;
	int           mCurrent;
	int           mTotal;
	std::string   mPosition;
	std::string   mDuration;
	const cMarks *mMarks;
	std::string   mPrompt;
	eMessageType  mType;
	std::string   mText;
	std::string   mButtonRed;
	std::string   mButtonGreen;
	std::string   mButtonYellow;
	std::string   mButtonBlue;

protected:
	virtual cxType GetTokenData(const txToken &Token);
	virtual const cMarks *GetMarks(void) const { return mMarks; }

public:
	cText2SkinDisplayReplay(cText2SkinLoader *Loader, bool ModeOnly);
	virtual ~cText2SkinDisplayReplay();
	virtual void SetTitle(const char *Title);
	virtual void SetMode(bool Play, bool Forward, int Speed);
	virtual void SetProgress(int Current, int Total);
	virtual void SetMarks(const cMarks *Marks);
	virtual void SetCurrent(const char *Current);
	virtual void SetTotal(const char *Total);
	virtual void SetJump(const char *Jump);
	virtual void SetMessage(eMessageType Type, const char *Text);
	virtual void SetButtons(const char *Red, const char *Green, const char *Yellow,
	                        const char *Blue);

	virtual void Flush(void) { cText2SkinRender::Flush(); }
};

class cText2SkinDisplayMessage: public cSkinDisplayMessage, public cText2SkinRender {
private:
	eMessageType mType;
	std::string  mText;

protected:
	virtual cxType GetTokenData(const txToken &Token);

public:
	cText2SkinDisplayMessage(cText2SkinLoader *Loader);
	virtual ~cText2SkinDisplayMessage();
	virtual void SetMessage(eMessageType Type, const char *Text);

	virtual void Flush(void) { cText2SkinRender::Flush(); }
};

class cText2SkinDisplayMenu: public cSkinDisplayMenu, public cText2SkinRender {
private:
	cSkinDisplayMenu *mFallbackDisplay;
	int               mMaxItems;

	// common for all menus
	std::string       mTitle;
	std::string       mButtonRed;
	std::string       mButtonGreen;
	std::string       mButtonYellow;
	std::string       mButtonBlue;
	eMessageType      mMessageType;
	std::string       mMessageText;

	// detailed event view
	const cEvent     *mEvent;
	std::string       ExtPresentDescription;
	// detailed recording
	const cRecording *mRecording;
	std::string       ExtRecordingDescription;
	// long text
	std::string       mText;

	// list view
	struct tListItem {
		std::string   text;
		std::string   tabs[MaxTabs];
		bool          sel;

		tListItem(const std::string &Text, bool Sel): text(Text), sel(Sel) {}

		bool operator!=(const tListItem &b) { return b.text != text || b.sel != sel; }
	};
	typedef std::vector<tListItem> tListItems;

	tListItems        mItems;
	uint              mCurrentItem;

protected:
	virtual cxType GetTokenData(const txToken &Token);
	virtual int GetTab(int n) { return cSkinDisplayMenu::Tab(n); }
	virtual bool HasTabText(int Index, int n);
	virtual void SetEditableWidth(int Width) { cSkinDisplayMenu::SetEditableWidth(Width); }
	virtual int MaxItems(void) { return mMaxItems; }
	virtual void SetMaxItems(int MaxItems) { mMaxItems = MaxItems; }
	virtual int GetMaxItems(void) { return mMaxItems; }

public:
	cText2SkinDisplayMenu(cText2SkinLoader *Loader);
	virtual ~cText2SkinDisplayMenu();

	virtual void Clear(void);
	virtual void SetTitle(const char *Title);
	virtual void SetButtons(const char *Red, const char *Green, const char *Yellow, const char *Blue);
	virtual void SetMessage(eMessageType Type, const char *Text);
	virtual void SetItem(const char *Text, int Index, bool Current, bool Selectable);
	virtual void SetEvent(const cEvent *Event);
	virtual void SetRecording(const cRecording *Recording);
	virtual void SetText(const char *Text, bool FixedFont);
	virtual void SetTabs(int Tab1, int Tab2, int Tab3, int Tab4, int Tab5);
	virtual void Scroll(bool Up, bool Page);

	virtual void Flush(void);
};

inline bool cText2SkinDisplayMenu::HasTabText(int Index, int n)
{
	if (Index < 0 || mItems.size () > (uint)Index)
		return n == -1
		       ? mItems[Index].text.length() > 0
		       : mItems[Index].tabs[n].length() > 0;
	return false;
}

inline void cText2SkinDisplayMenu::Flush(void)
{
	if (mFallbackDisplay != NULL)
		mFallbackDisplay->Flush();
	else
		cText2SkinRender::Flush();
}

class cText2SkinDisplayTracks: public cSkinDisplayTracks, public cText2SkinRender {
private:
	int         mMaxItems;

	std::string mTitle;

	typedef std::string tListItem;
	typedef std::vector<tListItem> tListItems;

	tListItems  mItems;
	uint        mCurrentItem;
	int         mAudioChannel;

protected:
	virtual cxType GetTokenData(const txToken &Token);
	virtual bool HasTabText(int Index, int n);
	virtual void SetMaxItems(int MaxItems) { mMaxItems = MaxItems; }

public:
	static const std::string &ChannelName(int AudioChannel);

	cText2SkinDisplayTracks(cText2SkinLoader *Loader, const char *Title, int NumTracks,
	                        const char * const *Tracks);
	virtual ~cText2SkinDisplayTracks();

	virtual void SetTrack(int Index, const char * const *Tracks);
	virtual void SetAudioChannel(int AudioChannel);

	virtual void Flush(void) { cText2SkinRender::Flush(); }
};

inline bool cText2SkinDisplayTracks::HasTabText(int Index, int n)
{
	if (Index < 0 || (mItems.size () > (uint)Index && n <= 0))
		return mItems[Index].length() > 0;
	return false;
}

#endif // VDR_TEXT2SKIN_SKIN_H
