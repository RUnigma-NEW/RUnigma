/*
 * 
 */

#ifndef VDR_VFD_COMMON_H
#define VDR_VFD_COMMON_H

#include <vdr/tools.h>

#ifdef DEBUG
# include <stdio.h>
# define Dprintf(x...) printf(x)
#else
# define Dprintf(x...)
#endif

#endif // VDR_VFD_COMMON_H
