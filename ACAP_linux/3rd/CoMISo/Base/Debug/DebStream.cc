// (C) Copyright 2014 by Autodesk, Inc.

#include "DebUtils.hh"
#include "DebFile.hh"
#include "DebCallStack.hh"
#include "DebConfig.hh"
#include "Base/Code/CodeLink.hh"
#include "Base/Utils/ThrowError.hh"
#include "Base/Utils/Environment.hh"
#include "Base/Test/ChecksumDebugEvent.hh"

#ifdef DEB_ON

#include <string>
#include <fstream>
#include <time.h>
#include <vector>
#include <iostream>
#include <map>
#include <memory>
#include <list>
#include <map>
#include <sstream>
#include <cstring>
#include <time.h>

#ifndef WIN32
  #define sprintf_s snprintf
#endif

namespace Debug {

void warning(const std::string& _wrng, const Base::CodeLink& _lnk)
{
  TEST(Test::Checksum::Debug::warning, record(_wrng, _lnk));
  Stream strm(File::modify());
  strm << WARNING << ": " << _wrng << ' ' << _lnk << Base::LF;
}

void error(const std::string& _err, const Base::CodeLink& _lnk)
{
  TEST(Test::Checksum::Debug::error, record(_err, _lnk));
  Stream strm(File::modify());
  strm << ERROR << ": " << _err << ' ' << _lnk << Base::Command::END_ERR;
}

//////////////////////////////////////////////////////////////////////////

Enter::Enter(const char* const _flnm, const char* const _fnct,
  int& _nmbr, int& _lvl)
  : flnm_(_flnm), fnct_(_fnct), outs_(0), strm_(File::modify())
{// TODO: for thread-safety we will need to make the constructor body atomic!
  CallStack::modify().push(this);

  nmbr_ = _nmbr++;

  if (_lvl == INVALID_LEVEL)
    _lvl = Config::query().level(flnm_, fnct_);
  lvl_ = _lvl;

  static int id_cnt = 0;
  id_ = ++id_cnt;
}

Enter::~Enter()
{
  CallStack::modify().pop();

  /* html support only disabled
  std::string str;
  File& file = strm_.file_;
  if (((outs_ > 0) || (lns_ > 0)) && file.anchor(str, id_, "exit", true))
  {
    file.anchor(str, id_, "exit", false);
    file.print_direct(str);
  }
  */
}

Stream& Enter::stream()
{
  File& file = strm_.file_;
  /* html support only disabled
  if (file.is_html())
  {
    // bool is_deb_error = (_warn == 2);
    // DEB_error font powerup goes here. BLINK is deprecated sadly.
    // if (is_deb_error)  file.print_direct("<BLINK>");
    const int col = 0xFF0000; // RED
    char buffer[256];
    sprintf_s(buffer, sizeof(buffer), "<FONT COLOR=\"#%06X\" SIZE=%i>",
              col, file.priority() + 1);
    file.print_direct(buffer);
  }
  */
  if (outs_ < 1)
    file.enter(id_);
  ++outs_;

  return strm_;
}

// =====================================
//        Stream member funcs
// =====================================

Stream::Stream(const Stream& _othr) : file_(_othr.file_) {}
Stream& Stream::operator = (const Stream&) { return *this; }


Base::IOutputStream& Stream::print(const int _i)
{
  file_.print(_i);
  return *this;
};

Base::IOutputStream& Stream::print(const size_t _i)
{
  file_.print(_i);
  return *this;
};

Base::IOutputStream& Stream::print(const double _d)
{
  file_.print(_d);
  return *this;
};

Base::IOutputStream& Stream::print(const char* _s)
{
  file_.print(_s);
  return *this;
};

Base::IOutputStream& Stream::print(const char _c)
{
  file_.print(_c);
  return *this;
};

Base::IOutputStream& Stream::print(const Base::Command& _co)
{
  file_.print(_co);
  return *this;
};

// ==================================================
//  Controller member funcs (currently all static)
// ==================================================

void Controller::set_double_format(const char* _frmt)
{
  File::modify().set_double_format(_frmt);
}

const char* Controller::double_format()
{
  return File::modify().double_format();
}

}//namespace Debug


#endif // DEB_ON
