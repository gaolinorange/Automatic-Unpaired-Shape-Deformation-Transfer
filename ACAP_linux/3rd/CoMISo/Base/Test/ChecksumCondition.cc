// (C) Copyright 2016 by Autodesk, Inc.

#if defined(TEST_ON)

#include "ChecksumCondition.hh"
#include "TestResult.hh"
#include "Base/Code/CodeLink.hh"

#include <algorithm>

namespace Test {
namespace Checksum {

const char* const CHECKED_TAG = " checked ";
const char* const TOTAL_TAG = "total#: ";
const char* const FAILED_TAG = "failed#: ";

void Condition::record(const char* const _cndt, const Base::CodeLink& _lnk, 
  const bool _rslt)
{
  Base::OStringStream strm;
  strm << _cndt << CHECKED_TAG << _lnk;
  add(_rslt ? Result::OK : Result::ERROR, strm.str);
  ++nmbr_;
  fail_nmbr_ += int(!_rslt);
}

void Condition::record_number()
{
  if (nmbr_ == 0)
    return;
  {
    Base::OStringStream strm;
    strm << TOTAL_TAG << nmbr_;
    add(Result::OK, strm.str);
  }
  {
    Base::OStringStream strm;
    strm << FAILED_TAG << fail_nmbr_;
    add(fail_nmbr_ == 0 ? Result::OK : Result::ERROR, strm.str);
  }
}

Difference Condition::compare_data(const String& _old, const String& _new) const
{
  // exclude the code link from the comparison
  auto old_tmp = _old;
  auto new_tmp = _new;
  const auto old_link_pos = old_tmp.rfind(CHECKED_TAG);
  const auto new_link_pos = new_tmp.rfind(CHECKED_TAG);

  if (old_link_pos != String::npos && new_link_pos != String::npos)
  {
    old_tmp.resize(old_link_pos);
    new_tmp.resize(new_link_pos);
  }

  // exclude the Condition prefix, e.g., number from the comparison
  //auto old_prfx_pos = old_tmp.find(PREFIX_MARK);
  //auto new_prfx_pos = new_tmp.find(PREFIX_MARK);
  //if (old_prfx_pos != String::npos && new_prfx_pos != String::npos)
  //{// remove the prefix and add white-spaces instead
  //  String prfx(new_prfx_pos, ' '); // align wrt the new result
  //  old_tmp = prfx + String(old_tmp.begin() + old_prfx_pos, old_tmp.end());
  //  new_tmp = prfx + String(new_tmp.begin() + new_prfx_pos, new_tmp.end());
  //}

  return Object::compare_data(old_tmp, new_tmp);
}


Condition condition;

}//Checksum
}//namespace Test

#endif//defined(TEST_ON)
