/*
 * setup.h: Setup for the DVB HD Full Featured On Screen Display
 *
 * See the README file for copyright information and how to reach the author.
 *
 * $Id: setup.h 1.1 2009/12/29 14:28:12 kls Exp $
 */

#ifndef __DVB_SETUP_H_
#define __DVB_SETUP_H_

#include <vdr/plugin.h>
#include "dvbcmd.h"

struct cDvbSetup
{
    cDvbSetup(void);
    bool SetupParse(const char * Name, const char * Value);
    void GetOsdSize(int &Width, int &Height, double &PixelAspect);
    DVB::eHdmiVideoMode GetVideoMode(void);

    int Resolution;         // VideoSystem
    int TvFormat;           // VideoFormat
    int VideoDisplay;       // VideoDisplayFormat, VideoConversion
    int OsdSize;
};

extern cDvbSetup gDvbSetup;

class cDvbSetupPage : public cMenuSetupPage
{
private:
    DVB::cDvbCmdIf * mDvbCmdIf;
    cDvbSetup mNewDvbSetup;

protected:
    virtual void Store(void);

public:
    cDvbSetupPage(DVB::cDvbCmdIf * pDvbCmdIf);
    virtual ~cDvbSetupPage(void);
};

#endif
