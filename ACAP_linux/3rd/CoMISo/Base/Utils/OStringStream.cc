// (C) Copyright 2016 by Autodesk, Inc.

#include "Base/Security/Mandatory.hh"
#include "OStringStream.hh"

#include <stdarg.h>

#ifndef WIN32
#include <cstdio>
#include <cstdarg>
#include <sstream>
#endif

namespace Base {

namespace {
// NOTE: This should be portable, but have not tested
template <size_t _bffr_size>
int sprintf_s(char (&_bffr)[_bffr_size], const char* _frmt, ...)
{
  va_list arg_ptr;
  va_start(arg_ptr, _frmt);
#ifndef WIN32
  int res = vsprintf(_bffr, _frmt, arg_ptr);
#else
  int res = vsprintf_s(_bffr, _bffr_size, _frmt, arg_ptr);
#endif // WIN32
  va_end(arg_ptr);
  return res;
}

}//namespace 

int print(char* _bffr, const size_t _bffr_size, const char* _frmt, ...)
{
  va_list arg_ptr;
  va_start(arg_ptr, _frmt);
#ifndef WIN32
  int res = vsprintf(_bffr, _frmt, arg_ptr);
#else
  int res = vsprintf_s(_bffr, _bffr_size, _frmt, arg_ptr);
#endif
  va_end(arg_ptr);
  return res;
}

IOutputStream& OStringStream::print(const char _c)
{
  str.append(1, _c);
  return *this;
}

IOutputStream& OStringStream::print(const int _i)
{
#ifndef WIN32
  std::stringstream ss;
  ss << _i;
  str.append(ss.str());
#else
  str.append(std::to_string(_i));
#endif // WIN32
  return *this;
}

IOutputStream& OStringStream::print(const size_t _i)
{
#ifndef WIN32
  std::stringstream ss;
  ss << _i;
  str.append(ss.str());
#else
  str.append(std::to_string(_i));
#endif // WIN32
  return *this;
}

IOutputStream& OStringStream::print(const double _d)
{
  char buf[128];
  sprintf_s(buf, "%.17g", _d);
  return OStringStream::print(buf);
}

IOutputStream& OStringStream::print(const char* const _str)
{
  str.append(_str);
  return *this;
}

IOutputStream& OStringStream::print(const Command& _cmd)
{
  // NOTE: Does nothing, strings should support 
  if ((_cmd.cmd & Command::END_LF) == (int)Command::END_LF)
      return OStringStream::print('\n');
  return *this;
}



//template <const char _frmt, const int _bffr_size = 128>
//IOutputStream& format(IOutputStream& _os, const double _d)
//{
//  char bffr[_bffr_size];
//  sprintf_s(bffr, _frmt, _d);
//  return std::string(bffr);
//}


}//namespace Base
