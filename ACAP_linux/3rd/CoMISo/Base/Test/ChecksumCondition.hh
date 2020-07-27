// (C) Copyright 2016 by Autodesk, Inc.

#ifndef REFORM_CHECKSUMCONDITION_HH_INCLUDED
#define REFORM_CHECKSUMCONDITION_HH_INCLUDED

#include <Base/Debug/DebOut.hh>
#include <Base/Test/Checksum.hh>

#if defined(TEST_ON)

// The functions in this file are used to count the number of errors and warnings
// and makes sense only if both the debug and test macro are on.

namespace Base {
struct CodeLink;
}//namespace Base

namespace Test {
namespace Checksum {

class Condition : public Object
{
public:
  Condition() : Object("Condition", L_STABLE), nmbr_(0), fail_nmbr_(0) {}
    
  virtual void record(const char* const _cndt, const Base::CodeLink& _lnk, 
    const bool _rslt);
  virtual void record_number();

protected:
  //! Implement "smarter" comparison
  virtual Difference compare_data(const String& _old, const String& _new) const;

protected:
  int nmbr_; //number of all checked conditions
  int fail_nmbr_; // number of failed checked conditions
};

extern Condition condition;

}//namespace Checksum
}//namespace Test

#endif//defined(TEST_ON)

#endif//REFORM_CHECKSUMCONDITION_HH_INCLUDED
