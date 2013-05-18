/*********************************************************
 * DESCRIPTION: 
 *             Header File
 *
 * $Id: config.h,v 1.9 2006/02/04 11:59:29 ralf Exp $
 *
 * Contact:    ranga@vdrtools.de
 *
 * Copyright (C) 2004 by Ralf Dotzert 
 *********************************************************/

#ifndef CONFIG_H
#define CONFIG_H
#include "tinystr.h"
#include "plugins.h"
#include "sysconfig.h"
#include "menus.h"
/**
@author Ralf Dotzert
*/





class Config{
public:
    Config(char *xmlFile);
    ~Config();
    bool         LoadFile();
    Plugins     *GetPlugins();
    Menus       *GetMenus();
    bool         SaveFile();
    bool         SaveFile(char* fname);
    char        *GetBootLinux();
    static const char *GetChildLock();
    static void SetChildLock(const char *lock);
    

private:
    TiXmlDocument       _xmlDoc;
    char               *_filename;
    char               *_bootLinux;
    Menus              _menus;
    Sysconfig          _sysconfig;
    Plugins            _activePlugins;
    Plugins            _notInSystemPlugins;
    static const char  *childLockCode;
    const char         *_libDir;
    const char         *_returnValue;
    bool                loadPlugins(TiXmlNode *node);
    TiXmlElement       *savePlugins() ;
    void                savePlugins(Plugins *p, TiXmlElement *xml);
    void                dumpXMLError();
    void                dumpXMLError(const char* errStr);
    bool                readVdrLib();
    void                prepareSysConfig();
};


#endif
