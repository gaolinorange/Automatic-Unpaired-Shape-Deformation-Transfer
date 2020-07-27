// (C) Copyright 2014 by Autodesk, Inc.

#ifndef BASE_DEBFILEOUT_HH_INCLUDED
#define BASE_DEBFILEOUT_HH_INCLUDED
#ifdef DEB_ON

#include <string>
#include <Base/Config/BaseDefines.hh>


namespace Debug {

//! Make a file name composing the input arguments:
//               prefix_filname_[suffix_]#count#.ext
// count is a number that increase any time the function is called,
// it is expressed with 4 decimal digits filled with zeros.
BASEDLLEXPORT
std::string make_filename(const char* _prfx, const char* _flnm, 
                          const char* _ext, const char* _sfx = NULL);

BASEDLLEXPORT
std::string set_filename_extension(const char* _flnm, const char* _ext);

} // Debug

#endif // DEB_ON

#endif // BASE_DEBFILEOUT_HH_INCLUDED
