//								-*- c++ -*-

#include "render.h"
#include "loader.h"
#include "display.h"
#include "scroller.h"
#include "status.h"
#include "common.h"
#include "xml/string.h"
#include <vdr/menu.h>
#include <vdr/plugin.h>

// --- cText2SkinDisplayChannel -----------------------------------------------

cText2SkinDisplayChannel::cText2SkinDisplayChannel(cText2SkinLoader *Loader, bool WithInfo):
		cText2SkinRender(Loader, WithInfo ? cxDisplay::channelInfo : cxDisplay::channelSmall),
		mFallbackDisplay(NULL),
		mChannel(NULL),
		mNumber(0),
		mPresent(NULL),
		mFollowing(NULL),
		mType(mtStatus),
		mText("") ,
		mButtonRed(""),
		mButtonGreen(""),
		mButtonYellow(""),
		mButtonBlue("")
{
	if (Fallback() != NULL) {
		mFallbackDisplay = Fallback()->DisplayChannel(WithInfo);
		Skins.Message(mtError, tr("Skin too large or incorrectly aligned"), 2);
	}
}

cText2SkinDisplayChannel::~cText2SkinDisplayChannel()
{
	if (mFallbackDisplay != NULL)
		delete mFallbackDisplay;
}

void cText2SkinDisplayChannel::SetChannel(const cChannel *Channel, int Number)
{
	if (mFallbackDisplay != NULL) {
		mFallbackDisplay->SetChannel(Channel, Number);
		return;
	}

	UpdateLock();
	if (mChannel != Channel || mNumber != Number) {
		mChannel = Channel;
		mNumber = Number;
		SetDirty(cxRefresh::update);
	}
	UpdateUnlock();
}

void cText2SkinDisplayChannel::SetEvents(const cEvent *Present, const cEvent *Following)
{
	if (mFallbackDisplay != NULL) {
		mFallbackDisplay->SetEvents(Present, Following);
		return;
	}

	UpdateLock();
	if (mPresent != Present || mFollowing != Following) {
		mPresent = Present;
		mFollowing = Following;
		SetDirty(cxRefresh::update);
	}
	UpdateUnlock();
}

void cText2SkinDisplayChannel::SetMessage(eMessageType Type, const char *Text)
{
	if (mFallbackDisplay != NULL) {
		mFallbackDisplay->SetMessage(Type, Text);
		return;
	}

	UpdateLock();
	if (Text == NULL) Text = "";
	if (mType != Type || mText != Text) {
		mType = Type;
		mText = Text;
		SetDirty(cxRefresh::all);
	}
	UpdateUnlock();
}

void cText2SkinDisplayChannel::SetButtons(const char *Red, const char *Green, const char *Yellow, const char *Blue)
{
	if (mFallbackDisplay != NULL) {
		mFallbackDisplay->SetButtons(Red, Green, Yellow, Blue);
		return;
	}

	UpdateLock();
	Dprintf("SetButtons(%s, %s, %s, %s)\n", Red, Green, Yellow, Blue);
	if (Red    == NULL) Red    = "";
	if (Green  == NULL) Green  = "";
	if (Yellow == NULL) Yellow = "";
	if (Blue   == NULL) Blue   = "";
	if (mButtonRed != Red || mButtonGreen != Green || mButtonYellow != Yellow
			|| mButtonBlue != Blue) {
		mButtonRed    = Red;
		mButtonGreen  = Green;
		mButtonYellow = Yellow;
		mButtonBlue   = Blue;
		SetDirty(cxRefresh::update);
	}
	UpdateUnlock();
}

cxType cText2SkinDisplayChannel::GetTokenData(const txToken &Token)
{
	switch (Token.Type) {
	case tChannelNumber:
		return ChannelNumber(mChannel, mNumber);

	case tChannelName:
		return mChannel != NULL
		       ? (cxType)ChannelName(mChannel, mNumber)
		       : (cxType)false;

	case tChannelShortName:
		return mChannel != NULL
		       ? (cxType)ChannelShortName(mChannel, mNumber)
		       : (cxType)false;

	case tChannelBouquet:
		return mChannel != NULL
		       ? (cxType)mChannel->Provider()
		       : (cxType)false;

	case tChannelPortal:
		return mChannel != NULL
		       ? (cxType)mChannel->PortalName()
		       : (cxType)false;

	case tChannelSource:
		return mChannel != NULL && Sources.Get(mChannel->Source()) != NULL
		       ? (cxType)Sources.Get(mChannel->Source())->Description()
		       : (cxType)false;

	case tTChannelID:
		return mChannel != NULL
		       ? (cxType)(const char*)mChannel->GetChannelID().ToString()
		       : (cxType)false;

	case tPresentStartDateTime:
		return mPresent != NULL
		       ? (cxType)TimeType(mPresent->StartTime(), Token.Attrib.Text)
		       : (cxType)false;

	case tPresentVPSDateTime:
		return mPresent != NULL
		       ? (cxType)TimeType(mPresent->Vps(), Token.Attrib.Text)
		       : (cxType)false;

	case tPresentEndDateTime: return mPresent != NULL
		       ? (cxType)TimeType(mPresent->EndTime(), Token.Attrib.Text)
		       : (cxType)false;

	case tPresentProgress:
		return mPresent != NULL
		       ? (cxType)DurationType(time(NULL) - mPresent->StartTime(),
		                              Token.Attrib.Text)
		       : (cxType)false;

	case tPresentDuration:
		return mPresent != NULL
		       ? (cxType)DurationType(mPresent->Duration(), Token.Attrib.Text)
		       : (cxType)false;

	case tPresentRemaining:
		if (mPresent != NULL && time(NULL) - mPresent->StartTime()
		                        <= mPresent->Duration()) {
			return (cxType)DurationType(mPresent->Duration() - (time(NULL) - mPresent->StartTime()),
		                                    Token.Attrib.Text);
		}
		return false;

	case tPresentTitle:
		return mPresent != NULL
		       ? (cxType)mPresent->Title()
		       : (cxType)false;

	case tPresentShortText:
		return mPresent != NULL
		       ? (cxType)mPresent->ShortText()
		       : (cxType)false;

	case tPresentDescription:
		return mPresent != NULL
		       ? (cxType)mPresent->Description()
		       : (cxType)false;

	case tFollowingStartDateTime:
		return mFollowing != NULL
		       ? (cxType)TimeType(mFollowing->StartTime(), Token.Attrib.Text)
		       : (cxType)false;

	case tFollowingVPSDateTime:
		return mFollowing != NULL
		       ? (cxType)TimeType(mFollowing->Vps(), Token.Attrib.Text)
		       : (cxType)false;

	case tFollowingEndDateTime:
		return mFollowing != NULL
		       ? (cxType)TimeType(mFollowing->EndTime(), Token.Attrib.Text)
		       : (cxType)false;

	case tFollowingDuration:
		return mFollowing != NULL
		       ? (cxType)DurationType(mFollowing->Duration(), Token.Attrib.Text)
		       : (cxType)false;

	case tFollowingTitle:
		return mFollowing != NULL
		       ? (cxType)mFollowing->Title()
		       : (cxType)false;

	case tFollowingShortText:
		return mFollowing != NULL
		       ? (cxType)mFollowing->ShortText()
		       : (cxType)false;

	case tFollowingDescription:
		return mFollowing != NULL
		       ? (cxType)mFollowing->Description()
		       : (cxType)false;

	case tLanguage: {
			cDevice *dev = cDevice::PrimaryDevice();
			eTrackType trackType = dev->GetCurrentAudioTrack();
			const tTrackId *track = dev->GetTrack(trackType);
			if (track) {
				std::string buffer(track->language);
				if (trackType >= ttDolby)
					buffer.append("DD");
				return (cxType)buffer.c_str();
			}
			return (cxType)false;
		}
		return false;

	case tVideoSizeWidth:    {
#if VDRVERSNUM >= 10707
			int width, height;
			double aspect;
			cDevice *dev = cDevice::PrimaryDevice();
			dev->GetVideoSize(width, height, aspect);
			return (cxType)width;
#else
			return 0;
#endif
		}

	case tVideoSizeHeight:    {
#if VDRVERSNUM >= 10707
			int width, height;
			double aspect;
			cDevice *dev = cDevice::PrimaryDevice();
			dev->GetVideoSize(width, height, aspect);
			return (cxType)height;
#else
			return 0;
#endif
		}

	case tHasTeletext:
	case tChannelHasTeletext:
		return mChannel != NULL && mChannel->Tpid() != 0;

	case tHasMultilang:
	case tChannelHasMultilang:
		return mChannel != NULL && mChannel->Apid(1) != 0;

	case tHasDolby:
	case tChannelHasDolby:
		return mChannel != NULL && mChannel->Dpid(0) != 0;

	case tIsEncrypted:
	case tChannelIsEncrypted:
		return mChannel != NULL && mChannel->Ca() != 0;

	case tIsRadio:
	case tChannelIsRadio:
		return mChannel != NULL && ISRADIO(mChannel);

	case tHasVPS:
	case tChannelHasVPS:
		return mPresent != NULL && mPresent->Vps() != 0;

	case tPresentHasVPS:
		return mPresent != NULL && mPresent->Vps() != 0 && mPresent->Vps() != mPresent->StartTime();

	case tHasTimer:
	case tPresentHasTimer:
		return mPresent != NULL && mPresent->HasTimer();

	case tIsRunning:
	case tPresentIsRunning:
		return mPresent != NULL && mPresent->IsRunning();

	case tFollowingHasTimer:
		return mFollowing != NULL && mFollowing->HasTimer();

	case tFollowingIsRunning:
		return mFollowing != NULL && mFollowing->IsRunning();

	case tFollowingHasVPS:
		return mFollowing != NULL && mFollowing->Vps() != 0
		    && mFollowing->Vps() != mFollowing->StartTime();

	case tMessage:
		return mText;

	case tMessageInfo:
		return mType == mtInfo
		       ? (cxType)mText
		       : (cxType)false;

	case tMessageWarning:
		return mType == mtWarning
		       ? (cxType)mText
		       : (cxType)false;

	case tMessageStatus:
		return mType == mtStatus
		       ? (cxType)mText
		       : (cxType)false;

	case tMessageError:
		return mType == mtError
		       ? (cxType)mText
		       : (cxType)false;

	case tButtonRed:
		return mButtonRed;

	case tButtonGreen:
		return mButtonGreen;

	case tButtonYellow:
		return mButtonYellow;

	case tButtonBlue:
		return mButtonBlue;

	default:
		return cText2SkinRender::GetTokenData(Token);
	}
}

// --- cText2SkinDisplayVolume ------------------------------------------------

cText2SkinDisplayVolume::cText2SkinDisplayVolume(cText2SkinLoader *Loader):
		cText2SkinRender(Loader, cxDisplay::volume),
		mCurrent(0),
		mTotal(0),
		mMute(false)
{
}

cText2SkinDisplayVolume::~cText2SkinDisplayVolume()
{
}

void cText2SkinDisplayVolume::SetVolume(int Current, int Total, bool Mute)
{
	UpdateLock();
	if (mCurrent != Current || mTotal != Total || mMute != Mute) {
		mCurrent = Current;
		mTotal = Total;
		mMute = Mute;
		SetDirty(cxRefresh::update);
	}
	UpdateUnlock();
}

cxType cText2SkinDisplayVolume::GetTokenData(const txToken &Token) {
	switch (Token.Type) {
	case tVolumeCurrent: return mCurrent;

	case tVolumeTotal:   return mTotal;

	case tIsMute:
	case tVolumeIsMute:  return mMute;

	default:             return cText2SkinRender::GetTokenData(Token);
	}
}

// --- cText2SkinDisplayReplay ------------------------------------------------

cText2SkinDisplayReplay::cText2SkinDisplayReplay(cText2SkinLoader *Loader, bool ModeOnly):
		cText2SkinRender(Loader, ModeOnly ? cxDisplay::replaySmall : cxDisplay::replayInfo),
		mTitle(""),
		mStateInfo(false),
		mPlay(false),
		mForward(false),
		mSpeed(0),
		mCurrent(0),
		mTotal(0),
		mMarks(NULL),
		mPrompt(""),
		mType(mtStatus),
		mText(""),
		mButtonRed(""),
		mButtonGreen(""),
		mButtonYellow(""),
		mButtonBlue("")
{
}

cText2SkinDisplayReplay::~cText2SkinDisplayReplay()
{
}

void cText2SkinDisplayReplay::SetTitle(const char *Title)
{
	UpdateLock();
	if (Title == NULL) Title = "";
	if (mTitle != Title) {
		mTitle = Title;
		SetDirty(cxRefresh::update);
	}
	UpdateUnlock();
}

void cText2SkinDisplayReplay::SetMode(bool Play, bool Forward, int Speed)
{
	Dprintf("SetMode(%d, %d, %d)\n", Play, Forward, Speed);
	UpdateLock();
	if (!mStateInfo || mPlay != Play || mForward != Forward || mSpeed != Speed) {
		mStateInfo = true;
		mPlay = Play;
		mForward = Forward;
		mSpeed = Speed;
		SetDirty(cxRefresh::update);
	}
	UpdateUnlock();
}

void cText2SkinDisplayReplay::SetProgress(int Current, int Total)
{
	UpdateLock();
	if (mCurrent != Current || mTotal != Total) {
		mCurrent = Current;
		mTotal = Total;
		// SetDirty(cxRefresh::update); TODO: let this cause a display update every frame?
	}
	UpdateUnlock();
}

void cText2SkinDisplayReplay::SetMarks(const cMarks *Marks)
{
	UpdateLock();
	mMarks = Marks;
	SetDirty(cxRefresh::update);
	UpdateUnlock();
}

void cText2SkinDisplayReplay::SetCurrent(const char *Current)
{
	UpdateLock();
	if (Current == NULL) Current = "";
	if (mPosition != Current) {
		mPosition = Current;
		SetDirty(cxRefresh::update);
	}
	UpdateUnlock();
}

void cText2SkinDisplayReplay::SetTotal(const char *Total)
{
	UpdateLock();
	if (Total == NULL) Total = "";
	if (mDuration != Total) {
		mDuration = Total;
		SetDirty(cxRefresh::update);
	}
	UpdateUnlock();
}

void cText2SkinDisplayReplay::SetJump(const char *Jump)
{
	UpdateLock();
	if (Jump == NULL) Jump = "";
	if (mPrompt != Jump) {
		mPrompt = Jump;
		SetDirty(cxRefresh::update);
	}
	UpdateUnlock();
}

void cText2SkinDisplayReplay::SetMessage(eMessageType Type, const char *Text)
{
	UpdateLock();
	if (Text == NULL) Text = "";
	if (mType != Type || mText != Text) {
		mType = Type;
		mText = Text;
		SetDirty(cxRefresh::all);
	}
	UpdateUnlock();
}

void cText2SkinDisplayReplay::SetButtons(const char *Red, const char *Green, const char *Yellow,
                                         const char *Blue)
{
	UpdateLock();
	Dprintf("SetButtons(%s, %s, %s, %s)\n", Red, Green, Yellow, Blue);
	if (Red    == NULL) Red    = "";
	if (Green  == NULL) Green  = "";
	if (Yellow == NULL) Yellow = "";
	if (Blue   == NULL) Blue   = "";
	if (mButtonRed != Red || mButtonGreen != Green || mButtonYellow != Yellow || mButtonBlue != Blue) {
		mButtonRed    = Red;
		mButtonGreen  = Green;
		mButtonYellow = Yellow;
		mButtonBlue   = Blue;
		SetDirty(cxRefresh::update);
	}
	UpdateUnlock();
}

cxType cText2SkinDisplayReplay::GetTokenData(const txToken &Token)
{
	switch (Token.Type) {
	case tReplayTitle:
		return mTitle;

	case tReplayPositionIndex:
		return DurationType(mCurrent,
		                    Text2SkinStatus.ReplayFramesPerSecond(),
		                    Token.Attrib.Text);

	case tReplayDurationIndex:
		return DurationType(mTotal,
		                    Text2SkinStatus.ReplayFramesPerSecond(),
		                    Token.Attrib.Text);

	case tReplayPosition:
		return mPosition;

	case tReplayDuration:
		return mDuration;

	case tReplayRemaining:
		return DurationType(mTotal - mCurrent,
		                    Text2SkinStatus.ReplayFramesPerSecond(),
		                    Token.Attrib.Text);

	case tReplayPrompt:
		return mPrompt;

	case tIsPlaying:
	case tReplayIsPlaying:
		return mStateInfo && mSpeed == -1 && mPlay;

	case tIsPausing:
	case tReplayIsPausing:
		return mStateInfo && mSpeed == -1 && !mPlay;

	case tIsFastForward:
	case tReplayIsFastForward:
		if (mStateInfo && mSpeed != -1 && mPlay && mForward) {
			return Token.Attrib.Type == aNumber
			       ? (cxType)(mSpeed == Token.Attrib.Number)
			       : (cxType)true;
		}
		return false;

	case tIsFastRewind:
	case tReplayIsFastRewind:
		if (mStateInfo && mSpeed != -1 && mPlay && !mForward) {
			return Token.Attrib.Type == aNumber
			       ? (cxType)(mSpeed == Token.Attrib.Number)
			       : (cxType)true;
		}
		return false;

	case tIsSlowForward:
	case tReplayIsSlowForward:
		if (mStateInfo && mSpeed != -1 && !mPlay && mForward) {
			return Token.Attrib.Type == aNumber
			       ? (cxType)(mSpeed == Token.Attrib.Number)
			       : (cxType)true;
		}
		return false;

	case tIsSlowRewind:
	case tReplayIsSlowRewind:
		if (mStateInfo && mSpeed != -1 && !mPlay && !mForward) {
			return Token.Attrib.Type == aNumber
			       ? (cxType)(mSpeed == Token.Attrib.Number)
			       : (cxType)true;
		}
		return false;

	case tLanguage: {
		cDevice *dev = cDevice::PrimaryDevice();
		eTrackType trackType = dev->GetCurrentAudioTrack();
		const tTrackId *track = dev->GetTrack(trackType);
		if (track) {
			std::string buffer(track->language);
			if (trackType >= ttDolby)
				buffer.append("DD");
			return (cxType)buffer.c_str();
		}
		return (cxType)false;
	}
	return false;

	case tMessage:
		return mText;

	case tMessageInfo:
		return mType == mtInfo
		       ? (cxType)mText
		       : (cxType)false;

	case tMessageWarning:
		return mType == mtWarning
		       ? (cxType)mText
		       : (cxType)false;

	case tMessageStatus:
		return mType == mtStatus
		       ? (cxType)mText
		       : (cxType)false;

	case tMessageError:
		return mType == mtError
		       ? (cxType)mText
		       : (cxType)false;

	case tButtonRed:
		return mButtonRed;

	case tButtonGreen:
		return mButtonGreen;

	case tButtonYellow:
		return mButtonYellow;

	case tButtonBlue:
		return mButtonBlue;

	default:
		return cText2SkinRender::GetTokenData(Token);
	}
}

// --- cText2SkinDisplayMessage -----------------------------------------------

cText2SkinDisplayMessage::cText2SkinDisplayMessage(cText2SkinLoader *Loader):
		cText2SkinRender(Loader, cxDisplay::message),
		mText("")
{
}

cText2SkinDisplayMessage::~cText2SkinDisplayMessage()
{
}

void cText2SkinDisplayMessage::SetMessage(eMessageType Type, const char *Text)
{
	UpdateLock();
	if (Text == NULL) Text = "";
	if (mType != Type || mText != Text) {
		mType = Type;
		mText = Text;
		SetDirty(cxRefresh::all);
	}
	UpdateUnlock();
}

cxType cText2SkinDisplayMessage::GetTokenData(const txToken &Token)
{
	switch (Token.Type) {
	case tMessage:
		return mText;

	case tMessageInfo:
		return mType == mtInfo
		       ? (cxType)mText
		       : (cxType)false;

	case tMessageWarning:
		return mType == mtWarning
		       ? (cxType)mText
		       : (cxType)false;

	case tMessageStatus:
		return mType == mtStatus
		       ? (cxType)mText
		       : (cxType)false;

	case tMessageError:
		return mType == mtError
		       ? (cxType)mText
		       : (cxType)false;

	default:
		return cText2SkinRender::GetTokenData(Token);
	}
}

// --- cText2SkinDisplayMenu --------------------------------------------------

cText2SkinDisplayMenu::cText2SkinDisplayMenu(cText2SkinLoader *Loader):
		cText2SkinRender(Loader, cxDisplay::menu),
		mFallbackDisplay(NULL),
		mMaxItems(0),
		mTitle(""),
		mButtonRed(""),
		mButtonGreen(""),
		mButtonYellow(""),
		mButtonBlue(""),
		mMessageType(mtStatus),
		mMessageText(""),
		mEvent(NULL),
		mRecording(NULL),
		mItems(),
		mCurrentItem((uint)-1)
{
	if (Fallback() != NULL) {
		mFallbackDisplay = Fallback()->DisplayMenu();
		mMaxItems = mFallbackDisplay->MaxItems();
		Skins.Message(mtError, tr("Skin too large or incorrectly aligned"), 2);
		return;
	}

	cxDisplay *disp = Loader->Data()->Get(cxDisplay::menu);
	const cxObject *area = NULL;
	for (uint i = 0; i < disp->Objects(); ++i) {
		const cxObject *o = disp->GetObject(i);
		if (o->Type() == cxObject::list) {
			area = o;
			break;
		}
	}

	if (area != NULL) {
		const cxObject *item = area->GetObject(0);
		if (item != NULL && item->Type() == cxObject::item)
			mMaxItems = area->Size().h / item->Size().h;
	}
}

cText2SkinDisplayMenu::~cText2SkinDisplayMenu()
{
	if (mFallbackDisplay != NULL)
		delete mFallbackDisplay;
}

void cText2SkinDisplayMenu::Clear(void)
{
	if (mFallbackDisplay != NULL) {
		printf("fallback clear\n");
		mFallbackDisplay->Clear();
		return;
	}

	UpdateLock();
	mItems.clear();
	mCurrentItem = (uint)-1;
	mEvent = NULL;
	ExtPresentDescription = "";
	mRecording = NULL;
	ExtRecordingDescription = "";
	mText = "";
	cText2SkinRender::Clear();
	SetDirty(cxRefresh::all);
	UpdateUnlock();
}

void cText2SkinDisplayMenu::SetTitle(const char *Title)
{
	if (mFallbackDisplay != NULL) {
		printf("fallback title\n");
		mFallbackDisplay->SetTitle(Title);
		return;
	}

	UpdateLock();
	if (Title == NULL) Title = "";
	if (mTitle != Title) {
		mUpdate.timerConflict = true;
		mUpdate.events = true;
		mUpdate.resetMarquee = true;
		mTitle = Title;
		SetDirty(cxRefresh::update);
	}
	UpdateUnlock();
}

void cText2SkinDisplayMenu::SetButtons(const char *Red, const char *Green, const char *Yellow,
                                       const char *Blue)
{
	if (mFallbackDisplay != NULL) {
		mFallbackDisplay->SetButtons(Red, Green, Yellow, Blue);
		return;
	}

	UpdateLock();
	if (Red    == NULL) Red    = "";
	if (Green  == NULL) Green  = "";
	if (Yellow == NULL) Yellow = "";
	if (Blue   == NULL) Blue   = "";
	if (mButtonRed != Red || mButtonGreen != Green || mButtonYellow != Yellow
			|| mButtonBlue != Blue) {
		mButtonRed    = Red;
		mButtonGreen  = Green;
		mButtonYellow = Yellow;
		mButtonBlue   = Blue;
		SetDirty(cxRefresh::update);
	}
	UpdateUnlock();
}

void cText2SkinDisplayMenu::SetMessage(eMessageType Type, const char *Text)
{
	if (mFallbackDisplay != NULL) {
		printf("fallback message\n");
		mFallbackDisplay->SetMessage(Type, Text);
		return;
	}

	UpdateLock();
	if (Text == NULL) Text = "";
	if (mMessageType != Type || mMessageText != Text) {
		mMessageType = Type;
		mMessageText = Text;
		SetDirty(cxRefresh::all);
	}
	UpdateUnlock();
}

void cText2SkinDisplayMenu::SetItem(const char *Text, int Index, bool Current, bool Selectable)
{
	if (mFallbackDisplay != NULL) {
		mFallbackDisplay->SetItem(Text, Index, Current, Selectable);
		return;
	}

	UpdateLock();
	if (Text == NULL) Text = "";
	tListItem item(Text, Selectable);

	for (int i = 0; i < MaxTabs; ++i) {
		const char *tab = GetTabbedText(Text, i);
		if (tab)
			item.tabs[i] = tab;
		if (!Tab(i + 1))
			break;
	}

	if (mItems.size() <= (uint)Index) {
		mItems.push_back(item);
		mDirtyItems.push_back(Index);
		SetDirty(cxRefresh::list);
	}
	else if (mItems[Index] != item) {
		mItems[Index] = item;
		// refresh only the changed items
		mDirtyItems.push_back(Index);
		SetDirty(cxRefresh::list);
	}

	if (Current && mCurrentItem != (uint)Index) {
		// refresh only the changed items
		mDirtyItems.push_back(mCurrentItem);
		mDirtyItems.push_back(Index);
		SetDirty(cxRefresh::list);
		mCurrentItem = Index;
	}

	if (Current)
		mRender->mMenuScrollbar.currentOnScreen = (uint)Index;
	UpdateUnlock();
}

void cText2SkinDisplayMenu::SetEvent(const cEvent *Event)
{
	if (mFallbackDisplay != NULL) {
		mFallbackDisplay->SetEvent(Event);
		return;
	}

	UpdateLock();
	if (mEvent != Event) {
		mEvent = Event;
		ExtPresentDescription = "";
		if (mEvent != NULL)
			SetDirty(cxRefresh::update);
	}
	UpdateUnlock();
}

void cText2SkinDisplayMenu::SetRecording(const cRecording *Recording)
{
	if (mFallbackDisplay != NULL) {
		mFallbackDisplay->SetRecording(Recording);
		return;
	}

	UpdateLock();
	// yet unused
	if (mRecording != Recording) {
		mRecording = Recording;
		ExtRecordingDescription = "";
		if (mRecording != NULL)
			SetDirty(cxRefresh::update);
	}
	UpdateUnlock();
}

void cText2SkinDisplayMenu::SetText(const char *Text, bool FixedFont)
{
	if (mFallbackDisplay != NULL) {
		mFallbackDisplay->SetText(Text, FixedFont);
		return;
	}

	UpdateLock();
	if (Text == NULL) Text = "";
	if (mText != Text) {
		mText = Text;
		SetDirty(cxRefresh::update);
	}
	UpdateUnlock();
}

void cText2SkinDisplayMenu::SetTabs(int Tab1, int Tab2, int Tab3, int Tab4, int Tab5)
{
	if (mFallbackDisplay != NULL) {
		mFallbackDisplay->SetTabs(Tab1, Tab2, Tab3, Tab4, Tab5);
		return;
	}

	UpdateLock();
	cSkinDisplayMenu::SetTabs(Tab1, Tab2, Tab3, Tab4, Tab5);
	UpdateUnlock();
}

void cText2SkinDisplayMenu::Scroll(bool Up, bool Page)
{
	if (mFallbackDisplay != NULL) {
		mFallbackDisplay->Scroll(Up, Page);
		return;
	}

	UpdateLock();
	cText2SkinRender::Scroll(Up, Page);
	SetDirty(cxRefresh::scroll);
	UpdateUnlock();
}

cxType cText2SkinDisplayMenu::GetTokenData(const txToken &Token)
{
	switch (Token.Type) {
	case tMenuItem:
	case tMenuGroup:
	case tMenuCurrent:
		break;
	default:
		if (Token.Tab >= 0) return false;
		break;
	}

	switch (Token.Type) {
	case tMenuTitle:
		return mTitle;

	case tMenuItem:
		if (Token.Index < 0)
			return false;

		if (mItems.size() <= (uint)Token.Index || !mItems[Token.Index].sel ||
		    mCurrentItem == (uint)Token.Index)
			return false;

		if (Token.Tab < 0)
			return Token.Attrib.Type == aNumber
			       ? (cxType)mItems[Token.Index].tabs[Token.Attrib.Number]
			       : (cxType)mItems[Token.Index].text;

		return (cxType)mItems[Token.Index].tabs[Token.Tab];

	case tIsMenuItem:
		return mItems.size() > (uint)Token.Index && mItems[Token.Index].sel
		       && mCurrentItem != (uint)Token.Index;

	case tMenuCurrent:
		if (mItems.size() <= mCurrentItem)
			return false;

		if (Token.Index < 0)
			return Token.Attrib.Type == aNumber
			       ? (cxType)mItems[mCurrentItem].tabs[Token.Attrib.Number]
			       : (cxType)mItems[mCurrentItem].text;

		if (mItems.size() <= (uint)Token.Index || !mItems[Token.Index].sel ||
		    mCurrentItem != (uint)Token.Index)
			return false;

		if (Token.Tab < 0)
			return Token.Attrib.Type == aNumber
			       ? (cxType)mItems[Token.Index].tabs[Token.Attrib.Number]
			       : (cxType)mItems[Token.Index].text;

		return (cxType)mItems[Token.Index].tabs[Token.Tab];

	case tIsMenuCurrent:
		return mItems.size() > (uint)Token.Index && mItems[Token.Index].sel
		       && mCurrentItem == (uint)Token.Index;

	case tMenuGroup:
		if (Token.Index < 0)
			return false;

		if (mItems.size() <= (uint)Token.Index || mItems[Token.Index].sel)
			return false;

		if (Token.Tab < 0)
			return Token.Attrib.Type == aNumber
			       ? (cxType)mItems[Token.Index].tabs[Token.Attrib.Number]
			       : (cxType)mItems[Token.Index].text;

		return (cxType)mItems[Token.Index].tabs[Token.Tab];

	case tIsMenuGroup:
		return mItems.size() > (uint)Token.Index && !mItems[Token.Index].sel;

	case tButtonRed:
		return mButtonRed;

	case tButtonGreen:
		return mButtonGreen;

	case tButtonYellow:
		return mButtonYellow;

	case tButtonBlue:
		return mButtonBlue;

	case tMessage:
		return mMessageText;

	case tMessageInfo:
		return mMessageType == mtInfo
		       ? (cxType)mMessageText
		       : (cxType)false;

	case tMessageWarning:
		return mMessageType == mtWarning
		       ? (cxType)mMessageText
		       : (cxType)false;

	case tMessageStatus:
		return mMessageType == mtStatus
		       ? (cxType)mMessageText
		       : (cxType)false;

	case tMessageError:
		return mMessageType == mtError
		       ? (cxType)mMessageText
		       : (cxType)false;

	case tPresentStartDateTime:
		return mEvent != NULL
		       ? (cxType)TimeType(mEvent->StartTime(), Token.Attrib.Text)
		       : (cxType)false;

	case tPresentVPSDateTime:
		return mEvent != NULL
		       ? (cxType)TimeType(mEvent->Vps(), Token.Attrib.Text)
		       : (cxType)false;

	case tPresentEndDateTime:
		return mEvent != NULL
		       ? (cxType)TimeType(mEvent->EndTime(), Token.Attrib.Text)
		       : (cxType)false;

	case tPresentProgress:
		return mEvent != NULL
		       ? (cxType)DurationType(time(NULL) - mEvent->StartTime(),
		                              Token.Attrib.Text)
		       : (cxType)false;

	case tPresentDuration:
		return mEvent != NULL
		       ? (cxType)DurationType(mEvent->Duration(), Token.Attrib.Text)
		       : (cxType)false;

	case tPresentRemaining:
		return mEvent != NULL
		       ? (cxType)DurationType(mEvent->Duration() - (time(NULL) - mEvent->StartTime()),
		                              Token.Attrib.Text)
		       : (cxType)false;

	case tPresentTitle:
		return mEvent != NULL
		       ? (cxType)mEvent->Title()
		       : (cxType)false;

	case tPresentShortText:
		return mEvent != NULL
		       ? (cxType)mEvent->ShortText()
		       : (cxType)false;

	case tPresentDescription:
		if (mEvent) {
			if (ExtPresentDescription == "") {
				// find corresponding timer
				const char *aux = NULL;
				for (cTimer *tim = Timers.First(); tim; tim = Timers.Next(tim))
					if (tim->Event() == mEvent)
						aux = tim->Aux();
				ExtPresentDescription = AddExtInfoToDescription(mEvent->Title(), mEvent->ShortText(), mEvent->Description(), aux, Text2SkinSetup.StripAux);
			}
			return (cxType)ExtPresentDescription;
		} else
			return (cxType)false;

	case tPresentLanguageCode:
		if (mEvent) {
			const cComponents *components = mEvent->Components();
			if (components) {
				int index = Token.Attrib.Number;

				// don't return language-code for the video-stream
				for (int i = 0; i < components->NumComponents(); i++) {
					const tComponent *c = components->Component(i);
					if (c->stream != 2) // only audio-streams
						index++;
					if (i == index) {
						std::string buffer(c->language);
						if (c->type == 1)
							buffer.append("MONO");
						if ((c->type == 2) || (c->type == 4))
							buffer.append("DUAL");
						if (c->type == 5)
							buffer.append("DD");
						return (cxType)buffer.c_str();
					}
				}
			}
		}
		return false;

	case tPresentLanguageDescription:
		if (mEvent) {
			const cComponents *components = mEvent->Components();
			if (components) {
				int index = Token.Attrib.Number;

				// don't return language-code for the video-stream
				for (int i = 0; i < components->NumComponents(); i++) {
					const tComponent *c = components->Component(i);
					if (c->stream != 2) // only audio-streams
						index++;
					if (i == index)
						return (cxType)c->description;
				}
			}
		}
		return false;

	case tPresentVideoAR:
		if (mEvent) {
			const cComponents *components = mEvent->Components();
			if (components) {
				for (int i = 0; i < components->NumComponents(); i++) {
					const tComponent *c = components->Component(i);
					if (c->stream == 1) {
						switch (c->type) {
						case 1:
							return "4:3";
						case 3:
							return "16:9";
						}
					}
				}
			}
		}
		return false;

	case tPresentEventID:
		return mEvent != NULL
		       ? (cxType)EventType(mEvent->EventID())
		       : (cxType)false;

	case tHasVPS:
	case tChannelHasVPS:
		return mEvent != NULL && mEvent->Vps() != 0;

	case tChannelName:
		if (mEvent) { // extended EPG
			cChannel *channel = Channels.GetByChannelID(mEvent->ChannelID(), true);
			return channel != NULL
			       ? (cxType)ChannelName(channel, 0)
			       : (cxType)false;
		}
		else if (mRecording) { // recording Info
			cRecordingInfo *recInfo = const_cast<cRecordingInfo*>(mRecording->Info());
			cChannel *channel = Channels.GetByChannelID(recInfo->ChannelID(), true);
			return channel != NULL
			       ? (cxType)ChannelName(channel, 0)
			       : (cxType)false;
		} else
			return (cxType)false;

	case tChannelShortName:
		if (mEvent) { // extended EPG
			cChannel *channel = Channels.GetByChannelID(mEvent->ChannelID(), true);
			return channel != NULL
			       ? (cxType)ChannelShortName(channel, 0)
			       : (cxType)false;
		}
		else if (mRecording) { // recording Info
			cRecordingInfo *recInfo = const_cast<cRecordingInfo*>(mRecording->Info());
			cChannel *channel = Channels.GetByChannelID(recInfo->ChannelID(), true);
			return channel != NULL
			       ? (cxType)ChannelShortName(channel, 0)
			       : (cxType)false;
		} else
			return (cxType)false;

	case tPresentHasVPS:
		return mEvent != NULL && mEvent->Vps() != 0 && mEvent->Vps() != mEvent->StartTime();

	case tHasTimer:
	case tPresentHasTimer:
		return mEvent != NULL && mEvent->HasTimer();

	case tIsRunning:
	case tPresentIsRunning:
		return mEvent != NULL && mEvent->IsRunning();

	case tMenuText:
		return mText;

	case tRecordingName:
		return mRecording != NULL
		       ? (cxType)mRecording->Name()
		       : (cxType)false;

	case tRecordingFilename:
		return mRecording != NULL
		       ? (cxType)mRecording->FileName()
		       : (cxType)false;

	case tRecordingPriority:
		return mRecording != NULL
#if APIVERSNUM >= 10721
		       ? (cxType)mRecording->Priority()
#else
		       ? (cxType)mRecording->priority
#endif
		       : (cxType)false;

	case tRecordingLifetime:
		return mRecording != NULL
#if APIVERSNUM >= 10721
		       ? (cxType)mRecording->Lifetime()
#else
		       ? (cxType)mRecording->lifetime
#endif
		       : (cxType)false;

	case tRecordingDateTime:
		return mRecording != NULL
#if APIVERSNUM >= 10721
		       ? (cxType)TimeType(mRecording->Start(), Token.Attrib.Text)
#else
		       ? (cxType)TimeType(mRecording->start, Token.Attrib.Text)
#endif
		       : (cxType)false;

	case tRecordingTitle:
		return mRecording != NULL
		       ? (cxType)mRecording->Info()->Title()
		       : (cxType)false;

	case tRecordingShortText:
		return mRecording != NULL
		       ? (cxType)mRecording->Info()->ShortText()
		       : (cxType)false;

	case tRecordingDescription:
		if (mRecording) {
			if (ExtRecordingDescription == "")
				ExtRecordingDescription = AddExtInfoToDescription(mRecording->Info()->Title(), mRecording->Info()->ShortText(), mRecording->Info()->Description(), Text2SkinSetup.ShowAux ? mRecording->Info()->Aux() : NULL, Text2SkinSetup.StripAux);
			return (cxType)ExtRecordingDescription;
		} else
			return (cxType)false;

	case tRecordingLanguageCode:
		if (mRecording) {
			const cComponents *components = mRecording->Info()->Components();
			if (components) {
				int index = Token.Attrib.Number;

				// don't return language-code for the video-stream
				for (int i = 0; i < components->NumComponents(); i++) {
					const tComponent *c = components->Component(i);
					if (c->stream != 2) // only audio-streams
						index++;
					if (i == index) {
						std::string buffer(c->language);
						if (c->type == 1)
							buffer.append("MONO");
						if ((c->type == 2) || (c->type == 4))
							buffer.append("DUAL");
						if (c->type == 5)
							buffer.append("DD");
						return (cxType)buffer.c_str();
					}
				}
			}
		}
		return false;

	case tRecordingLanguageDescription:
		if (mRecording) {
			const cComponents *components = mRecording->Info()->Components();
			if (components) {
				int index = Token.Attrib.Number;

				// don't return language-code for the video-stream
				for (int i = 0; i < components->NumComponents(); i++) {
					const tComponent *c = components->Component(i);
					if (c->stream != 2) // only audio-streams
						index++;
					if (i == index)
						return (cxType)c->description;
				}
			}
		}
		return false;

	case tRecordingVideoAR:
		if (mRecording) {
			const cComponents *components = mRecording->Info()->Components();
			if (components) {
				for (int i = 0; i < components->NumComponents(); i++) {
					const tComponent *c = components->Component(i);
					if (c->stream == 1) {
						switch (c->type) {
						case 1:
							return "4:3";
						case 3:
							return "16:9";
						}
					}
				}
			}
		}
		return false;

	case tRecordingSize:
		return mRecording != NULL
		       ? (cxType)GetRecordingSize(mRecording->FileName())
		       : (cxType)false;

	case tRecordingLength:
		return mRecording != NULL
#if VDRVERSNUM >= 10703
		       ? (cxType)GetRecordingLength(mRecording->FileName(), mRecording->FramesPerSecond(), mRecording->IsPesRecording())
#else
		       ? (cxType)GetRecordingLength(mRecording->FileName(), FRAMESPERSEC, true)
#endif
		       : (cxType)false;

	case tRecordingCuttedLength:
		return mRecording != NULL
#if VDRVERSNUM >= 10703
		       ? (cxType)GetRecordingCuttedLength(mRecording->FileName(), mRecording->FramesPerSecond(), mRecording->IsPesRecording())
#else
		       ? (cxType)GetRecordingCuttedLength(mRecording->FileName(), FRAMESPERSEC, true)
#endif
		       : (cxType)false;

	default:
		return cText2SkinRender::GetTokenData(Token);
	}
}

// --- cText2SkinDisplayTracks ------------------------------------------------

const std::string ChannelNames[] = { "", "stereo", "left", "right" };

cText2SkinDisplayTracks::cText2SkinDisplayTracks(cText2SkinLoader *Loader, const char *Title,
                                                 int NumTracks, const char * const *Tracks):
		cText2SkinRender(Loader, cxDisplay::audioTracks),
		mTitle(Title),
		mItems(),
		mCurrentItem((uint)-1),
		mAudioChannel(-1)
{
	Dprintf("NumTracks: %d\n", NumTracks);
	for (int i = 0; i < NumTracks; ++i) {
		tListItem item(Tracks[i]);
		mItems.push_back(item);
	}
}

cText2SkinDisplayTracks::~cText2SkinDisplayTracks()
{
}

const std::string &cText2SkinDisplayTracks::ChannelName(int AudioChannel)
{
	return ChannelNames[AudioChannel + 1];
}

void cText2SkinDisplayTracks::SetTrack(int Index, const char * const *Tracks)
{
	UpdateLock();
	Dprintf("SetTrack: %d (%s here, %s in array)\n", Index, Tracks[Index], mItems[Index].c_str());
	if (mCurrentItem != (uint)Index) {
		// refresh only the changed items
		mDirtyItems.push_back(mCurrentItem);
		mDirtyItems.push_back(Index);
		mCurrentItem = Index;
		SetDirty(cxRefresh::list);
	}
	UpdateUnlock();
}

void cText2SkinDisplayTracks::SetAudioChannel(int AudioChannel)
{
	UpdateLock();
	if (mAudioChannel != AudioChannel) {
		mAudioChannel = AudioChannel;
		SetDirty(cxRefresh::update);
	}
	UpdateUnlock();
}

cxType cText2SkinDisplayTracks::GetTokenData(const txToken &Token)
{
	if (Token.Tab >= 0)
		return false;

	int index = Token.Index;
	if (index >= 0 && mCurrentItem >= (uint)mMaxItems) {
		int offset = mCurrentItem - mMaxItems + 1;
		index += offset;
	}

	switch (Token.Type) {
	case tMenuTitle:
		return mTitle;

	case tMenuItem:
		if (index < 0)
			return false;

		if (mItems.size() <= (uint)index || mCurrentItem == (uint)index)
			return false;

		return (cxType)mItems[index];

	case tIsMenuItem:
		return mItems.size() > (uint)index && mCurrentItem != (uint)index;

	case tMenuCurrent:
		if (mItems.size() <= mCurrentItem)
			return false;

		if (index < 0)
			return (cxType)mItems[mCurrentItem];

		if (mItems.size() <= (uint)index || mCurrentItem != (uint)index)
			return false;

		return (cxType)mItems[index];

	case tIsMenuCurrent:
		return mItems.size() > (uint)index && mCurrentItem == (uint)index;

	case tAudioTrack:
		return mItems[mCurrentItem];

	case tAudioChannel:
		return ChannelName(mAudioChannel);

	default:
		return cText2SkinRender::GetTokenData(Token);
	}
}
