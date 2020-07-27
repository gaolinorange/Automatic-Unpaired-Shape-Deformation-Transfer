// (C) Copyright 2015 by Autodesk, Inc.

#include "DebFileOut.hh"

#include <string>
#include <sstream>
#include <iomanip> 

namespace Debug {

std::string make_filename(const char* _prfx, const char* _flnm, 
                          const char* _ext, const char* _sfx)
{
  static int cnt = 0;
  const char SEP = '_';
  std::stringstream sstr_flnm;
  sstr_flnm << std::setfill('0') << std::setw(4) << cnt++;
  sstr_flnm << SEP << _prfx << SEP << _flnm;
  if (_sfx != NULL)
    sstr_flnm << SEP << _sfx;

  sstr_flnm << '.' << _ext;
  return sstr_flnm.str();
}

std::string set_filename_extension(const char* _flnm, const char* _ext)
{
  std::string flnm(_flnm);
  const size_t dot_pos = flnm.find_last_of('.');
  if (dot_pos != std::string::npos) 
    flnm.replace(flnm.begin() + dot_pos + 1, flnm.end(), _ext);
  else
    flnm += std::string(".") + _ext;
  return flnm;
}

} // Debug
