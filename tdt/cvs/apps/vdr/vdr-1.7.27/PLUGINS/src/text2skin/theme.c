//								-*- c++ -*-

#include "theme.h"
#include <vdr/osd.h>

cText2SkinTheme::cText2SkinTheme(const char *Skin): cText2SkinFile(Skin) {
}

cText2SkinTheme::~cText2SkinTheme() {
}

bool cText2SkinTheme::Parse(const char *Text) {
	int l = strlen(Text);
	bool result = false;
	if (l) {
		if (strncmp(Text, "Item=Color,", 11) == 0) {
			Text += 11;
			std::string name;
			tColor value;
			if (ParseVar(Text, "name", name) && ParseVar(Text, "default", &value)) {
				mMap[name] = mTheme.AddColor(name.c_str(), value);
				result = true;
			} else
				esyslog("ERROR: text2skin: Parameters name and default must be present");
		} else
			esyslog("ERROR: text2skin: syntax error");
	}
	return result;
}

