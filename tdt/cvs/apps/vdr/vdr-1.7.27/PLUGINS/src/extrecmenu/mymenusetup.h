#include <vdr/menu.h>

#if VDRVERSNUM >= 10713
extern cNestedItemList RecordingDirCommands;
#else
extern cCommands RecordingDirCommands;
#endif

#define STRN0CPY(dst, src) \
  do { \
    strn0cpy(dst, src, sizeof(dst)); \
  } while(0)
/*
#define STRN0CPYLOG(dst, src, Name) \
  do { \
    strn0cpy(dst, src, sizeof(dst)); \
    if(strlen(src) >= sizeof(dst)) \
      LOGMSG("WARNING: Setting %s truncated to %s !", Name, dst); \
  } while(0)
*/

#define MAX_RECLIST_COLUMNS 4

#define COLTYPE_NONE		0
#define COLTYPE_BLANK		1
#define COLTYPE_DATE		2
#define COLTYPE_TIME		3
#define COLTYPE_DATETIME	4
#define COLTYPE_LENGTH		5
#define COLTYPE_RATING		6
#define COLTYPE_FILE		7
#define COLTYPE_FILETHENCOMMAND	8
#define MAX_COLTYPES		9



static const char * const RecsPerDir_texts[] = {
  trNOOP("0-9"),
  trNOOP("0-99"),
  trNOOP("0-999"),
  trNOOP("0-9999"),
  trNOOP("0-99999"),
  NULL};


typedef struct {
  int Type;
  char Name[64];
  int Width;
  int Align;
  char Op1[1024];
  char Op2[1024];
} RecListColumnType;



class mySetup
{
 public:
  mySetup();
  RecListColumnType RecListColumn[MAX_RECLIST_COLUMNS];
  int HideMainMenuEntry;
  int ReplaceOrgRecMenu;
  int PatchNew;
  int ShowNewRecs;
  int RecsPerDir;
  int DescendSorting;
  int GoLastReplayed;
  int ReturnToPlugin;
  int LimitBandwidth;
  int UseVDRsRecInfoMenu;
  int PatchFont;
  int FileSystemFreeMB;
  int UseCutterQueue;
};

extern mySetup mysetup;

class myMenuSetup:public cMenuSetupPage
{
 private:
  const char *sortingtypetexts[2];
  RecListColumnType reclistcolumn[MAX_RECLIST_COLUMNS];
  int hidemainmenuentry;
  int replaceorgrecmenu;
  int patchnew;
  int shownewrecs;
  int recsperdir;
  int descendsorting;
  int golastreplayed;
  int returntoplugin;
  int limitbandwidth;
  int usevdrsrecinfomenu;
  int patchfont;
  int filesystemfreemb;
  int usecutterqueue;
 protected:
  virtual void Store();
 public:
  myMenuSetup();
//  virtual ~myMenuSetup();
  virtual eOSState ProcessKey(eKeys Key);
};

class myMenuSetupColumns:public cOsdMenu
{
 private:
  RecListColumnType * preclistcolumns;
  const char* ColumnType_descriptions[9];
  const char* AlignmentType_names[3];
 public:
  myMenuSetupColumns(RecListColumnType *prlcs);
//  virtual ~myMenuSetupColumns();
  virtual void Set();
  virtual eOSState ProcessKey(eKeys Key);
};

char* IndentMenuItem(const char*, int indentions=1);

static inline cString Label_SubMenu(const char *Label) {
  return cString::sprintf("%s ...", Label);
}
static inline cOsdItem *SubMenuItem(const char *Label, eOSState state) {
  return new cOsdItem(Label_SubMenu(Label), state);
}
