// (C) Copyright 2015 by Autodesk, Inc.

#include <Base/Security/Mandatory.hh>
#include "StopWatch.hh"

#ifdef _MSC_VER

// Windows implementation
//////////////////////////////////////////////////////////////////////////

INSECURE_INCLUDE_SECTION_BEGIN
#include <windows.h>
INSECURE_INCLUDE_SECTION_END

namespace Base {

class StopWatch::Impl
{
public:
  Impl() 
  { 
    QueryPerformanceFrequency(&freq_); 
  }

  void start() 
  {
    QueryPerformanceCounter(&starttime_);
  }

  void stop()
  {
    QueryPerformanceCounter(&endtime_);
  }

  double elapsed() const 
  {
    return (double)(endtime_.QuadPart - starttime_.QuadPart) / 
      (double)freq_.QuadPart * 1000.0f;
  }

private:
  LARGE_INTEGER starttime_, endtime_;
  LARGE_INTEGER freq_;
};

} //namespace Base

#else 

// Linux implementation
//////////////////////////////////////////////////////////////////////////

#include <sys/time.h>

namespace Base {


class StopWatch::Impl
{
public:
  Impl()
  { 
    starttime_.tv_sec = starttime_.tv_usec = 0;
    endtime_.tv_sec   = endtime_.tv_usec = 0;
  }

  void start() 
  {
    starttime_ = current_time();
  }

  void stop() 
  {
    endtime_ = current_time();
  }

  /// Get the total time in milli-seconds (watch has to be stopped).
  double elapsed() const 
  {
    return ((endtime_.tv_sec  - starttime_.tv_sec )*1000.0 +
	    (endtime_.tv_usec - starttime_.tv_usec)*0.001);
  }

private:
  timeval starttime_, endtime_;

private:
  static timeval current_time()
  {
    struct timeval tv;
    gettimeofday(&tv, 0);
    return tv;
  }
};

} //namespace Base

#endif//

// StopWatch interface implementation
//////////////////////////////////////////////////////////////////////////

namespace Base {

StopWatch::StopWatch() 
  : impl_(new Impl) 
{}

StopWatch::~StopWatch() 
{ delete impl_; }

void StopWatch::start()
{
  impl_->start();
}

double StopWatch::restart()
{
  double t = stop(); 
  start();
  return t;
}

double StopWatch::stop()
{
  impl_->stop();
  return elapsed();
}

double StopWatch::elapsed() const
{
  return impl_->elapsed();
}

} // namespace Base
