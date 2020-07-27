// (C) Copyright 2016 by Autodesk, Inc.

#ifdef TEST_ON

#include "Checksum.hh"
#include "Base/Debug/DebCallStack.hh"
#include <fstream>
#include <iostream>
#include <mutex>

namespace Test {
namespace Checksum {

Level run_lvl = L_NONE;

namespace {

Registry& registry_modify()
{
  static Registry chksm_reg;
  return chksm_reg;
}

}//namespace

const Registry& registry()
{
  return registry_modify();
}

///////////////////////////////////////////////////////////////////////////////
// class Checksum implementation

Object::Object(const char* const _name, const Level _lvl)
  : lvl_(_lvl), name_(_name)
{
  auto pos = registry_modify().emplace(name_, this);
  if (!pos.second)
  {
    std::cout << "Duplicate checksum definition: " << _name << std::endl;
    throw;
  }
}

Difference Object::compare_data(const String& _old, const String& _new) const
{
  if (_old == _new)
    return Difference::EQUAL;

  String diff;
  diff.resize((std::max(_old.length(), _new.length())));
  for (int i = 0; i < diff.size(); ++i)
  {
    diff[i] = i < _old.length() && i < _new.length() && _old[i] == _new[i] ?
      ' ' : '*';
  }
  return Difference(Difference::UNKNOWN, diff);
}

Difference Object::compare(
  const Path& /*_old_path*/,
  const Record& _old_rcrd,
  const Path& /*_new_path*/,
  const Record& _new_rcrd
  ) const
{
  const auto old_rslt_type = _old_rcrd.rslt.type();
  const auto new_rslt_type = _new_rcrd.rslt.type();
  auto data_diff = compare_data(_old_rcrd.data, _new_rcrd.data);
  if (old_rslt_type == new_rslt_type) // same result, so just data compare
    return data_diff;

  // result types are different, qualify the difference
#define DIFFERENCE(TYPE) data_diff += Difference(Difference::TYPE)
  switch (old_rslt_type)
  {
  case Result::OK: 
    switch (new_rslt_type)
    {
    case Result::WARNING: DIFFERENCE(SUSPICIOUS); break;
    case Result::ERROR: DIFFERENCE(REGRESSED); break;
    default: DIFFERENCE(FAILED); break; // FAILURE, CRASH, HANG
    };
    break;
  case Result::WARNING : 
    switch (new_rslt_type)
    {
    case Result::OK: DIFFERENCE(IMPROVED); break;
    case Result::ERROR: DIFFERENCE(REGRESSED); break;
    default: DIFFERENCE(FAILED); break; // FAILURE, CRASH, HANG
    };
    break;
  case Result::ERROR : 
    switch (new_rslt_type)
    {
    case Result::OK:
    case Result::WARNING: DIFFERENCE(IMPROVED); break;
    default: DIFFERENCE(FAILED); break; // FAILURE, CRASH, HANG
    };
    break;
  case Result::FAILURE: 
    switch (new_rslt_type)
    {
    // worked with or w/o issues, now fails
    case Result::OK:
    case Result::WARNING: 
    case Result::ERROR: DIFFERENCE(WORKED); break;
    // gracious failure replaced by HANG or CRASH?!
    default: DIFFERENCE(FAILED); break; // CRASH, HANG
    };
    break;
  case Result::CRASH: 
    switch (new_rslt_type)
    {
    case Result::OK:
    case Result::WARNING: 
    case Result::ERROR: DIFFERENCE(WORKED); break;
    // CRASH replaced by gracious failure!
    case Result::FAILURE: DIFFERENCE(IMPROVED); break; 
    default: DIFFERENCE(FAILED); break; // CRASH replaced by HANG?!
    };
    break;
  case Result::HANG: 
    switch (new_rslt_type)
    {
    case Result::OK:
    case Result::WARNING: 
    case Result::ERROR: DIFFERENCE(WORKED); break;
    // HANG replaced by gracious failure!
    case Result::FAILURE: DIFFERENCE(IMPROVED); break; 
    case Result::CRASH: DIFFERENCE(SUSPICIOUS); break; // HANG is now CRASH!
    default: ; // disable warnings
    };
    break;
  }
  return data_diff;
}

void Object::add(const Result& _rslt, const String& _data)
{
  static std::mutex mtx; //synchronize access to the checksum report stream
  std::lock_guard<std::mutex> lock(mtx);

  static Base::OutputStreamAdaptT<std::ofstream> test_str(REPORT_FILENAME);
  static bool tag_out = false;

#ifdef DEB_ON
  static String prev_call_stck;
  String call_stck("/");
  Debug::CallStack::query().append(call_stck);

  if (prev_call_stck != call_stck)
  {
    test_str << call_stck << Base::LF;
    prev_call_stck = call_stck;
  }
#endif//DEB_ON

  if (!tag_out)
  {
    tag_out = true;
    test_str << REPORT_LEVEL_TAG << LEVEL_TEXT[run_lvl] << Base::LF;
  }
  test_str << _rslt << "   " << name() << ": " << _data << Base::LF;
}

}//Checksum
}//namespace Test

#endif//TEST_ON
