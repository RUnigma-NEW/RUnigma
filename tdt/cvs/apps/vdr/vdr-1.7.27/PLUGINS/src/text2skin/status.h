//								-*- c++ -*-

#ifndef VDR_TEXT2SKIN_STATUS_H
#define VDR_TEXT2SKIN_STATUS_H

#include "common.h"
#include <vdr/status.h>

class cText2SkinRender;

class cText2SkinStatus: public cStatus {
public:
	enum eReplayMode {
		replayNone,
		replayNormal,
		replayMP3,
		replayMPlayer,
		replayDVD,
		replayVCD,
		replayImage,
		replayAudioCd,

#define __REPLAY_COUNT__ (cText2SkinStatus::replayAudioCd+1)
	};

	typedef std::string tRecordingInfo;
	typedef std::vector<tRecordingInfo> tRecordings;

	struct tEvent : public cListObject {
		time_t startTime;
		time_t stopTime;
		int channelNumber;
		std::string channelName;
		int priority;
		bool isRecording;
		std::string title;

		tEvent(cTimer *timer) :
			startTime(timer->StartTime()),
			stopTime(timer->StopTime()),
			channelNumber(timer->Channel()->Number()),
			channelName(timer->Channel()->Name()),
			priority(timer->Priority()),
			isRecording(timer->Recording()),
			title(timer->File()) {}

		virtual int Compare(const cListObject &listObj) const {
			tEvent *e = (tEvent *)&listObj;
			int r = startTime - e->startTime;
			if (r == 0)
				r = e->priority - priority;
			return r;
		}
	};

	typedef std::vector<tEvent> tEvents;

private:
	void UpdateEvents(void);

	cText2SkinRender *mRender;
	eReplayMode       mReplayMode;
	bool              mReplayIsLoop;
	bool              mReplayIsShuffle;
	tRecordings       mRecordings;
	const cRecording *mReplay;
	cList<tEvent>     mEvents;
	cMutex            mRecordingsLock;
	uint              mCurrentRecording;
	uint              mNextRecording;
	int               mLastLanguage;
	int               mTimerConflicts;
	double            mReplayFramesPerSecond;

protected:
	virtual void Replaying(const cControl *Control, const char *Name,
						   const char *FileName, bool On);
	virtual void Recording(const cDevice *Device, const char *Name,
						   const char *FileName, bool On);
	virtual void OsdClear(void);

	virtual void OsdCurrentItem(const char *Text);
	virtual void OsdItem(const char *Text, int Index);

public:
	cText2SkinStatus(void);

	void SetLanguage(int Language) { mLastLanguage = Language; }
	void SetRender(cText2SkinRender *Render);

	cxType GetTokenData(const txToken &Token);

	eReplayMode ReplayMode(void) const { return mReplayMode; }

	double ReplayFramesPerSecond(void) const { return mReplayFramesPerSecond; }
};

extern cText2SkinStatus Text2SkinStatus;

#endif // VDR_TEXT2SKIN_STATUS_H
