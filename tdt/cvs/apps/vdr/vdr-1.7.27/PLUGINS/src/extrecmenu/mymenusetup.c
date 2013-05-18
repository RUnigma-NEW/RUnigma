/*
 * See the README file for copyright information and how to reach the author.
 */

#include <stdarg.h>
#include <vdr/menu.h>
#include "mymenusetup.h"

typedef struct {
  int Width;
  int Align;
} RecListDefaultType;

RecListDefaultType recListDefaultValues[MAX_COLTYPES] = {
  { 0,0},
  { 5,0},
  { 8,0},
  { 5,0},
  {14,0},
  { 4,2},
  { 7,0},
  { 5,0},
  { 5,0}
};


#if VDRVERSNUM > 10713
cNestedItemList RecordingDirCommands;
#else
cCommands RecordingDirCommands;
#endif

mySetup::mySetup()
{
  mysetup.HideMainMenuEntry=0;
  mysetup.PatchNew=1;
  mysetup.ReplaceOrgRecMenu=0;

  for(int i=0; i<MAX_RECLIST_COLUMNS; i++)
    mysetup.RecListColumn[0].Type = COLTYPE_NONE;

  mysetup.RecListColumn[0].Type = COLTYPE_DATE;
  STRN0CPY(mysetup.RecListColumn[0].Name, "");
  mysetup.RecListColumn[0].Width = recListDefaultValues[mysetup.RecListColumn[0].Type].Width;
  mysetup.RecListColumn[0].Align = recListDefaultValues[mysetup.RecListColumn[0].Type].Align;
  STRN0CPY(mysetup.RecListColumn[0].Op1, "");
  STRN0CPY(mysetup.RecListColumn[0].Op2, "");

  mysetup.RecListColumn[1].Type = COLTYPE_TIME;
  STRN0CPY(mysetup.RecListColumn[1].Name, "");
  mysetup.RecListColumn[1].Width = recListDefaultValues[mysetup.RecListColumn[1].Type].Width;
  mysetup.RecListColumn[1].Align = recListDefaultValues[mysetup.RecListColumn[1].Type].Align;
  STRN0CPY(mysetup.RecListColumn[1].Op1, "");
  STRN0CPY(mysetup.RecListColumn[1].Op2, "");

  mysetup.ShowNewRecs=1;
  mysetup.RecsPerDir=2;
  mysetup.DescendSorting=0;
  mysetup.GoLastReplayed=0;
  mysetup.ReturnToPlugin=1;
  mysetup.LimitBandwidth=0;
  mysetup.UseVDRsRecInfoMenu=0;
  mysetup.PatchFont=1;
  mysetup.FileSystemFreeMB=1;
  mysetup.UseCutterQueue=1;
}

mySetup mysetup;

/******************** myMenuSetup ********************/
myMenuSetup::myMenuSetup()
{
  SetCols(45);

  hidemainmenuentry=mysetup.HideMainMenuEntry;
  patchnew=mysetup.PatchNew;
  replaceorgrecmenu=mysetup.ReplaceOrgRecMenu;

  for(int i=0; i<MAX_RECLIST_COLUMNS; i++) {
    reclistcolumn[i].Type         = mysetup.RecListColumn[i].Type;
    STRN0CPY(reclistcolumn[i].Name, mysetup.RecListColumn[i].Name);
    reclistcolumn[i].Width        = mysetup.RecListColumn[i].Width;
    reclistcolumn[i].Align       = mysetup.RecListColumn[i].Align;
    STRN0CPY(reclistcolumn[i].Op1 , mysetup.RecListColumn[i].Op1);
    STRN0CPY(reclistcolumn[i].Op2 , mysetup.RecListColumn[i].Op2);
  }
  shownewrecs=mysetup.ShowNewRecs;
  recsperdir=mysetup.RecsPerDir;
  descendsorting=mysetup.DescendSorting;
  golastreplayed=mysetup.GoLastReplayed;
  returntoplugin=mysetup.ReturnToPlugin;
  limitbandwidth=mysetup.LimitBandwidth;
  usevdrsrecinfomenu=mysetup.UseVDRsRecInfoMenu;
  patchfont=mysetup.PatchFont;
  filesystemfreemb=mysetup.FileSystemFreeMB;
  usecutterqueue=mysetup.UseCutterQueue;

  sortingtypetexts[0]=tr("ascending");
  sortingtypetexts[1]=tr("descending");

  Add(new cMenuEditBoolItem(tr("Show nr. of new recordings of a directory"),&shownewrecs));
  Add(new cMenuEditStraItem(tr("Maximum number of recordings per directory"), &recsperdir, 5, RecsPerDir_texts));
  Add(SubMenuItem(tr("Items to show in recording list"), osUser1)); 
  Add(new cMenuEditBoolItem(tr("Show alternative to new marker"),&patchnew));
  Add(new cMenuEditBoolItem(tr("Show free disk space for each file system"),&filesystemfreemb));
  Add(new cMenuEditStraItem(tr("Sorting"),&descendsorting,2,sortingtypetexts));
  Add(new cMenuEditBoolItem(tr("Hide main menu entry"),&hidemainmenuentry));
#ifdef MAINMENUHOOKSVERSNUM
  Add(new cMenuEditBoolItem(tr("Replace original recordings menu"),&replaceorgrecmenu));
#endif
  Add(new cMenuEditBoolItem(tr("Jump to last replayed recording"),&golastreplayed));
  Add(new cMenuEditBoolItem(tr("Call plugin after playback"),&returntoplugin));
  Add(new cMenuEditBoolItem(tr("Limit bandwidth for move recordings"),&limitbandwidth));
  Add(new cMenuEditBoolItem(tr("Use VDR's recording info menu"),&usevdrsrecinfomenu));
  Add(new cMenuEditBoolItem(tr("Use cutter queue"),&usecutterqueue));
}

void myMenuSetup::Store()
{
  SetupStore("HideMainMenuEntry",mysetup.HideMainMenuEntry=hidemainmenuentry);
  SetupStore("PatchNew",mysetup.PatchNew=patchnew);
  SetupStore("ReplaceOrgRecMenu",mysetup.ReplaceOrgRecMenu=replaceorgrecmenu);

  char varname[16];
  char* tmp=NULL;
  for(int i=0; i<MAX_RECLIST_COLUMNS; i++) {
    if(asprintf(&tmp,"type=%d,name=%s,width=%d,align=%d,op1=%s,op2=%s",
                      reclistcolumn[i].Type,
                      reclistcolumn[i].Name && (reclistcolumn[i].Type==COLTYPE_FILE || reclistcolumn[i].Type==COLTYPE_FILETHENCOMMAND) ? reclistcolumn[i].Name : "",
                      reclistcolumn[i].Width,
                      reclistcolumn[i].Align,
                      reclistcolumn[i].Op1 && (reclistcolumn[i].Type==COLTYPE_FILE || reclistcolumn[i].Type==COLTYPE_FILETHENCOMMAND) ? reclistcolumn[i].Op1 : "",
                      reclistcolumn[i].Op2 && reclistcolumn[i].Type==COLTYPE_FILETHENCOMMAND ? reclistcolumn[i].Op2 : "") != -1) {
      mysetup.RecListColumn[i].Type         = reclistcolumn[i].Type;
      STRN0CPY(mysetup.RecListColumn[i].Name, reclistcolumn[i].Name);
      mysetup.RecListColumn[i].Width        = reclistcolumn[i].Width;
      mysetup.RecListColumn[i].Align        = reclistcolumn[i].Align;
      STRN0CPY(mysetup.RecListColumn[i].Op1,  reclistcolumn[i].Op1);
      STRN0CPY(mysetup.RecListColumn[i].Op2,  reclistcolumn[i].Op2);

      snprintf(varname, 16, "RecListColumn.%d", i);
      SetupStore(varname, tmp);
      free(tmp);
    }
  }
  SetupStore("ShowNewRecs",mysetup.ShowNewRecs=shownewrecs);
  SetupStore("RecsPerDir",mysetup.RecsPerDir=recsperdir);
  SetupStore("DescendSorting",mysetup.DescendSorting=descendsorting);
  SetupStore("GoLastReplayed",mysetup.GoLastReplayed=golastreplayed);
  SetupStore("ReturnToPlugin",mysetup.ReturnToPlugin=returntoplugin);
  SetupStore("LimitBandwidth",mysetup.LimitBandwidth=limitbandwidth);
  SetupStore("UseVDRsRecInfoMenu",mysetup.UseVDRsRecInfoMenu=usevdrsrecinfomenu);
  SetupStore("PatchFont",mysetup.PatchFont=patchfont);
  SetupStore("FileSystemFreeMB",mysetup.FileSystemFreeMB=filesystemfreemb);
  SetupStore("UseCutterQueue",mysetup.UseCutterQueue=usecutterqueue);
}

eOSState myMenuSetup::ProcessKey(eKeys Key) {
  eOSState state = cMenuSetupPage::ProcessKey(Key);

  switch (state) {
    case osUser1:
      return AddSubMenu(new myMenuSetupColumns(&reclistcolumn[0]));
    default: ;
  }
  return state;
}


/******************** myMenuSetupColumns ********************/
myMenuSetupColumns::myMenuSetupColumns(RecListColumnType *prlcs) : cOsdMenu(tr("Items to show in recording list"), 4) {
  preclistcolumns = prlcs;

  ColumnType_descriptions[0] = tr("--none--");
  ColumnType_descriptions[1] = tr("Blank");
  ColumnType_descriptions[2] = tr("Date of Recording");
  ColumnType_descriptions[3] = tr("Time of Recording");
  ColumnType_descriptions[4] = tr("Date and Time of Recording");
  ColumnType_descriptions[5] = tr("Length of Recording");
  ColumnType_descriptions[6] = tr("Rating of Recording");
  ColumnType_descriptions[7] = tr("Content of File");
  ColumnType_descriptions[8] = tr("Content of File, then Result of a Command");

  AlignmentType_names[0] = tr("left");
  AlignmentType_names[1] = tr("center");
  AlignmentType_names[2] = tr("right");

  Set();
}

void myMenuSetupColumns::Set() {
  SetCols(30);

  // build up menu entries
//  SetHelp(tr("Edit"), NULL, NULL, NULL);  //SetHelp(tr("Edit"), NULL, tr("Move Up"), tr("Move down"));

  // save current postion
  int current = Current();
  // clear entries, if any
  Clear();


  cOsdItem *sItem;
  char *itemStr;
  int ret;

  ret = asprintf(&itemStr, "%s\t%s", tr("Icon"), tr("(fixed to the first position)"));
  if (ret <= 0) {
    esyslog("Error allocating linebuffer for cOsdItem.");
    return;
  }
  sItem = new cOsdItem(itemStr);
  sItem->SetSelectable(false);
  Add(sItem);


  // build up setup menu
  for(int i=0; i<MAX_RECLIST_COLUMNS; i++) {

    ret = asprintf(&itemStr, "%s %i", tr("Item"),i+1);
    if (ret <= 0) {
      esyslog("Error allocating linebuffer for cOsdItem.");
      return;
    }

    Add(new cMenuEditStraItem(itemStr, &(preclistcolumns[i].Type), MAX_COLTYPES, ColumnType_descriptions));

    switch (preclistcolumns[i].Type) {
    case COLTYPE_NONE:
      break;

    case COLTYPE_BLANK:
      Add(new cMenuEditIntItem(IndentMenuItem(tr("Width")), &(preclistcolumns[i].Width), 0, 24));
      break;

    case COLTYPE_DATE:
      Add(new cMenuEditIntItem(IndentMenuItem(tr("Width")), &(preclistcolumns[i].Width), 8, 24));
      Add(new cMenuEditStraItem(IndentMenuItem(tr("Alignment")), &(preclistcolumns[i].Align), 3, AlignmentType_names));
      break;

    case COLTYPE_TIME:
      Add(new cMenuEditIntItem(IndentMenuItem(tr("Width")), &(preclistcolumns[i].Width), 5, 24));
      Add(new cMenuEditStraItem(IndentMenuItem(tr("Alignment")), &(preclistcolumns[i].Align), 3, AlignmentType_names));
      break;

    case COLTYPE_DATETIME:
      Add(new cMenuEditIntItem(IndentMenuItem(tr("Width")), &(preclistcolumns[i].Width), 14, 24));
      Add(new cMenuEditStraItem(IndentMenuItem(tr("Alignment")), &(preclistcolumns[i].Align), 3, AlignmentType_names));
      break;

    case COLTYPE_LENGTH:
      Add(new cMenuEditIntItem(IndentMenuItem(tr("Width")), &(preclistcolumns[i].Width), 3, 24));
      Add(new cMenuEditStraItem(IndentMenuItem(tr("Alignment")), &(preclistcolumns[i].Align), 3, AlignmentType_names));
      break;

    case COLTYPE_RATING:
      Add(new cMenuEditIntItem(IndentMenuItem(tr("Width")), &(preclistcolumns[i].Width), 5, 24));
      Add(new cMenuEditStraItem(IndentMenuItem(tr("Alignment")), &(preclistcolumns[i].Align), 3, AlignmentType_names));
     break;

    case COLTYPE_FILE:
      Add(new cMenuEditStrItem(IndentMenuItem(tr("Name")), (preclistcolumns[i].Name), 64, tr(FileNameChars)));

      Add(new cMenuEditIntItem(IndentMenuItem(tr("Width")), &(preclistcolumns[i].Width), 1, 24));
      Add(new cMenuEditStraItem(IndentMenuItem(tr("Alignment")), &(preclistcolumns[i].Align), 3, AlignmentType_names));
      Add(new cMenuEditStrItem(IndentMenuItem(tr("Filename")), (preclistcolumns[i].Op1), 64, tr(FileNameChars)));
      break;

    case COLTYPE_FILETHENCOMMAND:
      Add(new cMenuEditStrItem(IndentMenuItem(tr("Name")), (preclistcolumns[i].Name), 64, tr(FileNameChars)));

      Add(new cMenuEditIntItem(IndentMenuItem(tr("Width")), &(preclistcolumns[i].Width), 1, 24));
      Add(new cMenuEditStraItem(IndentMenuItem(tr("Alignment")), &(preclistcolumns[i].Align), 3, AlignmentType_names));
      Add(new cMenuEditStrItem(IndentMenuItem(tr("Filename")), (preclistcolumns[i].Op1), 1024, tr(FileNameChars)));
      Add(new cMenuEditStrItem(IndentMenuItem(tr("Command")), (preclistcolumns[i].Op2), 1024, tr(FileNameChars)));
      break;
    }
  }

  ret = asprintf(&itemStr, "%s\t%s", tr("Name of the recording"), tr("(fixed to the last position)"));
  if (ret <= 0) {
    esyslog("Error allocating linebuffer for cOsdItem.");
    return;
  }
  sItem = new cOsdItem(itemStr);
  sItem->SetSelectable(false);
  Add(sItem);

  // restore current position
  SetCurrent(Get(current));
}

eOSState myMenuSetupColumns::ProcessKey(eKeys Key) {
  int iTemp_type [4], i;
  for (i=0; i<MAX_RECLIST_COLUMNS; i++) {
    iTemp_type[i] = preclistcolumns[i].Type;
  }

  eOSState state = cOsdMenu::ProcessKey(Key);

  int iChanged = -1;
  for (i=0; i<MAX_RECLIST_COLUMNS && iChanged==-1; i++) {
    if (iTemp_type[i] != preclistcolumns[i].Type) {
      iChanged = i;
    }
  }

  if (iChanged >= 0) {
    preclistcolumns[iChanged].Width = recListDefaultValues[preclistcolumns[iChanged].Type].Width;
    preclistcolumns[iChanged].Align = recListDefaultValues[preclistcolumns[iChanged].Type].Align;
    Set();
    Display();
  }

  if (state == osUnknown) {
    switch (Key) {
      case kOk:
        return osBack;

      default:
        break;
    }
  }

  return state;
}

int msprintf(char **strp, const char *fmt, ...) {
  va_list ap;
  va_start (ap, fmt);
  int res=vasprintf (strp, fmt, ap);
  va_end (ap);
  return res;
}

char* IndentMenuItem(const char* szString, int indentions) {
  char* szIndented = NULL;
  msprintf(&szIndented, "%*s", strlen(szString)+indentions*2, szString);
  return szIndented;
}
