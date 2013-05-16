//								-*- c++ -*-

#ifndef VDR_TEXT2SKIN_XML_STRING_H
#define VDR_TEXT2SKIN_XML_STRING_H

#include "xml/type.h"
#include <string>
#include <vector>

enum exToken {
	tDateTime,
	tFreeDiskSpace,
	tUsedDiskSpace,
	tTotalDiskSpace,
	tVideoSizeWidth,
	tVideoSizeHeight,
	tAudioTrack,
	tAudioChannel,

	// Channel Display
	tChannelNumber,
	tChannelName,
	tChannelShortName,
	tChannelBouquet,
	tChannelPortal,
	tChannelSource,
	tTChannelID, // (name clash)
	// next 9 also in Menu
	tPresentStartDateTime,
	tPresentVPSDateTime,
	tCurrentEventsTitle1,
	tCurrentEventsStartDateTime1,
	tCurrentEventsStopDateTime1,
	tCurrentEventsChannelNumber1,
	tCurrentEventsChannelName1,
	tCurrentEventsIsRecording1,
	tCurrentEventsTitle2,
	tCurrentEventsStartDateTime2,
	tCurrentEventsStopDateTime2,
	tCurrentEventsChannelNumber2,
	tCurrentEventsChannelName2,
	tCurrentEventsIsRecording2,
	tCurrentEventsTitle3,
	tCurrentEventsStartDateTime3,
	tCurrentEventsStopDateTime3,
	tCurrentEventsChannelNumber3,
	tCurrentEventsChannelName3,
	tCurrentEventsIsRecording3,
	tTimerConflicts,
	tPresentEndDateTime,
	tPresentDuration,
	tPresentProgress,
	tPresentRemaining,
	tPresentLanguageCode,
	tPresentLanguageDescription,
	tPresentVideoAR,
	tPresentEventID,
	tPresentTitle,
	tPresentShortText,
	tPresentDescription,
	tFollowingStartDateTime,
	tFollowingVPSDateTime,
	tFollowingEndDateTime,
	tFollowingDuration,
	tFollowingTitle,
	tFollowingShortText,
	tFollowingDescription,
	tLanguage,
	tHasTeletext,
	tChannelHasTeletext, // alias
	tHasMultilang,
	tChannelHasMultilang, // alias
	tHasDolby,
	tChannelHasDolby, // alias
	tIsEncrypted,
	tChannelIsEncrypted, // alias
	tIsRadio,
	tChannelIsRadio, // alias
	// next 2 also in all other displays
	tIsRecording,
	tCurrentRecording,
	// next 3 also in Menu
	tHasVPS,
	tHasTimer,
	tIsRunning,
	tChannelHasVPS, // alias
	// next 3 also in Menu
	tPresentHasTimer, // alias
	tPresentIsRunning, // alias
	tPresentHasVPS,
	tFollowingHasTimer,
	tFollowingIsRunning,
	tFollowingHasVPS,

	// VolumeDisplay
	tVolumeCurrent,
	tVolumeTotal,
	tIsMute,
	tVolumeIsMute, // alias

	// Message Display (also in all other displays)
	tMessage,
	tMessageStatus,
	tMessageInfo,
	tMessageWarning,
	tMessageError,

	// Replay Display
	tReplayTitle,
	tReplayPositionIndex,
	tReplayDurationIndex,
	tReplayPrompt,
	tReplayName,
	tReplayDateTime,
	tReplayShortText,
	tReplayDescription,
	tReplayLanguageCode,
	tReplayLanguageDescription,
	tReplayVideoAR,
	tIsPlaying,
	tReplayIsPlaying, // alias
	tIsFastForward,
	tReplayIsFastForward, // alias
	tIsFastRewind,
	tReplayIsFastRewind, // alias
	tIsSlowForward,
	tReplayIsSlowForward, // alias
	tIsSlowRewind,
	tReplayIsSlowRewind, // alias
	tIsPausing,
	tReplayIsPausing, // alias
	tReplayPosition,
	tReplayDuration,
	tReplayRemaining,
	tReplayMode,
	tReplayIsShuffle,
	tReplayIsLoop,

	// Menu Page
	tMenuTitle,
	tMenuGroup,
	tIsMenuGroup,
	tMenuItem,
	tIsMenuItem,
	tMenuCurrent,
	tIsMenuCurrent,
	tMenuText,
	// Recordings Page
	tRecordingName,
	tRecordingFilename,
	tRecordingDateTime,
	tRecordingTitle,
	tRecordingShortText,
	tRecordingDescription,
	tRecordingLanguageCode,
	tFrontendSTR,
	tFrontendSNR,
	tFrontendHasLock,
	tFrontendHasSignal,
	tRecordingPriority,
	tRecordingLifetime,
	tRecordingVideoAR,
	tRecordingSize,
	tRecordingLength,
	tRecordingCuttedLength,
	tOsdWidth,
	tOsdHeight,
	tRecordingLanguageDescription,
	// next four also in Channel and Replay display (if supported by vdr/plugin)
	tButtonRed,
	tButtonGreen,
	tButtonYellow,
	tButtonBlue,
	tCanScrollUp,
	tCanScrollDown,

#define __COUNT_TOKEN__ (tCanScrollDown + 1)
};

enum exAttrib {
	aNone,
	aNumber,
	aString,
	aClean,
	aRest

#define __COUNT_ATTRIB__ (aRest + 1)
};

struct txAttrib {
	exAttrib    Type;
	std::string Text;
	int         Number;

	txAttrib(const std::string &a): Type(aString), Text(a), Number(0) {}
	txAttrib(int n): Type(aNumber), Text(""), Number(n) {}
	txAttrib(exAttrib t): Type(t), Text(""), Number(0) {}
	txAttrib(void): Type(aNone), Text(""), Number(0) {}

	friend bool operator== (const txAttrib &A, const txAttrib &B);
	friend bool operator<  (const txAttrib &A, const txAttrib &B);
};

inline bool operator== (const txAttrib &A, const txAttrib &B)
{
	return A.Type == B.Type
	    && A.Text == B.Text
	    && A.Number == B.Number;
}

inline bool operator<  (const txAttrib &A, const txAttrib &B)
{
	return A.Type == B.Type
	       ? A.Text == B.Text
		     ? A.Number < B.Number
	         : A.Text < B.Text
	       : A.Type < B.Type;
}

struct txToken {
	exToken     Type;
	uint        Offset;
	txAttrib    Attrib;
	int         Index;
	int         Tab;

	txToken(void): Index(-1), Tab(-1) {}
	txToken(exToken t, uint o, const std::string &a):
		Type(t), Offset(o), Attrib(a), Index(-1), Tab(-1) {}

	friend bool operator< (const txToken &A, const txToken &B);

	static std::string Token(const txToken &Token);
};

inline bool operator< (const txToken &A, const txToken &B)
{
	return A.Type == B.Type
	       ? A.Attrib == B.Attrib
	         ? A.Index == B.Index
	           ? A.Tab < B.Tab
	           : A.Index < B.Index
	         : A.Attrib < B.Attrib
	       : A.Type < B.Type;
}

class cxObject;
class cxSkin;

class cxString {
private:
	typedef std::vector<cxString*> tStringList;
	static tStringList mStrings;

	cxObject            *mObject;
	cxSkin              *mSkin;
	std::string          mText;
	std::string          mOriginal;
	std::vector<txToken> mTokens;
	bool                 mTranslate;

public:
	static void Reparse(void);

	cxString(cxObject *Parent, bool Translate);
	~cxString();

	bool Parse(const std::string &Text, bool Translate = false);
	cxType Evaluate(void) const;

	void SetListIndex(uint Index, int Tab);

	cxObject *Object(void) const { return mObject; }
	cxSkin   *Skin(void)   const { return mSkin; }
};

inline void cxString::SetListIndex(uint Index, int Tab)
{
	for (uint i = 0; i < mTokens.size(); ++i) {
		mTokens[i].Index = Index;
		mTokens[i].Tab = Tab;
	}
}

#endif // VDR_TEXT2SKIN_XML_STRING_H
