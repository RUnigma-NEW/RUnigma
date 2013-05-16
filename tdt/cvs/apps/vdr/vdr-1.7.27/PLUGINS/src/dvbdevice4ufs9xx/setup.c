/*
 * setup.c: Setup for the DVB On Screen Display
 *
 * See the README file for copyright information and how to reach the author.
 *
 * $Id: setup.c 1.1 2009/12/29 14:30:45 kls Exp $
 */

#include "setup.h"
//#include "dvbhddevice.h"
#include "dvbcmd.h"

/*const int kResolution1080i = 0;
const int kResolution720p = 1;
const int kResolution576p = 2;
const int kResolution576i = 3;P*/
const int kResolutionPAL = 0;
const int kResolution480p = 1;
const int kResolution576p50 = 2;
const int kResolution720p60 = 3;
const int kResolution1080i60 = 4;
const int kResolution720p50 = 5;
const int kResolution1080i50 = 6;
const int kResolution1080p30 = 7;
const int kResolution1080p24 = 8;
const int kResolution1080p25 = 9;


cDvbSetup gDvbSetup;

cDvbSetup::cDvbSetup(void)
{
    //Resolution = 5;
    //TvFormat = 1;
    //VideoDisplay = vdLetterBox;
    Resolution = kResolution720p50;
    TvFormat = DVB::tvFormat16by9;
    VideoDisplay = DVB::videoDisplayLetterBox;
    OsdSize = 0;
}

bool cDvbSetup::SetupParse(const char *Name, const char *Value)
{
    if      (strcmp(Name, "Resolution")      == 0) Resolution      = atoi(Value);
    else if (strcmp(Name, "TvFormat")        == 0) TvFormat        = atoi(Value);
    else if (strcmp(Name, "VideoDisplay")    == 0) VideoDisplay    = atoi(Value);
    else if (strcmp(Name, "OsdSize")         == 0) OsdSize         = atoi(Value);
    else return false;
    return true;
}

void cDvbSetup::GetOsdSize(int &Width, int &Height, double &PixelAspect)
{
    if (OsdSize == 0) {
        if (Resolution == kResolution1080i60 || Resolution == kResolution1080i50 || Resolution == kResolution1080p30 || Resolution == kResolution1080p24 || Resolution == kResolution1080p25) {
            Width = 1920;
            Height = 1080;
        }
        else if (Resolution == kResolution720p60 || Resolution == kResolution720p50) {
            Width = 1280;
            Height = 720;
        }
        else if (Resolution == kResolution480p) {
            Width = 720;
            Height = 480;
        }
        else {             // PAL, 576p50
            Width = 720;
            Height = 576;
        }
        if (TvFormat == DVB::tvFormat16by9)
            PixelAspect = 16.0 / 9.0;
        else
            PixelAspect = 4.0 / 3.0;
    }
    else if (OsdSize == 1) {
        Width = 1920;
        Height = 1080;
        PixelAspect = 16.0 / 9.0;
    }
    else if (OsdSize == 2) {
        Width = 1280;
        Height = 720;
        PixelAspect = 16.0 / 9.0;
    }
    else if (OsdSize == 3) {
        Width = 1024;
        Height = 576;
        PixelAspect = 16.0 / 9.0;
    }
    else if (OsdSize == 4) {
        Width = 720;
        Height = 480;
        PixelAspect = 4.0 / 3.0;
    }
    else {
        Width = 720;
        Height = 576;
        PixelAspect = 4.0 / 3.0;
    }
    PixelAspect /= double(Width) / Height;
}

DVB::eHdmiVideoMode cDvbSetup::GetVideoMode(void)
{
    switch (Resolution)
    {
        /*case kResolution1080i:
        default:
            return DVB::videoMode1080i50;
        case kResolution720p:
            return DVB::videoMode720p50;
        case kResolution576p:
            return DVB::videoMode576p50;
        case kResolution576i:
            return DVB::videoMode576i50;*/
        case kResolutionPAL:
            return DVB::videoModePAL;
        case kResolution480p:
            return DVB::videoMode480p;
        case kResolution576p50:
            return DVB::videoMode576p50;
        case kResolution720p60:
            return DVB::videoMode720p60;
        case kResolution1080i60:
            return DVB::videoMode1080i60;
        case kResolution720p50:
        default:
            return DVB::videoMode720p50;
        case kResolution1080i50:
            return DVB::videoMode1080i50;
        case kResolution1080p30:
            return DVB::videoMode1080p30;
        case kResolution1080p24:
            return DVB::videoMode1080p24;
        case kResolution1080p25:
            return DVB::videoMode1080p25;
    }
}

cDvbSetupPage::cDvbSetupPage(DVB::cDvbCmdIf *pDvbCmdIf)
{
    //const int kResolutions = 4;
    const int kResolutions = 10;
    const int kTvFormats = 2;
    //const int kVideoDisplay = 6;
    const int kVideoDisplay = 4;
    //const int kOsdSizes = 5;
    const int kOsdSizes = 6;

    static const char * ResolutionItems[kResolutions] =
    {
    /*    "1080i",
        "720p",
        "576p",
        "576i",*/
        "PAL",
        "480p",
        "576p50",
        "720p60",
        "1080i60",
        "720p50",
        "1080i50",
        "1080p30",
        "1080p24",
        "1080p25",
    };

    static const char * TvFormatItems[kTvFormats] =
    {
        "4/3",
        "16/9",
    };

    static const char * VideoDisplayItems[kVideoDisplay] =
    {
    /*    tr("Automatic"),
        tr("Letterbox 16/9"),
        tr("Letterbox 14/9"),
        tr("Pillarbox"),
        tr("CentreCutOut"),
        tr("Always 16/9"),*/
        tr("pan&scan"),
        tr("letterbox"),
        tr("bestfit"),
        tr("nonlinear"),
    };

    static const char * OsdSizeItems[kOsdSizes] =
    {
        tr("Follow resolution"),
        "1920x1080",
        "1280x720",
        "1024x576",
        "720x480",
        "720x576",
    };

    mDvbCmdIf = pDvbCmdIf; 
    mNewDvbSetup = gDvbSetup;

    Add(new cMenuEditStraItem(tr("Resolution"), &mNewDvbSetup.Resolution, kResolutions, ResolutionItems));
    Add(new cMenuEditStraItem(tr("TV format"), &mNewDvbSetup.TvFormat, kTvFormats, TvFormatItems));
    Add(new cMenuEditStraItem(tr("Video Display"), &mNewDvbSetup.VideoDisplay, kVideoDisplay, VideoDisplayItems));
    Add(new cMenuEditStraItem(tr("OSD Size"), &mNewDvbSetup.OsdSize, kOsdSizes, OsdSizeItems));
}

cDvbSetupPage::~cDvbSetupPage(void)
{
}

void cDvbSetupPage::Store(void)
{
    SetupStore("Resolution", mNewDvbSetup.Resolution);
    SetupStore("TvFormat", mNewDvbSetup.TvFormat);
    SetupStore("VideoDisplay", mNewDvbSetup.VideoDisplay);
    SetupStore("OsdSize", mNewDvbSetup.OsdSize);

    if (mDvbCmdIf)
    {
        if (mNewDvbSetup.Resolution != gDvbSetup.Resolution)
            mDvbCmdIf->CmdHdmiSetVideoMode(mNewDvbSetup.GetVideoMode());

        DVB::tVideoFormat videoFormat;

        videoFormat.TvFormat = (DVB::eTvFormat) mNewDvbSetup.TvFormat;
        videoFormat.VideoDisplay = (DVB::eVideoDisplay) mNewDvbSetup.VideoDisplay;
        mDvbCmdIf->CmdAvSetVideoFormat(0, &videoFormat);
    }

    gDvbSetup = mNewDvbSetup;
}
