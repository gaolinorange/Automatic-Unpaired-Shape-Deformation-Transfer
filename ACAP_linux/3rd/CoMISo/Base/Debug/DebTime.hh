// (C) Copyright 2015 by Autodesk, Inc.

#ifndef BASE_DEBTIME_HH_INCLUDED
#define BASE_DEBTIME_HH_INCLUDED

#include <Base/Utils/StopWatch.hh>
#include <Base/Debug/DebOut.hh>
#include <Base/Utils/Exception.hh>

#ifdef DEB_ON

namespace Debug {

class StopWatchSession
{
public:
  StopWatchSession(Enter& _deb, const char* _sssn_name = NULL,
    const int _deb_lvl = 2) 
    : deb(_deb), sssn_name_(_sssn_name), deb_lvl_(_deb_lvl)
  {
    sw_.start();
  }

  ~StopWatchSession() NOEXCEPT(false)
  {
    // TODO: implement "prettier" DEB out if seconds turn into minutes/hours/etc
    DEB_line(deb_lvl_, sssn_name_ << " took " << sw_.stop()/1000.0 << " s.");
  }

private:
  Enter& deb; // intentional variable name match with the DEB_marcos!
  const char* sssn_name_;
  const int deb_lvl_;
  Base::StopWatch sw_;

private:
  // disable copy and assignment
  StopWatchSession(const StopWatchSession&);
  StopWatchSession& operator=(const StopWatchSession&);
};

} //namespace Debug

#define DEB_time_session(SSSN, LL) PROGRESS_TICK; \
  Debug::StopWatchSession __sw_sssn(deb, SSSN, LL);

#define DEB_time_session_def(SSSN) PROGRESS_TICK; \
  Debug::StopWatchSession __sw_sssn(deb, SSSN, 2);

#define DEB_time_func(LL) DEB_enter_func \
  Debug::StopWatchSession __sw_func(deb, __FUNCTION__, LL);

#define DEB_time_func_def DEB_time_func(2)

#else

#define DEB_time_session(SSSN, LL) PROGRESS_TICK;

#define DEB_time_session_def(SSSN) PROGRESS_TICK;

#define DEB_time_func(LL) PROGRESS_TICK;

#define DEB_time_func_def PROGRESS_TICK;

#endif // DEB_ON

#endif//BASE_DEBTIME_HH_INCLUDED
