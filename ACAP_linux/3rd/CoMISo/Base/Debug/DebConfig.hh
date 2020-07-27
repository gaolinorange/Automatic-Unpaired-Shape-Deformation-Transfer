// (C) Copyright 2016 by Autodesk, Inc.

#ifndef BASE_DEBCONFIG_HH_INCLUDED
#define BASE_DEBCONFIG_HH_INCLUDED
#ifdef DEB_ON

#include <Base/Config/BaseDefines.hh>
#include <string>
#include <vector>

namespace Debug {

/*! 
Access the global, per-process, configuration options of the Debug system.
\todo Make this a per-thread configuration.
*/
class BASEDLLEXPORT Config
{
public:
  static const Config& query(); 
  static Config& modify();

public:
  //! Turn on/off the console. This could be extended to redirect it.
  void set_console(const bool _on = true);

  //! Get if the console is turned on/off. 
  bool console() const;

  //! Turn on/off the logfile. This could be extended to provide a FILE*.
  void set_logfile(const bool _on);

  //! Get if the logfile is turned on/off. 
  bool logfile() const;

  //! Get the level for this filename and function
  int level(const char* const _flnm, const char* const _fnct) const;

private:
  class Impl;
  Impl* impl_;

private:
  //! Private constructor
  Config();

  //! Private destructor
  ~Config();

  //! Disable copy
  Config(const Config&);

  //! Disable assignment
  Config& operator=(const Config&);
}; 

};//namespace Debug

#endif//DEB_ON
#endif//BASE_DEBCONFIG_HH_INCLUDED
