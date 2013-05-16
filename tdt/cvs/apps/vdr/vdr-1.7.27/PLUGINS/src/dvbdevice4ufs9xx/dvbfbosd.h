/*
 * dvbfbosd.h: Implementation of the DVB FB On Screen Display
 *
 * See the README file for copyright information and how to reach the author.
 *
 * $Id: dvbfbosd.h $
 */

#ifndef __DVBFBOSD_H
#define __DVBFBOSD_H

#include "vdr/osd.h"

class cDvbOsdProvider : public cOsdProvider {
private:
  int osdDev;
public:
  cDvbOsdProvider(int OsdDev);
  virtual cOsd *CreateOsd(int Left, int Top, uint Level);
  };

#endif //__DVBFBOSD_H
