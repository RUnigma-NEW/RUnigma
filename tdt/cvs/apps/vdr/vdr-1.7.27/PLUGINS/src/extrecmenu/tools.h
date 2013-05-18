std::string myStrReplace(std::string S,char C1,const char* C2);

class SortListItem:public cListObject
{
  private:
    std::string path;
  public:
    SortListItem(std::string _Path){path=_Path;};
    std::string Path(){return path;}
};

class SortList:public cList<SortListItem>
{
  public:
    void ReadConfigFile();
    void WriteConfigFile();
    bool Find(std::string Path);
};

extern SortList *mySortList;

bool MoveRename(const char *OldName,const char *NewName,cRecording *Recording,bool Move);

class myRecListItem:public cListObject
{
  friend class myRecList;
  private:
    static bool SortByName;
    char *filename;
    static char *StripEpisodeName(char *s);
  public:
    myRecListItem(cRecording *Recording);
    ~myRecListItem();
    virtual int Compare(const cListObject &ListObject)const;
    cRecording *recording;
};

class myRecList:public cList<myRecListItem>
{
  public:
    void Sort(bool SortByName);
};

// --- MoveListItem -----------------------------------------------------------
class MoveListItem:public cListObject
{
  private:
    bool moveinprogress;
    bool movecanceled;
    std::string from;
    std::string to;
  public:
    MoveListItem(std::string _From,std::string _To){from=_From;to=_To;moveinprogress=false;movecanceled=false;}
    std::string From(){return from;}
    std::string To(){return to;}
    void SetMoveInProgress(){moveinprogress=true;}
    bool GetMoveInProgress(){return moveinprogress;}
    void SetMoveCanceled(){movecanceled=true;}
    bool GetMoveCanceled(){return movecanceled;}
};

// --- MoveList ---------------------------------------------------------------
class MoveList:public cList<MoveListItem>
{
};

// --- CutterListItem ---------------------------------------------------------
class CutterListItem:public cListObject
{
  private:
    bool cutinprogress;
    std::string filename;
    std::string newfilename;
  public:
    CutterListItem(std::string _FileName){filename=_FileName;cutinprogress=false;};
    void SetNewFileName(std::string _NewFileName){newfilename=_NewFileName;}
    std::string FileName(){return filename;}
    std::string NewFileName(){return newfilename;}
    void SetCutInProgress(){cutinprogress=true;}
    bool GetCutInProgress(){return cutinprogress;}
};

// --- CutterList -------------------------------------------------------------
class CutterList:public cList<CutterListItem>
{
};

// --- WorkerThread -----------------------------------------------------------
class WorkerThread:public cThread
{
  private:
    bool cancelmove,cancelcut;
    MoveList *MoveBetweenFileSystemsList;
    CutterList *CutterQueue;
    void Cut(std::string From,std::string To);
    bool Move(std::string From,std::string To);
  protected:
    virtual void Action();
  public:
    WorkerThread();
    ~WorkerThread();
    const char *Working();
    bool IsCutting(std::string Path);
    bool IsMoving(std::string Path);
    void CancelCut(std::string Path);
    void CancelMove(std::string Path);
    void AddToCutterQueue(std::string Path);
    void AddToMoveList(std::string From,std::string To);
    bool IsCutterQueueEmpty(){return CutterQueue->First();}
    bool IsMoveListEmpty(){return MoveBetweenFileSystemsList->First();}
};

extern WorkerThread *MoveCutterThread;

class Icons
{
  private:
    static bool IsUTF8;
  public:
    static void InitCharSet();
    static const char* Continue(){return IsUTF8?"\ue000":"\x80";}
    static const char* DVD(){return IsUTF8?"\ue001":"\x81";}
    static const char* Directory(){return IsUTF8?"\ue002":"\x82";}
    static const char* FixedBlank(){return IsUTF8?"\ue003":"\x83";}
    static const char* Scissor(){return IsUTF8?"\ue004":"\x84";}
    static const char* MovingRecording(){return IsUTF8?"\ue005":"\x85";}
    static const char* MovingDirectory(){return IsUTF8?"\ue006":"\x86";}
    static const char* ProgressStart(){return IsUTF8?"\ue007":"\x87";}
    static const char* ProgressFilled(){return IsUTF8?"\ue008":"\x88";}
    static const char* ProgressEmpty(){return IsUTF8?"\ue009":"\x89";}
    static const char* ProgressEnd(){return IsUTF8?"\ue00a":"\x8a";}
    static const char* Recording(){return IsUTF8?"\ue00b":"\x8b";}
    static const char* AlarmClock(){return IsUTF8?"\ue00c":"\x8c";}
    static const char* TVScrambled(){return IsUTF8?"\ue00d":"\x8d";}
    static const char* Radio(){return IsUTF8?"\ue00e":"\x8e";}
    static const char* TV(){return IsUTF8?"\ue00f":"\x8f";}
    static const char* New(){return IsUTF8?"\ue010":"\x90";}
    static const char* Repititive_timer(){return IsUTF8?"\ue011":"\x91";}
    static const char* Running(){return IsUTF8?"\ue012":"\x92";}
    static const char* HDD(){return IsUTF8?"\ue01c":"\x9c";}
    static const char* StarFull(){return IsUTF8?"\ue018":"\x98";}
    static const char* StarHalf(){return IsUTF8?"\ue019":"\x99";}
};
