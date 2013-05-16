/*
 * hdffmsgdef.h: TODO(short description)
 *
 * See the README file for copyright information and how to reach the author.
 *
 * $Id: hdffmsgdef.h 1.1 2009/12/29 14:27:22 kls Exp $
 */

#ifndef _DVB_MSGDEF_H_
#define _DVB_MSGDEF_H_


namespace DVB
{

typedef enum _eAudioStreamType
{
    audioStreamMpeg1,
    audioStreamMpeg2,
    audioStreamAc3,
    audioStreamAac,
    audioStreamHeAac,
    audioStreamPcm,
    audioStreamEAc3,
    audioStreamDts,
    audioStreamMaxValue
} eAudioStreamType;

typedef enum _eVideoStreamType
{
    videoStreamMpeg1,
    videoStreamMpeg2,
    videoStreamH264,
    videoStreamMpeg4Asp,
    videoStreamVc1,
    videoStreamMaxValue
} eVideoStreamType;


typedef enum _eTvFormat
{
    tvFormat4by3,
    tvFormat16by9,
    tvFormatMaxValue
} eTvFormat;

typedef enum _eVideoDisplay
{
    /*videoConversionAutomatic,
    videoConversionLetterbox16by9,
    videoConversionLetterbox14by9,
    videoConversionPillarbox,
    videoConversionCentreCutOut,
    videoConversionAlways16by9,
    videoConversionMaxValue*/
    videoDisplayPanAndScan,
    videoDisplayLetterBox,
    videoDisplayBestFit,
    videoDisplayNonLinear
} eVideoDisplay;

typedef struct _tVideoFormat
{
    eTvFormat TvFormat;
    eVideoDisplay VideoDisplay;
} tVideoFormat;

// HDMI definitions

typedef enum _eHdmiVideoMode
{
    /*videoMode576p50 = 18,
    videoMode720p50 = 19,
    videoMode1080i50 = 20,
    videoMode576i50 = 22,*/
    videoModePAL,
    videoMode480p,
    videoMode576p50,
    videoMode720p60,
    videoMode1080i60,
    videoMode720p50,
    videoMode1080i50,
    videoMode1080p30,
    videoMode1080p24,
    videoMode1080p25,
    videoModeMaxValue
} eHdmiVideoMode;

} // end of namespace

#endif
