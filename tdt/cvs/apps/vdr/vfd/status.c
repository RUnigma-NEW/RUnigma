/*
 * 
 */

#include "status.h"
#include "common.h"
#include "vfd-functions.h"

cVFDStatus::cVFDStatus() {
	opendevice();
}

cVFDStatus::~cVFDStatus() {
	closedevice();
}

void cVFDStatus::OsdChannel(const char *Text) {
	writedisplay(Text);
}

void cVFDStatus::SetVolume(int Volume, bool Absolute) {
	if (Volume == 0) {
		displayicon(0x19, 0x01);
	} else {
		displayicon(0x19, 0x00);
	}
}