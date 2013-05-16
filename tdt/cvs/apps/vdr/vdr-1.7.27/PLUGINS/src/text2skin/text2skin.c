/*								-*- c++ -*-
 * text2skin.c: A plugin for the Video Disk Recorder
 *
 * See the README file for copyright information and how to reach the author.
 */

#include "text2skin.h"
#include "bitmap.h"
#include "setup.h"
#include "menu.h"
#include "i18n.h"
#include "loader.h"
#include "status.h"

#if APIVERSNUM < 10600
#error "VDR-1.6.0 API version or greater is required!"
#endif

const char *cText2SkinPlugin::VERSION        = "1.3.2+git";
const char *cText2SkinPlugin::SKINVERSION    = "1.1";
const char *cText2SkinPlugin::DESCRIPTION    = trNOOP("Loader for text-based skins");

cText2SkinPlugin::cText2SkinPlugin(void) {
}

cText2SkinPlugin::~cText2SkinPlugin() {
}

const char **cText2SkinPlugin::SVDRPHelpPages(void)
{
	static const char *HelpPages[] = {
		"FLUS\n"
		"    Flush the image cache (useful if images have changed and the"
		"    current version should be loaded).",
		NULL
		};
	return HelpPages;
}

cString cText2SkinPlugin::SVDRPCommand(const char *Command, const char *Option, int &ReplyCode)
{
	if (strcasecmp(Command, "FLUS") == 0) {
		// we use the default reply code here
		cText2SkinBitmap::FlushCache();
		return "image cache flushed.";
	}
	return NULL;
}

bool cText2SkinPlugin::Start(void) {
	Text2SkinStatus.SetLanguage(I18nCurrentLanguage());
	cText2SkinBitmap::Init();
	cText2SkinLoader::Start();
	return true;
}

cMenuSetupPage *cText2SkinPlugin::SetupMenu(void) {
	return new cText2SkinSetupPage;
}

bool cText2SkinPlugin::SetupParse(const char *Name, const char *Value) {
  return Text2SkinSetup.SetupParse(Name, Value);
}

VDRPLUGINCREATOR(cText2SkinPlugin); // Don't touch this!
