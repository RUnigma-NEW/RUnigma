/****************************************************************************
 * DESCRIPTION:
 *             Setup Dialog
 *
 * $Id: setupsetup.h,v 1.2 2005/10/12 13:44:14 ralf Exp $
 *
 * Contact:    ranga@vdrtools.de
 *
 * Copyright (C) 2005 by Ralf Dotzert
 ****************************************************************************/


#ifndef SETUPSETUP_H
#define SETUPSETUP_H

#include <vdr/plugin.h>
#include "i18n.h"

/**
@author Ralf Dotzert
*/
//*****************************************************************************
// Setup Configuration
//*****************************************************************************

class cSetupSetup
{
  public:
    char _menuSuffix[20];
    char _entryPrefix[2];
    int  DirectMenu;
    int  ReturnValue;

    cSetupSetup();
    bool SetupParse(const char *Name, const char *Value);
};

extern cSetupSetup setupSetup;

//*****************************************************************************
// Setup Page
//*****************************************************************************
class cSetupSetupPage : public cMenuSetupPage
{
  private:
    const char *ReturnValues[3];
  public:
    cSetupSetupPage();
    void Store(void);
    eOSState ProcessKey(eKeys Key);
};

#endif
