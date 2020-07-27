// (C) Copyright 2015 by Autodesk, Inc.

#include "BaseError.hh"

namespace Base { 

static const char* ERROR_MESSAGE[] =
{
  #define DEFINE_ERROR(CODE, MSG) MSG,
  #include "BaseErrorInc.hh"
  #undef DEFINE_ERROR
};

const char* Error::message() const { return ERROR_MESSAGE[idx_]; }

}//namespace Base
