// (C) Copyright 2016 by Autodesk, Inc.

#ifndef BASE_CHECKSUMCOUNT_HH_INCLUDE
#define BASE_CHECKSUMCOUNT_HH_INCLUDE

#include <Base/Debug/DebOut.hh>
#include <Base/Test/Checksum.hh>

#if defined(TEST_ON) && defined(DEB_ON)

// The functions in this file are used to count the number of errors and warnings
// and makes sense only if both the debug and test macro are on.

namespace Base {
struct CodeLink;
}//namespace Base

namespace Test {
namespace Checksum {
namespace Debug {

class Event : public Object
{
public:
  Event(const char* _name) : Object(_name), nmbr_(0) {}

  virtual void record(const std::string& _evnt, const Base::CodeLink& _lnk);
  virtual void record_number();

protected:
  //! Implement "smarter" comparison
  virtual Difference compare_data(const String& _old, const String& _new) const;

protected:
  int nmbr_;
};

extern Event error;
extern Event warning;

}//namespace Debug
}//namespace Checksum
}//namespace Test

#endif//defined(TEST_ON) && defined(DEB_ON)

#endif//BASE_CHECKSUMCOUNT_HH_INCLUDE

