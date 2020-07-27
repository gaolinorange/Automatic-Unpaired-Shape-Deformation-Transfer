// (C) Copyright 2014 by Autodesk, Inc.

#ifndef MANDATORY_HH_INCLUDED
#define MANDATORY_HH_INCLUDED

// only test this in MSVC and in builds where _SECURE is defined
#if defined(_MSC_VER) && defined(_SECURE)

#include <cstdio>
#include <cstring>
#include <cwchar>
#include <iosfwd>
#include <malloc.h>
#include <ostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <typeinfo>
#include <wchar.h>
#include <xlocale>
#include <xlocbuf>
#include <xlocmon>
#include <xlocnum>
#include <xutility>

#define _SDL_BANNED_RECOMMENDED 1
#include "banned.h"

#pragma warning(error:4995) // set this as an error from here on

#define INSECURE_INCLUDE_SECTION_BEGIN __pragma(warning(push)) \
  __pragma(warning(disable:4995)) 

#define INSECURE_INCLUDE_SECTION_END __pragma(warning(pop))

#else

#define INSECURE_INCLUDE_SECTION_BEGIN
#define INSECURE_INCLUDE_SECTION_END

#endif // ifdef _MSC_VER

#endif // MANDATORY_HH_INCLUDED
