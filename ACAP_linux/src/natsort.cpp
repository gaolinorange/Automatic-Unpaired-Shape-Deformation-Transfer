//========================================================================
// Copyright (c) 1998-2010,2011 Free Software Foundation, Inc.
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, distribute with modifications, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE ABOVE COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
// OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
// THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
// Except as contained in this notice, the name(s) of the above copyright
// holders shall not be used in advertising or otherwise to promote the
// sale, use or other dealings in this Software without prior written
// authorization.
//========================================================================

//========================================================================
//  Author: Jan-Marten Spit <jmspit@euronet.nl>
//========================================================================

#include "natsort.hpp"

#include <ctype.h>
#include <string.h>
#include <iostream>

inline bool t_digit( char c ) {
  return c >= 48 && c <= 57;
}

int natstrcmp( const char* s1, const char* s2 ) {
  const char* p1 = s1;
  const char* p2 = s2;
  const unsigned short st_scan = 0;
  const unsigned short st_alpha = 1;
  const unsigned short st_numeric = 2;
  unsigned short state = st_scan;
  const char* numstart1 = 0;
  const char* numstart2 = 0;
  const char* numend1 = 0;
  const char* numend2 = 0;
  unsigned long sz1 = 0;
  unsigned long sz2 = 0;
  while ( *p1 && *p2  ) {
    switch ( state ) {
      case st_scan:
        if ( !t_digit(*p1) && !t_digit(*p2) ) {
          state = st_alpha;
          if ( *p1 == *p2 ) {
            p1++;p2++;
          } else if ( *p1 < *p2 ) return -1;
          else return 1;
        } else
        if ( t_digit(*p1) && !t_digit(*p2) ) return -1;
        else if ( !t_digit(*p1) && t_digit(*p2) ) return 1;
        else {
          if ( sz1 == 0 )
            while ( *p1 == '0' ) {p1++; sz1++;}
          else
            while ( *p1 == '0' ) p1++;
          if ( sz2 == 0 )
            while ( *p2 == '0' ) {p2++; sz2++;}
          else
            while ( *p2 == '0' ) p2++;
          if ( sz1 == sz2 ) { sz1 = 0; sz2 = 0; };
          if ( !t_digit(*p1) ) p1--;
          if ( !t_digit(*p2) ) p2--;
          numstart1 = p1;
          numstart2 = p2;
          numend1 = numstart1;
          numend2 = numstart2;
          p1++;
          p2++;
        }
        break;
      case st_alpha:
        if ( !t_digit(*p1) && !t_digit(*p2) ) {
          if ( *p1 == *p2 ) {
            p1++;p2++;
          } else if ( *p1<*p2 ) return -1;
          else return 1;
        } else state = st_scan;
        break;
      case st_numeric:
        while ( t_digit(*p1) ) numend1 = p1++;
        while ( t_digit(*p2) ) numend2 = p2++;
        if ( numend1-numstart1 == numend2-numstart2 &&
            !strncmp( numstart1,numstart2,numend2-numstart2+1) ) state = st_scan; else {
          if ( numend1-numstart1 < numend2-numstart2 )
            return -1;
          if ( numend1-numstart1 > numend2-numstart2 )
            return 1;
          while ( *numstart1 && *numstart2 ) {
            if ( *numstart1 < *numstart2 ) return -1;
            if ( *numstart1 > *numstart2 ) return 1;
            numstart1++;
            numstart2++;
          }
        }
        break;
    }
  }
  if ( sz1 < sz2 ) return -1;
  if ( sz1 > sz2 ) return 1;
  if ( *p1 == 0 && *p2 != 0 ) return -1;
  if ( *p1 != 0 && *p2 == 0 ) return 1;  
  return 0;
}

/**
 * STL natural less-than string compare
 * @return true when natural s1 < s2
 */
bool natstrlt( const char* s1, const char* s2 ) {
  //std::cout << "natstrlt s1=" << s1 << " s2=" << s2 << std::endl;
  const char* p1 = s1;
  const char* p2 = s2;
  const unsigned short st_scan = 0;
  const unsigned short st_alpha = 1;
  const unsigned short st_numeric = 2;
  unsigned short state = st_scan;
  const char* numstart1 = 0;
  const char* numstart2 = 0;
  const char* numend1 = 0;
  const char* numend2 = 0;
  unsigned long sz1 = 0;
  unsigned long sz2 = 0;
  while ( *p1 && *p2  ) {
    switch ( state ) {
      case st_scan:
        if ( !t_digit(*p1) && !t_digit(*p2) ) {
          state = st_alpha;
          if ( *p1 == *p2 ) {
            p1++;p2++;
          } else return *p1<*p2;
        } else
        if ( t_digit(*p1) && !t_digit(*p2) ) return true;
        else if ( !t_digit(*p1) && t_digit(*p2) ) return false;
        else {
          state = st_numeric;
          if ( sz1 == 0 )
            while ( *p1 == '0' ) {p1++; sz1++;}
          else
            while ( *p1 == '0' ) p1++;
          if ( sz2 == 0 )
            while ( *p2 == '0' ) {p2++; sz2++;}
          else
            while ( *p2 == '0' ) p2++;
          if ( sz1 == sz2 ) { sz1 = 0; sz2 = 0; };
          if ( !t_digit(*p1) ) p1--;
          if ( !t_digit(*p2) ) p2--;
          numstart1 = p1;
          numstart2 = p2;
          numend1 = numstart1;
          numend2 = numstart2;
        }
        break;
      case st_alpha:
        if ( !t_digit(*p1) && !t_digit(*p2) ) {
          if ( *p1 == *p2 ) {
            p1++;p2++;
          } else return *p1<*p2;
        } else state = st_scan;
        break;
      case st_numeric:
        while ( t_digit(*p1) ) numend1 = p1++;
        while ( t_digit(*p2) ) numend2 = p2++;
        if ( numend1-numstart1 == numend2-numstart2 &&
            !strncmp( numstart1,numstart2,numend2-numstart2+1) ) state = st_scan; else {
          if ( numend1-numstart1 != numend2-numstart2 )
            return numend1-numstart1 < numend2-numstart2;
          while ( *numstart1 && *numstart2 ) {
            if ( *numstart1 != *numstart2 ) return *numstart1 < *numstart2;
            numstart1++;
            numstart2++;
          }
        }
        break;
    }
  }
  if ( sz1 < sz2 ) return true;
  if ( sz1 > sz2 ) return false;
  if ( *p1 == 0 && *p2 != 0 ) return true;
  if ( *p1 != 0 && *p2 == 0 ) return false;  
  return false;
}


