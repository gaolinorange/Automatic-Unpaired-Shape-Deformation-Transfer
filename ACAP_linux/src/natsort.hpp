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

#ifndef natsort_hpp
#define natsort_hpp

#include <string>

/**
 * standard C natural string compare
 * @param s1 left string
 * @param s2 right string
 * @return -1 when s1 < s2, 0 when s1 == s2, 1 when s1 > s2
 */
int natstrcmp( const char* s1, const char* s2 );

/**
 * STL natural less-than string compare
 * @param s1 left string
 * @param s2 right string
 * @return true when natural s1 < s2
 */
bool natstrlt( const char* s1, const char* s2 );

/**
 * @param s1 left string
 * @param s2 right string
 * std::string variant of natstrlt.
 * @return true when natural s1 < s2
 */
inline bool stlnatstrlt( const std::string& s1, const std::string &s2 ) {
  return natstrlt( s1.c_str(), s2.c_str() );
}
//bool natstrlt(std::string const &s1, std::string const &s2)

#endif
