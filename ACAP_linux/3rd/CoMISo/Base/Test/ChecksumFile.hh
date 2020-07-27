// (C) Copyright 2016 by Autodesk, Inc.

#ifndef BASE_CHECKSUMFILE_HH_INCLUDE
#define BASE_CHECKSUMFILE_HH_INCLUDE

#ifdef TEST_ON

#include <Base/Test/Checksum.hh>

namespace Test {
namespace Checksum {

/*!
checksum for output files. It has a method record that add a file hash.
*/
class File : public Object
{
public:
  enum TagUsed { TAG_USED };

  //! Make sure the constructor caller is aware of the tag requirement 
  File(const char* const _name, TagUsed) : Object(_name, L_PRIME) {}

  void record(const char* _flnm);

  static const char* tag() { return "-file"; }
};


}//namespace Checksum
}//namespace Test

#define TEST_CHECKSUM_FILE(VRBL, NAME) \
  Test::Checksum::File VRBL(NAME"-file", Test::Checksum::File::TAG_USED)

#else//TEST_ON

#define TEST_CHECKSUM_FILE(VRBL, NAME) 

#endif//TEST_ON

#endif//BASE_CHECKSUMFILE_HH_INCLUDE
