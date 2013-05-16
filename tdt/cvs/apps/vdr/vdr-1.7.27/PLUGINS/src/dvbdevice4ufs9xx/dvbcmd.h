/*
 * hdffcmd.h: TODO(short description)
 *
 * See the README file for copyright information and how to reach the author.
 *
 * $Id: hdffcmd.h 1.1 2009/12/29 14:27:18 kls Exp $
 */

#ifndef _DVB_CMD_H_
#define _DVB_CMD_H_

#include "dvbmsgdef.h"
#include <stdint.h>

namespace DVB
{

class cDvbCmdIf
{
private:
    int mOsdDev;

public:
    cDvbCmdIf(int OsdDev);
    ~cDvbCmdIf(void);

    void CmdAvSetVideoFormat(uint8_t DecoderIndex, const tVideoFormat * pVideoFormat);

    void CmdHdmiSetVideoMode(eHdmiVideoMode VideoMode);
};

} // end of namespace

#endif
