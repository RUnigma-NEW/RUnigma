/*********************************************************
 * DESCRIPTION: 
 *             Header File
 *
 * $Id: setupsystemmenu.h,v 1.1 2005/10/12 13:44:14 ralf Exp $
 *
 * Contact:    ranga@vdrtools.de        
 *
 * Copyright (C) 2004 by Ralf Dotzert 
 *********************************************************/

#ifndef SETUPSYSTEMMENU_H
#define SETUPSYSTEMMENU_H

#include <vdr/menu.h>
#include <vdr/interface.h>
#include <vdr/submenu.h>
#include "config.h"

class cSetupSystemMenu
{
public:
  cSetupSystemMenu();
  ~cSetupSystemMenu();
  static cOsdMenu *GetSystemMenu(const char *sysCommand, Config  *config);
};

#endif
