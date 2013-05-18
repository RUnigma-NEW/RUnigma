/*
 * 
 */

#ifndef VDR_VFD_STATUS_H
#define VDR_VFD_STATUS_H

#include <vdr/status.h>

class cVFDStatus : public cStatus {
protected:
	virtual void OsdChannel(const char *Text);
	virtual void SetVolume(int Volume, bool Absolute);

public:
	cVFDStatus();
	virtual ~cVFDStatus();
};

#endif // VDR_VFD_STATUS_H
