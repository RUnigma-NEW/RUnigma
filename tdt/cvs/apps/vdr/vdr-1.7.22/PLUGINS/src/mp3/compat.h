/*
 * MP3/MPlayer plugin to VDR (C++)
 *
 * (C) 2001-2009 Stefan Huelswitt <s.huelswitt@gmx.de>
 *
 * This code is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This code is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 * Or, point your browser to http://www.gnu.org/copyleft/gpl.html
 */

#ifndef ___COMPAT_H
#define ___COMPAT_H

#ifndef APIVERSNUM
#include <vdr/config.h>
#endif

#if APIVERSNUM < 10503
#include <iconv.h>

class cCharSetConv {
private:
  iconv_t cd;
  char *result;
  size_t length;
  static char *systemCharacterTable;
public:
  cCharSetConv(const char *FromCode = NULL, const char *ToCode = NULL);
  ~cCharSetConv();
  const char *Convert(const char *From, char *To = NULL, size_t ToLength = 0);
  static const char *SystemCharacterTable(void) { return systemCharacterTable; }
  static void SetSystemCharacterTableX(const char *CharacterTable);
  };

#endif

#endif //___COMPAT_H
