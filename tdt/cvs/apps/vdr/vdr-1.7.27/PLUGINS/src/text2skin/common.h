//								-*- c++ -*-

#ifndef VDR_TEXT2SKIN_COMMON_H
#define VDR_TEXT2SKIN_COMMON_H

#include "xml/string.h"
#include <string>
#include <vdr/osd.h>
#include <vdr/config.h>
#include <vdr/epg.h>

#if defined(DEBUG) || defined(BENCH)
#	ifdef DEBUG
#		define Dprintf(x...) fprintf(stderr, x)
#	else
#		define Dprintf(x...)
#	endif
#	define __STDC_FORMAT_MACROS
#	define Dbench(x) uint64_t bench_##x = cTimeMs::Now()
#	define Ddiff(t,x) fprintf(stderr, "%s took %"PRIu64" ms\n", t, cTimeMs::Now() - bench_##x)
#else
#	define Dprintf(x...)
#	define Dbench(x)
#	define Ddiff(t,x)
#endif

/* Check if a channel is a radio station. */
#define ISRADIO(x) ((x)->Vpid()==0||(x)->Vpid()==1||(x)->Vpid()==0x1fff)

// class forwards

class cMarks;
class cChannel;
class cRecording;

// helper functions

const std::string &SkinPath(void);
const char *ChannelNumber(const cChannel *Channel, int Number);
const char *ChannelName(const cChannel *Channel, int Number);
const char *ChannelShortName(const cChannel *Channel, int Number);
const char *EventType(uint Number);
//const char *ChannelBouquet(const cChannel *Channel, int Number);

bool StoppedTimer(const char *Name);
const cRecording *GetRecordingByName(const char *Name);
const cRecording *GetRecordingByFileName(const char *FileName);
int GetFrontendSTR(void); // Signal strength [%]
int GetFrontendSNR(void); // Signal to Noise ratio [%]
bool GetFrontendHasLock(void);
bool GetFrontendHasSignal(void);
std::string AddExtInfoToDescription(const char *Title, const char *ShortText, const char *Description, const char *Aux = NULL, bool StripAux = false);
int GetRecordingSize(const char *FileName); // [MB]
int GetRecordingLength(const char *FileName, double FramesPerSecond, bool IsPesRecording); // [min]
int GetRecordingCuttedLength(const char *FileName, double FramesPerSecond, bool IsPesRecording); // [min]

cxType TimeType(time_t Time, const std::string &Format);
cxType DurationType(uint Index, double FramesPerSecond, const std::string &Format);
cxType DurationType(int Seconds, const std::string &Format, int Frame = 0);

bool ParseVar(const char *Text, const char *Name, std::string &Value);
bool ParseVar(const char *Text, const char *Name, tColor *Value);

void SkipQuotes(std::string &Value);
std::string FitToWidth(std::string &Line, uint Width);
std::string FitToWidth(std::stringstream &Line, uint Width);
std::string StripXmlTag(std::string &Line, const char *Tag);

// Data structure for service "Epgsearch-searchresults-v1.0"
struct Epgsearch_searchresults_v1_0 {
// in
	char* query;               // search term
	int mode;                  // search mode (0=phrase, 1=and, 2=or, 3=regular expression)
	int channelNr;             // channel number to search in (0=any)
	bool useTitle;             // search in title
	bool useSubTitle;          // search in subtitle
	bool useDescription;       // search in description
// out

	class cServiceSearchResult : public cListObject {
	public:
		const cEvent* event;
		cServiceSearchResult(const cEvent* Event) : event(Event) {}
	};

	cList<cServiceSearchResult>* pResultList;   // pointer to the results
};

// Data structure for service "Epgsearch-lastconflictinfo-v1.0"
struct Epgsearch_lastconflictinfo_v1_0 {
// in
// out
	time_t nextConflict;       // next conflict date, 0 if none
	int relevantConflicts;     // number of relevant conflicts
	int totalConflicts;        // total number of conflicts
};

#endif // VDR_TEXT2SKIN_COMMON_H
