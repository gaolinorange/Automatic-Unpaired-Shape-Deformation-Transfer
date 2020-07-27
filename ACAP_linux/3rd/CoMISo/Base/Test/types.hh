// (C) Copyright 2016 by Autodesk, Inc.

#ifndef BASE_TYPES_HH_INCLUDE
#define BASE_TYPES_HH_INCLUDE

#ifdef TEST_ON

#include "Base/Utils/IOutputStream.hh"

namespace boost {
namespace filesystem {
class path;
}//namespace boost
}//namespace filesystem

namespace Test {

typedef boost::filesystem::path Path;

const char* const LOG_FILENAME = "out.txt";
const char* const REPORT_FILENAME = "report.txt";
const char* const REPORT_LEVEL_TAG = "Report Level: ";
}//namespace Test

#endif//TEST_ON

#endif//BASE_TYPES_HH_INCLUDE
