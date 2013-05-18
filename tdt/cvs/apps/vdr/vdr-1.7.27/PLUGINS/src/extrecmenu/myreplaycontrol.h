class myReplayControl:public cReplayControl
{
 private:
   bool timesearchactive;
   bool fCallPlugin;
 public:
   myReplayControl();
   ~myReplayControl();
   eOSState ProcessKey(eKeys Key);
};
