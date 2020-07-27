// (C) Copyright 2015 by Autodesk, Inc.

//=============================================================================
//
//  CLASS StopWatch
//
//=============================================================================

#ifndef BASE_STOPWATCH_HH_INCLUDED
#define BASE_STOPWATCH_HH_INCLUDED

#include <Base/Config/BaseDefines.hh>

namespace Base {

//== CLASS DEFINITION =========================================================
    
/** \class StopWatch StopWatch.hh 

    This class can be used for measuring time, providing optimal granularity
    for the current platform. 
**/

class BASEDLLEXPORT StopWatch
{
public:

  /// Constructor
  StopWatch(); 

  /// Destructor
  ~StopWatch();

  /// Start time measurement
  void start();

  /// Restart, return time elapsed until now.
  double restart();

  /// Stop time measurement, return time.
  double stop();

  /// Get the total time in milli-seconds (watch has to be stopped).
  double elapsed() const;

private:
  class Impl;
  Impl* impl_;

private:
  // disable copy and assignment
  StopWatch(const StopWatch&);
  StopWatch& operator=(const StopWatch&);
};


//=============================================================================
} // namespace Base

//=============================================================================
#endif//BASE_STOPWATCH_HH_INCLUDED
//=============================================================================

