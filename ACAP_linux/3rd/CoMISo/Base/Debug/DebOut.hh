// (C) Copyright 2014 by Autodesk, Inc.

#ifndef BASE_DEBOUT_HH_INCLUDED
#define BASE_DEBOUT_HH_INCLUDED

#include "Base/Progress/ProgressNode.hh"

// DEB_ON is defined, or not, in CMakeLists.txt for primary project
#ifndef DEB_ON

#define DEB_module(SS)
#define DEB_enter_func PROGRESS_TICK;
#define DEB_only(CC)
#define DEB_exec(LL, AA) { PROGRESS_TICK; } 
#define DEB_exec_if(CC, LL, AA) { PROGRESS_TICK; } 
#define DEB_out(LL, AA) { PROGRESS_TICK; } 
#define DEB_out_if(CC, LL, AA) { PROGRESS_TICK; } 
#define DEB_line(LL, AA) { PROGRESS_TICK; } 
#define DEB_line_if(CC, LL, AA) { PROGRESS_TICK; }

#define DEB_warning(LL, AA) {}
#define DEB_warning_if(CC, LL, AA) {}
#define DEB_error(AA) {} 
#define DEB_error_if(CC, AA) {} 

#else // DEB_ON

#include "Base/Code/CodeLink.hh"
#include "Base/Utils/OStringStream.hh"

#include <string>
#include <sstream>
#include <vector>

// ERROR is occasionally defined in system headers, e.g., windows.h
#ifdef ERROR 
#undef ERROR
#endif//ERROR

namespace Debug {

typedef unsigned int uint;

const int INVALID_LEVEL = -1;
const char* const ERROR = "ERROR";
const char* const WARNING = "WARNING";

/*! 
File is a private implementation for Stream, which is supplies data for several 
other Debug and Test features.
*/
class File;

/*! 
Stream is a specialization for the Debug system streaming.
*/
class BASEDLLEXPORT Stream : public Base::IOutputStream
{
public:
  Stream(File& _file) : file_(_file) {}

  Base::IOutputStream& print(const int);
  Base::IOutputStream& print(const size_t);
  Base::IOutputStream& print(const double);
  Base::IOutputStream& print(const char* const);
  Base::IOutputStream& print(const char);
  Base::IOutputStream& print(const Base::Command&);

private:
  File& file_;

private:
  // inhibit copy
  Stream(const Stream&);
  Stream& operator=(const Stream&);

  friend class Enter;
  friend class Controller;
};

/*!
Enter point in the Debug call stack, also used to filter debug output.
*/
class BASEDLLEXPORT Enter
{
public:
  Enter(const char* const _flnm, const char* const _fnct, int& _nmbr, int& _lvl);
  ~Enter();

  //! pass the output on the level or not?
  bool pass(const int _lvl) const { return _lvl <= lvl_; }
  //! Get the filename
  const char* filename() const { return flnm_; }
  //! Get the function
  const char* function() const { return fnct_; }
  //! Get the number of entries in this point
  int number() const { return nmbr_; }
  //! Get the stream
  Stream& stream();

private:  
  int id_;  //!< Unique identifier for this entry
  const char* flnm_;//!< File name of this entry 
  const char* fnct_;//!< Function name of this entry
  int nmbr_;//!< number of times this entry has occurred 
  int lvl_; //!< permission level for this entry
  /*!
  Number of DEB_* outputs encountered within this entry scope.
  Determines whether a given DEB_* should include or omit a call stack, 
  or an exit trace. 
  */
  int outs_; 
  Stream strm_; //!< Stream
};

BASEDLLEXPORT extern void warning(const std::string& _wrng, const ::Base::CodeLink& _lnk);
BASEDLLEXPORT extern void error(const std::string& _err, const ::Base::CodeLink& _lnk);

// TODO: Move this upstream to namespace Base?!
template <typename T>
std::string to_string(const T& _t)
{
  std::ostringstream ss;
  ss << _t;
  return ss.str();
}

}//namespace Debug

// Obsolete, use gradually removed
#define DEB_module(MODULE)

//TODO: This should use an atomic thread-safe static int(s)
#define DEB_enter_func \
  PROGRESS_TICK; \
  static int deb_nmbr = 0; \
  static int deb_lvl = Debug::INVALID_LEVEL; \
  ::Debug::Enter deb(__FILE__, __FUNCTION__, deb_nmbr, deb_lvl);

#define DEB_only(CC) CC

#define DEB_exec(LL, AA) DEB_exec_if(true, LL, AA)
#define DEB_exec_if(CC, LL, AA) { PROGRESS_TICK; \
  { if (deb.pass(LL) && (CC)) { AA; } } }

#define DEB_out(LL, AA) DEB_out_if(true, LL, AA)
#define DEB_out_if(CC, LL, AA) { PROGRESS_TICK; \
  { if (deb.pass(LL) && (CC)) { deb.stream() << AA << ::Base::Command::END; } } }

#define DEB_line(LL, AA) DEB_line_if(true, LL, AA)
#define DEB_line_if(CC, LL, AA) { PROGRESS_TICK; \
  { if (deb.pass(LL) && (CC)) { deb.stream() << AA << ::Base::LF; } } }

#define DEB_warning(LL, AA) DEB_warning_if(true, LL, AA)
#define DEB_warning_if(CC, LL, AA) \
  { if (deb.pass(LL) && (CC)) { ::Base::OStringStream strm; strm << AA; \
      ::Debug::warning(strm.str, BASE_CODELINK); } } 

#define DEB_error(AA) { ::Base::OStringStream strm; strm << AA; \
    ::Debug::error(strm.str, BASE_CODELINK); }
#define DEB_error_if(CC, AA) { if (CC) DEB_error(AA); }

// Debug::Stream does not fulfill ostream. If you want to exploit an existing
// ostream streamer to DEB_out a class as text without exploiting any
// numeric processing or custom Stream streamers then use this macro thus
// DEB_out(1, "my_class is " << DEB_os_str(my_c) )

#define DEB_os_str(AA) Debug::to_string(AA)

#endif // DEB_ON
#endif // BASE_DEBOUT_HH_INCLUDED
