// (C) Copyright 2016 by Autodesk, Inc.

#include "Base/Security/Mandatory.hh"
#include "IOutputStream.hh"
#include "Base/Code/CodeLink.hh"
#include "Base/Utils/ThrowError.hh"

#include <boost/filesystem.hpp>
#include <string>

namespace Base {

IOutputStream& operator<<(IOutputStream& _os, const int _i)
{
  return _os.print(_i);
}

IOutputStream& operator<<(IOutputStream& _os, const double _d)
{
  return _os.print(_d);
}

IOutputStream& operator<<(IOutputStream& _os, const char* const _s)
{
  return _os.print(_s);
}

IOutputStream& operator<<(IOutputStream& _os, const char _c)
{
  return _os.print(_c);
}

IOutputStream& operator<<(IOutputStream& _os, const size_t _i)
{
  return _os.print(_i);
}

#ifdef UINT_SIZET_DIFFER
IOutputStream& operator<<(IOutputStream& _os, const unsigned int _i)
{
  return _os.print(size_t(_i));
}
#endif//UINT_SIZET_DIFFER

IOutputStream& operator<<(IOutputStream& _os, const float _f)
{
  return _os.print((double)_f);
}

IOutputStream& operator<<(IOutputStream& _os, const std::string& _s)
{
  return _os.print(_s.c_str());
}

IOutputStream& operator<<(IOutputStream& _os, const Command& _co)
{
  return _os.print(_co);
}

IOutputStream& operator<<(IOutputStream& _os, const Command::Type _cmd_type)
{
  return _os.print(Command(_cmd_type));
}


IOutputStream& operator<<(IOutputStream& _os, const boost::filesystem::path& _path)
{
  return _os << '\"' << _path.string().c_str() << '\"';
}

// Represent this as an object in the stream, to preserve the streaming order
struct FunctionNameFilter
{
  FunctionNameFilter(const char* _fnct) : fnct_(_fnct) {}
  const char* fnct_;
};

IOutputStream& operator<<(IOutputStream& _os, const FunctionNameFilter& _fnf)
{
  // The goal is to filter unstable text from the function name, e.g., lambdas.
  // MSVC Lambda function name: <lambda_136f4d101172d40b57aea5f0078ce711>
  // Multiple lambdas could be in the same function?
  // TODO: this naming pattern is compiler/platform dependent
  // gcc lambda name is <lambdaX>
  
  const char lmbd[] = "<lambda";
  const char* fnct = _fnf.fnct_;
  for(;;)
  {
    const char* lmbd_pos = strstr(fnct, lmbd);
    if (lmbd_pos == NULL)
    {// print the rest of the function name
      _os << fnct;
      break;
    }
    // print everything until here (char by char, not optimal, but easy)
    for (; fnct != lmbd_pos; ++fnct)
      _os << *fnct;
    _os << lmbd; // now print lambda
    // and skip until > or end of string
    for (; *fnct != '>' && *fnct != '\0'; ++fnct)
      ;
  } 
  return _os;
}

IOutputStream& operator<<(IOutputStream& _os, const CodeLink& _lnk)
{
#ifdef WIN32
  const char path_sep = '\\';
#else//!WIN32
  const char path_sep = '/';
#endif//WIN32
  
  const char* flnm_pntr = strrchr(_lnk.file, path_sep);
  if (flnm_pntr == NULL)
    flnm_pntr = _lnk.file;
  else
    ++flnm_pntr;

  return _os << "@ [" << FunctionNameFilter(_lnk.fnct) << "() in " 
    << flnm_pntr << ":" << _lnk.line << "]";
}

}//namespace Base

#ifdef DEB_ON
namespace Debug {

Base::IOutputStream& operator<<(Base::IOutputStream& _os, 
  const ThrowInfo& _thrw_info)
{
  return _os << "thrown in " << _thrw_info.modl << " " << _thrw_info.lnk;
}

}//namespace Debug
#endif//DEB_ON
