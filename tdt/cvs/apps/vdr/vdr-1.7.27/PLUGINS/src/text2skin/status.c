//								-*- c++ -*-

#include "status.h"
#include "render.h"
#include "menu.h"
#include <vdr/timers.h>
#include <vdr/plugin.h>
#include <vdr/menu.h>

const std::string ReplayNames[__REPLAY_COUNT__] =
	{ "", "normal", "mp3", "mplayer", "dvd", "vcd", "image", "audiocd" };

cText2SkinStatus Text2SkinStatus;

cText2SkinStatus::cText2SkinStatus(void):
		mRender(NULL),
		mReplayMode(replayNone),
		mReplayIsLoop(false),
		mReplayIsShuffle(false),
		mRecordings(),
		mCurrentRecording(0),
		mNextRecording(0),
		mLastLanguage(0),
		mReplayFramesPerSecond(0)
{
}

void cText2SkinStatus::SetRender(cText2SkinRender *Render)
{
	mRender           = Render;
	mCurrentRecording = 0;
	mNextRecording    = 0;
}

void cText2SkinStatus::Replaying(const cControl* /*Control*/, const char *Name,
								 const char *FileName, bool On)
{
	Dprintf("cText2SkinStatus::Replaying(%s)\n", Name);
	eReplayMode oldMode = mReplayMode;
	mReplay = NULL;

	if (mRender != NULL)
		mRender->UpdateLock();

	if (On) {
		mReplayMode = replayMPlayer;
		if (strlen(Name) > 6 && Name[0]=='[' && Name[3]==']' && Name[5]=='(') {
			int i;
			for (i = 6; Name[i]; ++i) {
				if (Name[i] == ' ' && Name[i-1] == ')')
					break;
			}
			if (Name[i]) { // replaying mp3
				mReplayMode      = replayMP3;
				mReplayIsLoop    = Name[1] == 'L';
				mReplayIsShuffle = Name[2] == 'S';
			}
		}
		else if (const cRecording *rec = GetRecordingByFileName(FileName)) {
			mReplay = rec;
			mReplayMode = replayNormal;
		}
		else if (strcmp(Name, "DVD") == 0)
			mReplayMode = replayDVD;
		else if (strcmp(Name, "VCD") == 0)
			mReplayMode = replayVCD;
		else if (access(Name, F_OK) == 0)
			mReplayMode = replayMPlayer;
		else if (strncmp(Name, "[image]", 7) == 0)
			mReplayMode = replayImage;
		else if (strncmp(Name, "[cdda]", 6) == 0)
			mReplayMode = replayAudioCd;
		else if (strlen(Name) > 7) {
			int i, n;
			for (i = 0, n = 0; Name[i]; ++i) {
				if (Name[i] == ' ' && Name[i-1] == ',' && ++n == 4)
					break;
			}
			if (Name[i]) { // replaying DVD
				mReplayMode = replayDVD;
			}
		}
	} else {
	  mReplayMode = replayNone;
	  mReplayIsLoop = false;
	  mReplayIsShuffle = false;
	}

#if VDRVERSNUM >= 10703
	// Workaround: Control->FramesPerSecond() not possible because its not const
	mReplayFramesPerSecond = mReplay != NULL ? mReplay->FramesPerSecond() : DEFAULTFRAMESPERSECOND;
#else
	mReplayFramesPerSecond = FRAMESPERSEC;
#endif

	if (mRender != NULL) {
		if (mReplayMode != oldMode)
			mRender->SetDirty();
		mRender->UpdateUnlock();
	}
}

void cText2SkinStatus::Recording(const cDevice *Device, const char *Name,
								 const char *FileName, bool On)
{
	if (mRender != NULL)
		mRender->UpdateLock();

	mRecordingsLock.Lock();
	mRecordings.clear();
	cTimer *t = Timers.First();
	for (; t != NULL; t = Timers.Next(t)) {
		if (t->Recording())
			mRecordings.push_back(t->File());
	}
	mRecordingsLock.Unlock();

	if (mRender != NULL) {
		Dprintf("\nFlushing from cStatus\n\n");
		mRender->Flush(true);
		mRender->UpdateUnlock();
	}
}

void cText2SkinStatus::OsdClear(void)
{
	if (I18nCurrentLanguage() != mLastLanguage) {
		mLastLanguage = I18nCurrentLanguage();
		cxString::Reparse();
	}

	if (mRender != NULL)
		mRender->mMenuScrollbar.total = 0;
}

void cText2SkinStatus::OsdCurrentItem(const char *Text)
{
	if (mRender && Text) {
		// update infos
		cText2SkinRender::tUpdate *u = &mRender->mUpdate;
		//static std::string lastItem;

		//lastItem = u->currentItem;
		u->currentItem = Text;
		u->resetMarquee = true;
		u->foundFirstItem = false;

		// find current item in scrollbar
		if (Text2SkinSetup.MenuScrollbar) {
			cText2SkinRender::tMenuScrollbar *sb = &mRender->mMenuScrollbar;
			for (uint i = 0; i < sb->total; i++) {
				if (sb->items[i] == Text) {
					sb->current = i;
					break;
				}
			}
		}
	}
}

void cText2SkinStatus::OsdItem(const char *Text, int Index)
{
	if (mRender && Text2SkinSetup.MenuScrollbar && Text) {
		uint curr = (uint)Index;
		cText2SkinRender::tMenuScrollbar *sb = &mRender->mMenuScrollbar;

		if (curr < sb->items.size())
			sb->items[curr] = Text;
		else {
			sb->items.push_back(Text);
			sb->total = curr + 1;
		}

		if (curr + 1 > sb->total)
			sb->total = curr + 1;
	}
}

void cText2SkinStatus::UpdateEvents(void)
{
	if (mRender->mUpdate.events) {
		mRender->mUpdate.events = false;

		mEvents.Clear();
		Timers.IncBeingEdited();

		for (cTimer *tim = Timers.First(); tim; tim = Timers.Next(tim)) {
			if (tim->HasFlags(tfActive)) {
				int i = 0;
				cTimer dummy;
				dummy = *tim;

				do {
					mEvents.Add(new tEvent(&dummy));

					if (!dummy.IsSingleEvent()) { // add 4 additional rep. timer
						int j = 0;
						do {
							j++; // just to avoid a endless loop
							dummy.Skip();
							dummy.Matches(); // Refresh start- and end-time
						} while (!dummy.DayMatches(dummy.StartTime()) && (j < 7));
					}

					i++;
				} while (!dummy.IsSingleEvent() && i < 5);
			}
		}

		Timers.DecBeingEdited();
		mEvents.Sort();
	}
}

cxType cText2SkinStatus::GetTokenData(const txToken &Token)
{
	int event = 0;

	switch (Token.Type) {
	case tReplayMode:
		return ReplayNames[mReplayMode];

	case tFrontendSTR:
		return GetFrontendSTR();

	case tFrontendSNR:
		return GetFrontendSNR();

	case tFrontendHasLock:
		return GetFrontendHasLock();

	case tFrontendHasSignal:
		return GetFrontendHasSignal();

	case tCurrentEventsTitle3:
		event++;
	case tCurrentEventsTitle2:
		event++;
	case tCurrentEventsTitle1:
		UpdateEvents();
		return mEvents.Count() > event
		       ? (cxType)mEvents.Get(event)->title.c_str()
		       : (cxType)false;

	case tCurrentEventsStartDateTime3:
		event++;
	case tCurrentEventsStartDateTime2:
		event++;
	case tCurrentEventsStartDateTime1:
		UpdateEvents();
		return mEvents.Count() > event
		       ? (cxType)TimeType(mEvents.Get(event)->startTime, Token.Attrib.Text)
		       : (cxType)false;

	case tCurrentEventsStopDateTime3:
		event++;
	case tCurrentEventsStopDateTime2:
		event++;
	case tCurrentEventsStopDateTime1:
		UpdateEvents();
		return mEvents.Count() > event
		       ? (cxType)TimeType(mEvents.Get(event)->stopTime, Token.Attrib.Text)
		       : (cxType)false;

	case tCurrentEventsChannelNumber3:
		event++;
	case tCurrentEventsChannelNumber2:
		event++;
	case tCurrentEventsChannelNumber1:
		UpdateEvents();
		return mEvents.Count() > event
		       ? (cxType)mEvents.Get(event)->channelNumber
		       : (cxType)false;

	case tCurrentEventsChannelName3:
		event++;
	case tCurrentEventsChannelName2:
		event++;
	case tCurrentEventsChannelName1:
		UpdateEvents();
		return mEvents.Count() > event
		       ? (cxType)mEvents.Get(event)->channelName.c_str()
		       : (cxType)false;

	case tCurrentEventsIsRecording3:
		event++;
	case tCurrentEventsIsRecording2:
		event++;
	case tCurrentEventsIsRecording1:
		UpdateEvents();
		return mEvents.Count() > event
		       ? (cxType)mEvents.Get(event)->isRecording
		       : (cxType)false;

	case tTimerConflicts:
		if (Text2SkinSetup.CheckTimerConflict) {
			if (mRender->mUpdate.timerConflict) {
				Epgsearch_lastconflictinfo_v1_0 conflict;
				mRender->mUpdate.timerConflict = false;

				if (cPluginManager::CallFirstService("Epgsearch-lastconflictinfo-v1.0", &conflict))
					mTimerConflicts = conflict.relevantConflicts;
				else
					mTimerConflicts = 0;
			}
			return mTimerConflicts;
		} else
			return 0;

	case tReplayName:
		return mReplay != NULL
		       ? (cxType)mReplay->Name()
		       : (cxType)false;

	case tReplayDateTime:
		return mReplay != NULL
#if APIVERSNUM >= 10721
		       ? (cxType)TimeType(mReplay->Start(), Token.Attrib.Text)
#else
		       ? (cxType)TimeType(mReplay->start, Token.Attrib.Text)
#endif
		       : (cxType)false;

	case tReplayShortText:
		return mReplay != NULL
		       ? (cxType)mReplay->Info()->ShortText()
		       : (cxType)false;

	case tReplayDescription:
		return mReplay != NULL
		       ? (cxType)mReplay->Info()->Description()
		       : (cxType)false;

	case tReplayLanguageCode:
		if (mReplay) {
			const cComponents *components = mReplay->Info()->Components();
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

	case tReplayLanguageDescription:
		if (mReplay) {
			const cComponents *components = mReplay->Info()->Components();
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

	case tReplayVideoAR:
		if (mReplay) {
			const cComponents *components = mReplay->Info()->Components();
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

	case tCurrentRecording:
		Dprintf("token attrib type is: %d, number: %d\n", Token.Attrib.Type, Token.Attrib.Number);
		if (Token.Attrib.Type == aNumber) {
			return mRecordings.size() > (uint)Token.Attrib.Number
			       ? (cxType)mRecordings[Token.Attrib.Number]
			       : (cxType)false;
		} else if (!mRecordings.empty()) {
			mRecordingsLock.Lock();
			uint now = cTimeMs::Now();
			if (mCurrentRecording >= mRecordings.size())
				mCurrentRecording = mNextRecording = 0;
			if (mNextRecording == 0)
				mNextRecording = now + 2000;
			else if (now >= mNextRecording) {
				mCurrentRecording = (mCurrentRecording + 1) % mRecordings.size();
				mNextRecording = now + 2000;
			}

			uint next = 0;
			if (mRecordings.size() > 1) {
				next = mNextRecording - now;
				Dprintf("next update in %d ms\n", next);
			}

			cxType res = mRecordings[mCurrentRecording];
			mRecordingsLock.Unlock();
			res.SetUpdate(next);
			return res;
		}
		return false;

	case tReplayIsLoop:
		return mReplayIsLoop;

	case tReplayIsShuffle:
		return mReplayIsShuffle;

	default:
		break;
	};

	return false;
}
