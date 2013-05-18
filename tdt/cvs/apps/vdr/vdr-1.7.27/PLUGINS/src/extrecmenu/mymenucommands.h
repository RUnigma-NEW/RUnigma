class myMenuCommands:public cOsdMenu
{
 private:
#if VDRVERSNUM > 10713
  cList<cNestedItem> *commands;
  cString parameters;
  cString title;
  cString command;
  bool confirm;
  char *result;
  bool Parse(const char *s);
#else
  cCommands *commands;
  char *parameters;
#endif
  eOSState Execute(void);
 public:
#if VDRVERSNUM > 10713
  myMenuCommands(const char *Title, cList<cNestedItem> *Commands, const char *Parameters = NULL);
#else
  myMenuCommands(const char *Title, cCommands *Commands, const char *Parameters = NULL);
#endif
  virtual ~myMenuCommands();
  virtual eOSState ProcessKey(eKeys Key);
};
