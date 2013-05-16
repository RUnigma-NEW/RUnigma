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

#include <stdlib.h>
#include <string.h>

#if APIVERSNUM < 10503

#include <errno.h>
#include <vdr/tools.h>
#include "common.h"

// --- cCharSetConv ----------------------------------------------------------

char *cCharSetConv::systemCharacterTable = NULL;

cCharSetConv::cCharSetConv(const char *FromCode, const char *ToCode)
{
  if (!FromCode)
     FromCode = systemCharacterTable;
  if (!ToCode)
     ToCode = "UTF-8";
  cd = (FromCode && ToCode) ? iconv_open(ToCode, FromCode) : (iconv_t)-1;
  result = NULL;
  length = 0;
}

cCharSetConv::~cCharSetConv()
{
  free(result);
  iconv_close(cd);
}

void cCharSetConv::SetSystemCharacterTableX(const char *CharacterTable)
{
  free(systemCharacterTable);
  systemCharacterTable = NULL;
  if (!strcasestr(CharacterTable, "UTF-8")) {
     systemCharacterTable = strdup(CharacterTable);
     }
}

const char *cCharSetConv::Convert(const char *From, char *To, size_t ToLength)
{
  if (cd != (iconv_t)-1 && From && *From) {
     char *FromPtr = (char *)From;
     size_t FromLength = strlen(From);
     char *ToPtr = To;
     if (!ToPtr) {
        length = max(length, FromLength * 2); // some reserve to avoid later reallocations
        result = (char *)realloc(result, length);
        ToPtr = result;
        ToLength = length;
        }
     else if (!ToLength)
        return From; // can't convert into a zero sized buffer
     ToLength--; // save space for terminating 0
     char *Converted = ToPtr;
     while (FromLength > 0) {
           if (iconv(cd, &FromPtr, &FromLength, &ToPtr, &ToLength) == size_t(-1)) {
              if (errno == E2BIG || errno == EILSEQ && ToLength < 1) {
                 if (To)
                    break; // caller provided a fixed size buffer, but it was too small
                 // The result buffer is too small, so increase it:
                 size_t d = ToPtr - result;
                 size_t r = length / 2;
                 length += r;
                 Converted = result = (char *)realloc(result, length);
                 ToLength += r;
                 ToPtr = result + d;
                 }
              if (errno == EILSEQ) {
                 // A character can't be converted, so mark it with '?' and proceed:
                 FromPtr++;
                 FromLength--;
                 *ToPtr++ = '?';
                 ToLength--;
                 }
              else if (errno != E2BIG)
                 return From; // unknown error, return original string
              }
           }
     *ToPtr = 0;
     return Converted;
     }
  return From;
}

#endif
