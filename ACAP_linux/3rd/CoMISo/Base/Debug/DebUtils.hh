// (C) Copyright 2014 by Autodesk, Inc.

#ifndef BASE_DEBUTILS_HH_INCLUDED
#define BASE_DEBUTILS_HH_INCLUDED

#include <Base/Debug/DebOut.hh>

#ifdef DEB_ON

namespace Debug {

/*! A currently empty control interface class onto the DebFile class(es)
exploited by DebStream. */
class Controller
{
public:
  //! Set printf style format string used for double (and float) numeric print
  static void set_double_format(const char* _fmt);
  static const char* double_format();
};

class DoubleFormatSession
{
public:
  DoubleFormatSession(const char* _fmt) 
    : fmt_bck_(Controller::double_format())
  {
    Controller::set_double_format(_fmt);
  }

  ~DoubleFormatSession()
  {
    Controller::set_double_format(fmt_bck_.data());
  }

private:
  std::string fmt_bck_;
};

}// namespace Debug

//#define DEB_set_module_level(MM, LL) { Controller::set_module_level(MM, LL); }
//#define DEB_set_double_format(FF) { Controller::set_double_format(FF); }
#define DEB_double_format(FF) Debug::DoubleFormatSession double_format(FF);

#else

//#define DEB_set_module_level(MM, LL) {}
//#define DEB_set_double_format(FF) {}

#define DEB_double_format(FF)

#endif

#endif // BASE_DEBUTILS_HH_INCLUDED
