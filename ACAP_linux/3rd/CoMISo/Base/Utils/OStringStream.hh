// (C) Copyright 2016 by Autodesk, Inc.

#ifndef BASE_OSTRINGSTREAM_HH_INCLUDED
#define BASE_OSTRINGSTREAM_HH_INCLUDED

#include <Base/Utils/IOutputStream.hh>

namespace Base {

class BASEDLLEXPORT OStringStream : public IOutputStream
{
public:
  virtual IOutputStream& print(const char);
  virtual IOutputStream& print(const int);
  virtual IOutputStream& print(const size_t);
  virtual IOutputStream& print(const double);
  virtual IOutputStream& print(const char* const);
  virtual IOutputStream& print(const Command&);

  std::string str;
};

}//namespace Base

#endif//BASE_OSTRINGSTREAM_HH_INCLUDED
