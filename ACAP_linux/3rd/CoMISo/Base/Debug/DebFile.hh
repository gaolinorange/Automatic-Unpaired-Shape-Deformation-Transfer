// (C) Copyright 2016 by Autodesk, Inc.

#ifndef BASE_DEBFILE_HH_INCLUDED
#define BASE_DEBFILE_HH_INCLUDED
#ifdef DEB_ON

#include <vector>
#include <string>
#include <Base/Utils/IOutputStream.hh>

namespace Debug {
typedef unsigned int uint;
/*!
Debug file.
*/
class File
{
public:
  static const File& query();
  static File& modify();

public:
  enum Flags
  {
    APPEND    = 0x01,
    HTML      = 0x02,
    RETAIN    = 0x04,
    KEEP_OPEN = 0x08,
    CONSOLE   = 0x10,
    LOGFILE   = 0x20,
    DEFAULT = APPEND | RETAIN | CONSOLE | LOGFILE
  };

public:
  File(const char* const _flnm, const uint _flags = DEFAULT);
  ~File();

  void enter(const int _id);

  void print(const char _c);
  void print(const char* _s);
  void print(const size_t _i);
  void print(const int _i);
  void print(double _d);
  void print(const Base::Command& _co);

  const char* double_format() const;
  void set_double_format(const char* const str);

  //! Turn on/off the console. This could be extended to redirect it.
  void set_console(const bool _on = true);

  //! Get if the console is turned on/off. 
  bool console() const;

  //! Turn on/off the logfile. This could be extended to provide a FILE*.
  void set_logfile(const bool _on);

  //! Get if the logfile is turned on/off. 
  bool logfile() const;

private:
  class Impl;
  Impl* impl_;
};

}//namespace Debug

#endif//DEB_ON
#endif//BASE_DEBFILE_HH_INCLUDED
