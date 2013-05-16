//								-*- c++ -*-

#include "xml/string.h"
#include "render.h"

static const char *Tokens[__COUNT_TOKEN__] = {
	"DateTime", "FreeDiskSpace", "UsedDiskSpace", "TotalDiskSpace", "VideoSizeWidth", "VideoSizeHeight", "AudioTrack", "AudioChannel",

	// Channel Display
	"ChannelNumber", "ChannelName", "ChannelShortName", "ChannelBouquet", "ChannelPortal",
	"ChannelSource", "ChannelID", "PresentStartDateTime", "PresentVPSDateTime",
	"CurrentEventsTitle1", "CurrentEventsStartDateTime1", "CurrentEventsStopDateTime1", "CurrentEventsChannelNumber1", "CurrentEventsChannelName1", "CurrentEventsIsRecording1",
	"CurrentEventsTitle2", "CurrentEventsStartDateTime2", "CurrentEventsStopDateTime2", "CurrentEventsChannelNumber2", "CurrentEventsChannelName2", "CurrentEventsIsRecording2",
	"CurrentEventsTitle3", "CurrentEventsStartDateTime3", "CurrentEventsStopDateTime3", "CurrentEventsChannelNumber3", "CurrentEventsChannelName3", "CurrentEventsIsRecording3",
	"TimerConflicts",
	"PresentEndDateTime", "PresentDuration", "PresentProgress", "PresentRemaining",
	"PresentLanguageCode", "PresentLanguageDescription", "PresentVideoAR", "PresentEventID",
	"PresentTitle", "PresentShortText", "PresentDescription", "FollowingStartDateTime",
	"FollowingVPSDateTime", "FollowingEndDateTime", "FollowingDuration",
	"FollowingTitle", "FollowingShortText", "FollowingDescription", "Language",
	"HasTeletext", "ChannelHasTeletext", "HasMultilang", "ChannelHasMultilang", "HasDolby",
	"ChannelHasDolby", "IsEncrypted", "ChannelIsEncrypted", "IsRadio", "ChannelIsRadio",
	"IsRecording", "CurrentRecording", "HasVPS", "HasTimer", "IsRunning", "ChannelHasVPS",
	"PresentHasTimer", "PresentIsRunning", "PresentHasVPS", "FollowingHasTimer",
	"FollowingIsRunning", "FollowingHasVPS",

	// Volume Display
	"VolumeCurrent", "VolumeTotal", "IsMute", "VolumeIsMute",

	// Message Display
	"Message", "MessageStatus", "MessageInfo", "MessageWarning",
	"MessageError",

	// Replay Display
	"ReplayTitle", "ReplayPositionIndex", "ReplayDurationIndex", "ReplayPrompt",
	"ReplayName", "ReplayDateTime", "ReplayShortText", "ReplayDescription",
	"ReplayLanguageCode", "ReplayLanguageDescription", "ReplayVideoAR",
	"IsPlaying", "ReplayIsPlaying", "IsFastForward", "ReplayIsFastForward", "IsFastRewind",
	"ReplayIsFastRewind", "IsSlowForward", "ReplayIsSlowForward", "IsSlowRewind",
	"ReplayIsSlowRewind", "IsPausing", "ReplayIsPausing",
	"ReplayPosition", "ReplayDuration", "ReplayRemaining", "ReplayMode", "ReplayIsShuffle",
	"ReplayIsLoop",

	// Menu Page
	"MenuTitle", "MenuGroup", "IsMenuGroup", "MenuItem", "IsMenuItem", "MenuCurrent",
	"IsMenuCurrent", "MenuText", "RecordingName", "RecordingFilename", "RecordingDateTime", "RecordingTitle",
	"RecordingShortText", "RecordingDescription", "RecordingLanguageCode",
	"FrontendSTR", "FrontendSNR", "FrontendHasLock", "FrontendHasSignal", "RecordingPriority", "RecordingLifetime",
	"RecordingVideoAR", "RecordingSize", "RecordingLength", "RecordingCuttedLength", "OsdWidth", "OsdHeight",
	"RecordingLanguageDescription", "ButtonRed", "ButtonGreen",
	"ButtonYellow", "ButtonBlue", "CanScrollUp", "CanScrollDown"
};

std::string txToken::Token(const txToken &Token)
{
	std::string result = (std::string)"{" + Tokens[Token.Type];
	//if (Token.Attrib.length() > 0)
	//	result += ":" + Token.Attrib;
	result += "}";

	return result;
}

cxString::tStringList cxString::mStrings;

cxString::cxString(cxObject *Parent, bool Translate):
		mObject(Parent),
		mSkin(Parent->Skin()),
		mTranslate(Translate)
{
	mStrings.push_back(this);
}

cxString::~cxString()
{
	tStringList::iterator it = mStrings.begin();
	for (; it != mStrings.end(); ++it) {
		if ((*it) == this) {
			mStrings.erase(it);
			break;
		}
	}
}

void cxString::Reparse(void)
{
	tStringList::iterator it = mStrings.begin();
	for (; it != mStrings.end(); ++it) {
		if ((*it)->mTranslate && (*it)->mText.length() > 0)
			(*it)->Parse((*it)->mOriginal, true);
	}
}

bool cxString::Parse(const std::string &Text, bool Translate)
{
	std::string trans = Translate ? mSkin->Translate(Text) : Text;
	const char *text = trans.c_str();
	const char *ptr = text, *last = text;
	bool inToken = false;
	bool inAttrib = false;
	int offset = 0;

	Dprintf("parsing: %s\n", Text.c_str());
	mOriginal = Text;
	mText = "";
	mTokens.clear();

	for (; *ptr; ++ptr) {
		if (inToken && *ptr == '\\') {
			if (*(ptr + 1) == '\0') {
				esyslog("ERROR: Stray \\ in token attribute\n");
				return false;
			}

			++ptr;
			continue;
		}
		else if (*ptr == '{') {
			if (inToken) {
				esyslog("ERROR: Unexpected '{' in token");
				return false;
			}

			mText.append(last, ptr - last);
			last = ptr + 1;
			inToken = true;
		}
		else if (*ptr == '}' || (inToken && *ptr == ':')) {
			if (!inToken) {
				esyslog("ERROR: Unexpected '}' outside of token");
				return false;
			}

			if (inAttrib) {
				if (*ptr == ':') {
					esyslog("ERROR: Unexpected ':' inside of token attribute");
					return false;
				}

				int pos = -1;
				std::string attr;
				attr.assign(last, ptr - last);
				while ((pos = attr.find('\\', pos + 1)) != -1) {
					switch (attr[pos + 1]) {
					case 'n':
						attr.replace(pos, 2, "\n");
						break;

					default:
						attr.erase(pos, 1);
					}
				}

				txToken &lastToken = mTokens[mTokens.size() - 1];
				if (attr == "clean")
					lastToken.Attrib = aClean;
				else if (attr == "rest")
					lastToken.Attrib = aRest;
				else {
					char *end;
					int n = strtol(attr.c_str(), &end, 10);
					Dprintf("attr: %s, n: %d, end: |%s|\n", attr.c_str(), n, end);
					if (end != attr.c_str() && *end == '\0') {
						Dprintf("a number\n");
						lastToken.Attrib = n;
					} else
						lastToken.Attrib = attr;
				}

				inAttrib = false;
				inToken = false;
			} else {
				int i;
				for (i = 0; i < (int)__COUNT_TOKEN__; ++i) {
					if ((size_t)(ptr - last) == strlen(Tokens[i])
							&& memcmp(last, Tokens[i], ptr - last) == 0) {
						txToken token((exToken)i, offset, "");
						mTokens.push_back(token);
						break;
					}
				}

				if (i == (int)__COUNT_TOKEN__) {
					esyslog("ERROR: Unexpected token {%.*s}", (int)(ptr - last), last);
					return false;
				}

				if (*ptr == ':')
					inAttrib = true;
				else
					inToken = false;
			}

			last = ptr + 1;
		}
		else if (!inToken)
			++offset;
	}

	if (inToken) {
		esyslog("ERROR: Expecting '}' in token");
		return false;
	}

	mText.append(last, ptr - last);

	if (mTranslate && !Translate && mText.length() > 0)
		Parse(Text, true);
	return true;
}

cxType cxString::Evaluate(void) const
{
	std::string result;
	int offset = 0;

	if (mText.length() == 0 && mTokens.size() == 1)
		return cText2SkinRender::GetToken(mTokens[0]);

	for (uint i = 0; i < mTokens.size(); ++i) {
		result.append(mText.c_str() + offset, mTokens[i].Offset - offset);
		result.append(cText2SkinRender::GetToken(mTokens[i]));
		offset = mTokens[i].Offset;
	}
	result.append(mText.c_str() + offset);
	return result;
}

