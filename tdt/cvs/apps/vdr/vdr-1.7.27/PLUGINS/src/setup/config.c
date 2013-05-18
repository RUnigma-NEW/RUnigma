/****************************************************************************
 * DESCRIPTION: 
 *             Read / Write Config Data
 *
 * $Id: config.cpp,v 1.13 2006/02/04 11:59:29 ralf Exp $
 *
 * Contact:    ranga@vdrtools.de
 *
 * Copyright (C) 2004 by Ralf Dotzert 
 ****************************************************************************/

#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include "vdr/plugin.h"
#include "tinystr.h"
#include "config.h"
#include "setupsetup.h"
#include "sysconfig.h"
#include "plugins.h"
#include "debug.h"
#include "util.h"

/**
 * Constructor
 * @param fname  XML File
 * @return
 */

// Initialize static private data

const char *Config::childLockCode = NULL; // will be set within Menu

Config::Config(char *fname)
{
  _filename  = Util::Strdupnew(fname);
  _libDir = NULL;
  _xmlDoc = TiXmlDocument( fname );
  _bootLinux = NULL;
  _returnValue  = NULL;

  childLockCode = NULL; //will not be freed hold only reference to data
}


Config::~Config()
{
  delete [] _returnValue;
  delete [] _bootLinux;
  delete [] _libDir;
  delete [] _filename;
}

/************************
* Read XML-File
*************************/
/**
 * Decode XML File
 * @return
 */
bool Config::LoadFile()
{
  bool ok = false;
  TiXmlElement *root;
  const char *sysconfigFile = NULL;

  if ((ok = _xmlDoc.LoadFile())) {
     if ((root = _xmlDoc.FirstChildElement("setup" )) != NULL
     && (sysconfigFile = root->Attribute("sysconfigFile"))
     && (ok = _sysconfig.LoadFile(sysconfigFile)) == true) {
        // Set some default Values
        const char *tmp = NULL;

        if ((tmp = root->Attribute("bootLinux")) == NULL)
           _bootLinux = Util::Strdupnew("/sbin/reboot");
        else
           _bootLinux = Util::Strdupnew(tmp);

        if ((tmp = root->Attribute("VDRlibDir")) == NULL)
           _libDir = Util::Strdupnew("/usr/lib/vdr/plugins");
        else
           _libDir = Util::Strdupnew(tmp);

        if ((tmp = root->Attribute("ReturnValue")) == NULL)
            _returnValue = Util::Strdupnew("");
        else {
           _returnValue = Util::Strdupnew(tmp);
           if (!strcasecmp(_returnValue,  "true/false")) setupSetup.ReturnValue = 0;
           else if (!strcasecmp(_returnValue, "on/off")) setupSetup.ReturnValue = 1;
           else if (!strcasecmp(_returnValue, "yes/no")) setupSetup.ReturnValue = 2;
           else if (!strcasecmp(_returnValue, "1/0")) setupSetup.ReturnValue = 3;
           else _returnValue = Util::Strdupnew("");
           }

        root = root->FirstChildElement();
        if (root != NULL && strcmp(root->Value(), "plugins") == 0) {
           const char *nameSysconfig = root->Attribute("sysconfig");
           if (nameSysconfig != NULL) {
              _activePlugins.SetSysconfigName(nameSysconfig);
              if (ok = loadPlugins(root->FirstChildElement()))
                 ok = _menus.LoadXml(root->NextSibling("menus"));
              }
           else
              ok = false;
           }
        }
     }

  if (!ok)
     dumpXMLError("Error while Loading XML-FILE");

  if (ok) {
     if ((ok = _sysconfig.LoadFile(sysconfigFile)) == true)
        ok = readVdrLib();
     }

  return(ok);
}

/**
 * Save the the ConfigFile, overwrite opened file
 * @return true if no error occured
 */
bool Config::SaveFile()
{
  return(SaveFile(_filename));
}

/**
 * Save the the ConfigFile,
 * @param fname name of config file to write to
 * @return true if no error occured
 */
bool Config::SaveFile(char *fname)
{
  bool ok = true;
  if (_filename != fname) {
     delete _filename;
     _filename = Util::Strdupnew(fname);
    }

  TiXmlComment  comment;
  comment.SetValue("\n\
-   VDR Configuration File\n\
-\n\
-\n\
-   Example:\n\
-\n\
-  <setup sysconfigFile=\"/var/lib/vdr/plugins/setup/sysconfig\"> \n\
-         bootLinux=\"/sbin/reboot\"\n\
-         VDRlibDir=\"/usr/lib/vdr/plugins\"\n\
-         ReturnValue=\"\" -- kann leer bleiben oder: \"true/false\", \"on/off\", \"yes/no\", \"1/0\" --\n\
-    <plugins sysconfig=\"PLUGINLIST\">\n\
-         <plugin name=\"setup\" info=\"VDR-Setup\" active=\"yes\" />\n\
-         <plugin name=\"vdrcd\" param=\"-c /dev/dvd -c /media/dvd -c /media/cdfs\" info=\"VDR CD\"/>\n\
-         ...\n\
-    </plugin>\n\
-    <menus>\n\
-       <menu name=\"VDR\">\n\
-         <menu name=\"OSD\" system=\"OSD\" />\n\
-         <menu name=\"EPG\" system=\"EPG\" />\n\
-         <menu name=\"DVB\" system=\"DVB\" />\n\
-         <menu name=\"LNB\" system=\"LNB\" />\n\
-         <menu name=\"CAM\" system=\"CAM\" /> -- vdr-1.5.x --\n\
-         <menu name=\"CICAM\" system=\"CICAM\" /> -- vdr-1.4.7 --\n\
-         <menu name=\"Recording\" system=\"Record\" />\n\
-         <menu name=\"Replay\" system=\"Replay\" />\n\
-         <menu name=\"Miscellaneous\" system=\"Misc\" />\n\
-         <menu name=\"Plugins\" system=\"Plugins\" />\n\
-         <menu name=\"Menu Edit\" system=\"VDRMenu\" />\n\
-         <menu name=\"Plugins activate / deactivate\" system=\"ActPlugins\" />\n\
-         <menu name=\"LiveBuffer\" system=\"LiveBuffer\" /> -- Wenn der LiveBuffer-Patch angewendet wurde --\n\
-       </menu>\n\
-    <menu name=\"Netzwerk\" help1=\"Netzwerkeinstellungen\" help2=\"network.hlp\" command=\"/etc/init.d/networking restart\">\n\
-       <entry name=\"Benutze DHCP\" sysconfig=\"USE_DHCP\" type=\"bool\" value=\"true\" command=\"/etc/init.d/networking restart/>\n\
-       <entry name=\"IP-Adresse\" sysconfig=\"IP\" type=\"ip\" value=\"192.168.0.111\" />\n\
-        ...\n\
-    </menu>\n\
-    <menu name=\"Sonstiges\">\n\
-        <entry name=\"Ramddisk im MB\" sysconfig=\"RAMDISK\" type=\"number\" value=\"16\" />\n\
-        <menu name=\"SubMenu\">\n\
-          <entry ... />\n\
-        </menu>\n\
-    </menu>\n\
-   </menus>\n\
-  </setup>\n\
" );

  TiXmlDocument xml = TiXmlDocument(fname);
  TiXmlElement root("setup");
  root.SetAttribute("sysconfigFile", _sysconfig.GetFileName());
  root.SetAttribute("bootLinux", _bootLinux);
  root.SetAttribute("VDRlibDir", _libDir);
  root.SetAttribute("ReturnValue", _returnValue);

  TiXmlElement *plugins=savePlugins();

  if (plugins!= NULL && root.LinkEndChild(plugins) != NULL
  && _menus.SaveXml(&root) == true
  && xml.InsertEndChild(comment) != NULL
  && xml.InsertEndChild(root) != NULL
  && xml.SaveFile())
     ok=true;
  else {
     ok = false;
     dumpXMLError("Error writing file");
     }

  if (ok) {
     prepareSysConfig();
     ok = _sysconfig.SaveFile();
     }

  return(ok);
}

/**
 * Returns Child Lock Code
 * @return child Lock Code or NULL if not available
 */
const char *Config::GetChildLock()
{
  return(childLockCode);
}

/**
 * Set a child Clock Code,
 * @param lock lockk code "0000" or NULL means not locked.
 */
void Config::SetChildLock(const char *lock)
{
  childLockCode = lock;
}

//-------------------------------------------------------
// Private Methods
//-------------------------------------------------------
/**
 * Load XML Plugin Information
 * @param elem
 * @return
 */
bool Config::loadPlugins(TiXmlNode *node)
{
  bool ok = true;
  TiXmlElement *elem = NULL;
  if (node == NULL || strcmp(node->Value(), "plugin") != 0 ) {
     dumpXMLError("no <plugin> tag found");
     ok=false;
     }
  else {
     do {
        elem = node->ToElement ();
        const char *name = elem->Attribute("name");
        const char *info = elem->Attribute("info");
        const char *active = elem->Attribute("active");
        const char *param = elem->Attribute("param");
        const char *protect = elem->Attribute("protected");
        bool  b_active;
        bool  b_protect;
        if (protect == NULL || Util::isBool(protect, b_protect) == false)
           b_protect = false;

        if (name != NULL && info != NULL && active != NULL && Util::isBool(active, b_active))
           _activePlugins.AddPlugin(name, param,  info, b_active, b_protect);
        else
           ok = false;
        } while ((node = node->NextSibling()) != NULL && ok == true);
     }

  return(ok) ;
}

/**
 * Save Plugins
 * @return ptr to XML Element
 */
TiXmlElement *Config::savePlugins()
{
  TiXmlElement *xml = new TiXmlElement("plugins");

  xml->SetAttribute("sysconfig", _activePlugins.GetSysconfigName());

  savePlugins(&_activePlugins, xml);
  savePlugins(&_notInSystemPlugins, xml);

  return(xml);
}

void Config::savePlugins(Plugins *plugins, TiXmlElement *xml)
{
  int nr = plugins->GetNr();

  for (int i=0; i<nr && xml!=NULL; i++) {
     Plugin *p = plugins->Get(i);
     if (p!= NULL) {
        TiXmlElement *pl = new TiXmlElement("plugin");
        if (pl != NULL) {
           pl->SetAttribute("name", p->GetName());
           if (p->GetParameter() != NULL)
              pl->SetAttribute("param", p->GetParameter());
           pl->SetAttribute("info", p->GetInfo());
           pl->SetAttribute("active", p->GetActiveString());
           if (p->GetProtect() == true)
              pl->SetAttribute("protected", Util::boolToStr(p->GetProtect()));
           xml->LinkEndChild(pl);
           }
        }
     }
}


/**
 * Dump XML-Error
 */
void Config::dumpXMLError()
{
  const char *errStr;
  int col, row;
  if (_xmlDoc.Error()) {
     errStr = _xmlDoc.ErrorDesc();
     col    = _xmlDoc.ErrorCol();
     row    = _xmlDoc.ErrorRow();
     DEBUG6("%s: Error in %s Col=%d Row=%d :%s\n", DBG_PREFIX, _filename, col, row, errStr );
     }
}

void Config::dumpXMLError(const char *myErrStr)
{
  const char *errStr;
  int col, row;

  if (_xmlDoc.Error()) {
     errStr = _xmlDoc.ErrorDesc();
     col    = _xmlDoc.ErrorCol();
     row    = _xmlDoc.ErrorRow();
     DEBUG7("%s: %s: %s: Error in %s Col=%d Row=%d\n", DBG_PREFIX, myErrStr, errStr, _filename, col, row);
     }
}

/**
 * Returns a reference to the Plugins Object
 * @return plugins object
 */
Plugins *Config::GetPlugins()
{
  return(&_activePlugins);
}

/**
 * returns a reference to the Menus Object
 * @return reference to Menus Object
 */
Menus *Config::GetMenus()
{
  return(&_menus);
}

/**
 * Write internal representation to Sysconf Values
 */
void Config::prepareSysConfig()
{
  // Plugins
  const char *pluginList = _activePlugins.GetActivePlugins();
  _sysconfig.SetVariable(_activePlugins.GetSysconfigName(), pluginList);
  for (int i=0; i<_menus.GetNr(); i++)
     _menus.GetMenuNode(i)->SetSysConfig(&_sysconfig);
}

/**
 * get all installed plugins in the specified lib directory
 * and set the state within the plugin list
 * @return true on success
 */
bool Config::readVdrLib()
{
  bool ok = true;
  DIR  *dir = opendir(_libDir);
  struct dirent *entry = NULL;
  char *module = NULL;
  char *tmp = NULL;
  char *suffix = NULL;

  //accept only dynamic Link libs with the current vdr version
  asprintf(&suffix, ".so.%s", APIVERSION);
  if (dir !=NULL) {
     while ((entry = readdir(dir)) != NULL) {
        if (strncmp(entry->d_name, "libvdr-", 7) == 0  && (tmp = strstr(entry->d_name, suffix)) != NULL) {
           tmp[0]='\0';
           module = entry->d_name +7;
           _activePlugins.SetLibDirPlugin(module);
          }
        }

     int nrPlugins = _activePlugins.GetNr();
     for (int i=0; i<nrPlugins; i++) {
        Plugin *p = _activePlugins.Get(i);

        if (p != NULL && p->GetInSystem() == false) {
           _activePlugins.Del(p, false);
           _notInSystemPlugins.Add(p);
           nrPlugins--;
           i--;
           }
        }

     closedir(dir);
     }
  else {
     DEBUG3("%s: Could not read directory: %s\n", DBG_PREFIX, _libDir);
     ok = false;
     }

  if (suffix != NULL) free(suffix);

  return(ok);
}

char *Config::GetBootLinux()
{
  return(_bootLinux);
}
