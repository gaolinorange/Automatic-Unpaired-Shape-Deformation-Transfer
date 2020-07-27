// (C) Copyright 2016 by Autodesk, Inc.

#if defined(TEST_ON) && defined(DEB_ON)

#include "ChecksumDebugEvent.hh"
#include "TestResult.hh"
#include "Base/Code/CodeLink.hh"
#include "Base/Debug/DebOut.hh"

#include <algorithm>

namespace Test {
namespace Checksum {
namespace Debug {

// put test checksum tag and separators in the required format
const char* const LINK_MARK = " reported ";
const char* const PREFIX_MARK = ": ";

void Event::record(const std::string& _evnt, const Base::CodeLink& _lnk)
{
  Base::OStringStream strm;
  strm << "#" << nmbr_++ << ": " << _evnt << LINK_MARK << _lnk;
  add(name() == ::Debug::ERROR ? Result::ERROR : Result::WARNING, 
    strm.str);
}

void Event::record_number()
{
  Base::OStringStream strm;
  strm << "total#: " << nmbr_;
  add(
    nmbr_ == 0 ? Result::OK : 
      (name() == ::Debug::ERROR ? Result::ERROR : Result::WARNING), 
    strm.str);
}

Difference Event::compare_data(const String& _old, const String& _new) const
{
  // exclude the code link from the comparison
  auto old_tmp = _old;
  auto new_tmp = _new;
  const auto old_link_pos = old_tmp.rfind(LINK_MARK);
  const auto new_link_pos = new_tmp.rfind(LINK_MARK);

  if (old_link_pos != String::npos && new_link_pos != String::npos)
  {
    old_tmp.resize(old_link_pos);
    new_tmp.resize(new_link_pos);
  }

  // exclude the event prefix, e.g., number from the comparison
  auto old_prfx_pos = old_tmp.find(PREFIX_MARK);
  auto new_prfx_pos = new_tmp.find(PREFIX_MARK);
  if (old_prfx_pos != String::npos && new_prfx_pos != String::npos)
  {// remove the prefix and add white-spaces instead
    String prfx(new_prfx_pos, ' '); // align wrt the new result
    old_tmp = prfx + String(old_tmp.begin() + old_prfx_pos, old_tmp.end());
    new_tmp = prfx + String(new_tmp.begin() + new_prfx_pos, new_tmp.end());
  }
  return Object::compare_data(old_tmp, new_tmp);
}


Event error(::Debug::ERROR);
Event warning(::Debug::WARNING);

}//namespace Debug
}//Checksum
}//namespace Test

#endif//defined(TEST_ON) && defined(DEB_ON)
