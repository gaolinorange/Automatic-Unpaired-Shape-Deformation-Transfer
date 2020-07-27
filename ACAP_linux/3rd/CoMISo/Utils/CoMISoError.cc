// (C) Copyright 2015 by Autodesk, Inc.

#include "CoMISoError.hh"

namespace COMISO { 

static const char* ERROR_MESSAGE[] =
{
  #define DEFINE_ERROR(CODE, MSG) MSG,
  #include "CoMISoErrorInc.hh"
  #undef DEFINE_ERROR
};

const char* Error::message() const { return ERROR_MESSAGE[idx_]; }

}//namespace COMISO
