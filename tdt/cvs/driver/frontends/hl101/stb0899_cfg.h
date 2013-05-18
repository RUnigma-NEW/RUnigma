/*
	STB0899 Multistandard Frontend driver
	Copyright (C) Manu Abraham (abraham.manu@gmail.com)

	Copyright (C) ST Microelectronics

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef __STB0899_CFG_H
#define __STB0899_CFG_H

#define STB0899_DVBS2_ESNO_AVE			3
#define STB0899_DVBS2_ESNO_QUANT		32
#define STB0899_DVBS2_AVFRAMES_COARSE		10
#define STB0899_DVBS2_AVFRAMES_FINE		20
#define STB0899_DVBS2_MISS_THRESHOLD		6
#define STB0899_DVBS2_UWP_THRESHOLD_ACQ		1125
#define STB0899_DVBS2_UWP_THRESHOLD_TRACK	758
#define STB0899_DVBS2_UWP_THRESHOLD_SOF		1350
#define STB0899_DVBS2_SOF_SEARCH_TIMEOUT	1664100

#define STB0899_DVBS2_BTR_NCO_BITS		28
#define STB0899_DVBS2_BTR_GAIN_SHIFT_OFFSET	15
#define STB0899_DVBS2_CRL_NCO_BITS		30
#define STB0899_DVBS2_LDPC_MAX_ITER		70

#endif //__STB0899_CFG_H
