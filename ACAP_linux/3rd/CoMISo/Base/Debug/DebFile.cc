// (C) Copyright 2016 by Autodesk, Inc.

#ifdef DEB_ON
#include "DebFile.hh"
#include "DebCallStack.hh"
#include "DebDefault.hh"

#include "Base/Utils/Environment.hh"

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

#if !defined(WIN32) && !defined(_WIN32)
  #define sprintf_s snprintf
#endif

namespace Debug {
namespace {  
// TODO: make this use std::string; check for html extension; case insensitive
bool is_html_filename(const char* const str)
{
  if (str == NULL) return false;
  const char* dot = strrchr(str, '.');
  if (dot == NULL) return false;
  ++dot;
  return (!strncmp(dot, "htm", 3)) || (!strncmp(dot, "HTM", 3)) ;
}
}//namespace

class File::Impl
{
public:
  Impl(const char* const _flnm, const uint _flags) 
    : flags_(_flags), num_flush_(0), line_strt_(false) 
  {
    set_filename(_flnm);
  }

  ~Impl()
  {
    close();
    clear();
  }

  bool is_kept_open() const
  {
    return 0 != (flags_ & KEEP_OPEN);
  }

  bool is_html() const
  {
    return 0 != (flags_ & HTML);
  }

  bool is_retained() const
  {
    return 0 != (flags_ & RETAIN);
  }

  bool is_appended() const
  {
    return 0 != (flags_ & APPEND);
  }
  
  // Only applies to HTML DEB_out
  bool is_white_on_black() const
  {
    return true;
  }

  bool file_is_open() const
  {
    return file_stream_.is_open();
  }

  int priority() const
  {
    return priority_;
  }

  const char* filename() const 
  { 
    return flnm_.empty() ? NULL : flnm_.c_str(); 
  }

  void clear()
  {
    bffr_.clear();
    output_.clear();
    flnm_.clear();
  }

  void print(const char _c, const bool _cnsl = true)
  {
    if (line_strt_)
    {
      line_strt_ = false;
      print(' ', false); // indents never go onto the console!
    }
    bffr_.append(&_c, 1);
    if (_cnsl && console())
      std::cerr << _c; // print on the console
    if (_c == '\n')
    {
      std::cerr << std::flush;
      line_strt_ = true;
    }
  }

  void line_break(const bool _cnsl = true) { print('\n', _cnsl); }

  void print(const std::string& _s, const bool _cnsl = true)
  {
    for (size_t i = 0, n = _s.size(); i < n; ++i)
      print(_s[i], _cnsl);
  }

  void print(const char* const _s, const bool _cnsl = true)
  {
    if (_s == NULL)
      return;
    for (int i = 0, c = _s[0]; c != '\0'; c = _s[++i])
      print((char)c, _cnsl);
  }

  void print(const size_t _i)
  {
    char buffer[128];
#if defined(_MSC_VER) && _MSC_VER < 1900 // MSVC versions older than VC2015
    sprintf_s(buffer, sizeof(buffer), "%Iu", _i);
#else // MSVC 2015 and everything else
    sprintf_s(buffer, sizeof(buffer), "%zu", _i);
#endif
    print(buffer);
  }

  void print(const int _i)
  {
    char buffer[64];
    sprintf_s(buffer, sizeof(buffer), "%i", _i);
    print(buffer);
  }

  const char* double_format() const
  {
    if (double_format_.empty())
      return "%.17g";
    return double_format_.c_str();
  }

  void set_double_format(const char* const str)
  {
    if (str == NULL)
      double_format_.clear();
    else
      double_format_ = str;
  }

  void print(double _d)
  {
    char buffer[64];
    sprintf_s(buffer, sizeof(buffer), double_format(), _d);
    print(buffer);
  }

  void print(const Base::Command& _co)
  {
    switch (_co.cmd)
    {
    case Base::Command::END :
      break;
    case Base::Command::END_ERR :
    case Base::Command::END_LF :
      line_break(); 
      break;
    }
  }

  // Append current asctime to given string
  void add_time(std::string& str) { str += System::Environment::time(); }

#if 1
  bool hover(std::string& _str, const std::string& _hover, const bool _open)
  {
    if (is_html())
    {
      char buffer[1024];
      if (_open)  sprintf_s(buffer, sizeof(buffer),
                              "<span title=\"%s\">", _hover.c_str());
      else sprintf_s(buffer, sizeof(buffer), "</span>");
      _str.append(buffer);
      return true;
    }
    return false;
  }
#endif

  bool anchor(std::string& _str, const int _id, const char* _tag, const bool _open)
  {
    if (is_html())
    {
      char buffer[1024];
      if (_open)  
        sprintf_s(buffer, sizeof(buffer), "<A name=\"%08X_%s\">", _id, _tag);
      else 
        sprintf_s(buffer, sizeof(buffer), "</A>");
      _str.append(buffer);
      return true;
    }
    return false;
  }

  bool link_to(std::string& _str, const int _id, const char* _tag, const std::string& _hover, const bool _open)
  {
    if (is_html())
    {
      char buffer[2048];
      if (_open)
      {
        // HTML title hover text is cropped to 64 char in Firefox but displays
        // OK in Chrome. We could use javascript to avoid this limit but HTML
        // is simpler.
        if (_hover.empty()) sprintf_s(buffer, sizeof(buffer),
                                        "<A href=\"#%08X_%s\">", _id, _tag);
        else sprintf_s(buffer, sizeof(buffer),
                         "<A href=\"#%08X_%s\" title=\"%s\">", _id, _tag, _hover.c_str());
      }
      else sprintf_s(buffer, sizeof(buffer), "</A>");
      _str.append(buffer);
      return true;
    }
    return false;
  }


  void header(std::string& str)
  {
    if (is_html())
    {
      str.append("<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\">");
      str.append("\n<HTML><HEAD>");
      str.append("\n<TITLE>ReForm DEB_out");
      str.append("</TITLE>");
      // javascript lib loads go here
      // stylesheet loads go here
      // within HEAD javascript goes here
      str.append("\n</HEAD>");
      if (is_white_on_black())
      {
        str.append("\n<BODY BGCOLOR=\"#000000\" TEXT=\"#FFFFFF\" LINK=\"#%00FFFF\" VLINK=\"#FFFF00\" >");
        //str.append( "\n<BODY BGCOLOR=\"#000000\" TEXT=\"#FFFFFF\"  >");
      }
      else
      {
        str.append("\n<BODY BGCOLOR=\"#FFFFFF\" TEXT=\"#000000\" LINK=\"#%FF0000\" VLINK=\"#0000FF\" >");
        //str.append( "\n<BODY BGCOLOR=\"#000000\" TEXT=\"#FFFFFF\" >");
      }
      str.append("\n");
    } // endif is_html
    bool date_header = true;
    if (date_header)
    {
      if (!flnm_.empty())
      {
        str.append(flnm_);
        str.append(" opened ");
      }
      add_time(str);
      str.append("[ Build: " __TIME__  " " __DATE__  "] ");
      if (is_html()) str.append("<BR>");
      str.append("\n");
    }
   }

  void footer()
  {
    std::string str("\n");
    if (!flnm_.empty()) 
      str.append(flnm_);
    str.append(" Closed: ");
    add_time(str);
    str.append("\n");
    print(str.c_str());
  }

  bool is_first_flush() { return num_flush_ == 0; }

  bool flush()
  {
    if (bffr_.empty() || !logfile())
      return true;

    const char* const flnm = filename();
    if (flnm == NULL && !file_is_open())
      return false;

    if (!file_is_open())
    {
      file_stream_.open(flnm, std::fstream::out | 
        ((is_appended() && !is_first_flush()) ? 
          std::fstream::app : std::fstream::trunc));
    }

    if (!file_is_open()) // failed opening the file
      return false;

    std::string hdr;
    if (!is_appended())
    {// Re-output entire file
      header(hdr);
      output_.append(hdr);
      file_stream_ << output_;
    }
    else
    {
      if (is_first_flush())
      {
        header(hdr);
        if (is_retained())
          output_.append(hdr);
        file_stream_ << hdr;
      }
      ++num_flush_;
    }

    file_stream_ << bffr_;

    if (is_retained())
      output_.append(bffr_);

    bffr_.clear();
    if (!is_kept_open())
      file_stream_.close();

    return true;
  }

  void close()
  {
    footer();
    flush();
  }

  void set_filename(const char* const _flnm)
  {
    flnm_ = _flnm != NULL ? _flnm : "";
    if (is_html_filename(_flnm))
      flags_ = flags_ | HTML;
  }

  void enter()
  {
    const bool entr_cnsl = false; // do we print enters on the console?
    // First DEB_out in this function so output call-stack, and flush.
    if (!line_strt_)
      line_break(entr_cnsl); // make sure we start on a new line with this
    
    std::string str;
    str.append("*>"); // .txt call stack lead in
    CallStack::query().append(str);
    bffr_.append(str);

    line_break(entr_cnsl); // make sure we start on a new line with this
    flush();
  }

  void set_console(const bool _on = true) { set_flag<CONSOLE>(_on); }
  bool console() const { return flag<CONSOLE>(); }

  void set_logfile(bool _on) { set_flag<LOGFILE>(_on); }
  bool logfile() const { return flag<LOGFILE>(); }

private:
  uint flags_;
  int num_flush_;
  int priority_; // Last permission granted
  bool line_strt_; // are we at the start of th line?

  std::string bffr_;
  std::string output_;
  std::string flnm_;
  std::fstream file_stream_;

  std::string double_format_;

private:

  template <Flags _flag>
  void set_flag(const bool _on)
  {
    flags_ = _on ? flags_ | _flag : flags_ & ~_flag;
  }

  template <Flags _flag>
  bool flag() const { return (flags_ & _flag) == _flag; }

};

//////////////////////////////////////////////////////////////////////////

File& File::modify()
{
  // TODO: Thread-local storage, each (per thread) file in a separate  folder
  static File glbl_file(Default::LOG_FILENAME);
  return glbl_file;
}

const File& File::query() { return modify(); }

File::File(const char* const _flnm, const uint _flags)
  : impl_(new Impl(_flnm, _flags))
{}

File::~File()
{
  delete impl_;
}

void File::enter(const int /*_id*/) { impl_->enter(/*_id*/); }

void File::print(const char _c) { impl_->print(_c); }
void File::print(const char* _s) { impl_->print(_s); }
void File::print(const size_t _i) { impl_->print(_i); }
void File::print(const int _i) { impl_->print(_i); }
void File::print(double _d) { impl_->print(_d); }
void File::print(const Base::Command& _co) { impl_->print(_co); }

const char* File::double_format() const { return impl_->double_format(); }
void File::set_double_format(const char* const str) 
{ 
  impl_->set_double_format(str); 
}

void File::set_console(const bool _on) { impl_->set_console(_on); }
bool File::console() const { return impl_->console(); }

void File::set_logfile(bool _on) { impl_->set_logfile(_on); }
bool File::logfile() const { return impl_->logfile(); }

}//namespace Debug

#endif // DEB_ON
