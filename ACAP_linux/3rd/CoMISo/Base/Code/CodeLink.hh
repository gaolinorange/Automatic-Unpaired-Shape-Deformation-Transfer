// (C) Copyright 2016 by Autodesk, Inc.

#ifndef BASE_CODELINK_HH_INCLUDED
#define BASE_CODELINK_HH_INCLUDED

namespace Base {

class IOutputStream;

//! Convenient wrapper around a preprocessor source code link
struct CodeLink
{
  CodeLink(
    const char* _fnct,
    const char* _file,
    const int _line
    ) 
    : fnct(_fnct), file(_file), line(_line) 
  {}
  
  const char* fnct;
  const char* file;
  int line;
};

IOutputStream& operator<<(IOutputStream& _os, const CodeLink& _lnk);

}//namespace Base

#define BASE_CODELINK ::Base::CodeLink(__FUNCTION__, __FILE__, __LINE__)

#endif//BASE_CODELINK_HH_INCLUDED
