/*********************************************************
 * DESCRIPTION: 
 *             Header File
 *
 * $Id: i18n.h,v 1.3 2005/10/12 13:44:14 ralf Exp $
 *
 * Contact:    ranga@vdrtools.de
 *
 * Copyright (C) 2004 by Ralf Dotzert 
 *********************************************************/

#ifndef _I18N__H
#define _I18N__H

#include <vdr/i18n.h>

#if VDRVERSNUM < 10507

extern const tI18nPhrase Phrases[];

#ifndef trNOOP
#  define trNOOP(s) (s)
#endif

#ifndef trVDR
#  define trVDR(s) tr(s)
#endif

#endif // VDRVERSNUM < 10507

#endif //_I18N__H
