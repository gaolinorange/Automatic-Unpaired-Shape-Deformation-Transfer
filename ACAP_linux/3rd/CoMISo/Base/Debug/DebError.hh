// (C) Copyright 2015 by Autodesk, Inc.

#ifndef BASE_DEBERROR_HH_INCLUDED
#define BASE_DEBERROR_HH_INCLUDED

#include <Base/Utils/BaseError.hh>
#include <Base/Utils/IOutputStream.hh>

namespace Base {

inline IOutputStream& operator<<(IOutputStream& _ds, const Base::Error& _err)
{
  _ds << "\"" << _err.message() << "\"";
  return _ds;
}

}//namespace Debug

#endif//BASE_DEBERROR_HH_INCLUDED
