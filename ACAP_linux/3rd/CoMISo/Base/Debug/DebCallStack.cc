// (C) Copyright 2016 by Autodesk, Inc.

#ifdef DEB_ON

#include "DebCallStack.hh"
#include "DebOut.hh"
#include <cstring>

#ifndef WIN32
#include <cstdio>
#endif

namespace Debug {
namespace {  

// Replace interior of < > in function name with . for brevity
void append_function(std::string& _str, const char* const _func)
{
  int cnt = 0;
  const char* ptr = _func;
  while (ptr && (*ptr != '\0'))
  {
    char c = *ptr;
    if (c == '>') --cnt;
    if (cnt == 0) _str.append(1, c);
    if (c == '<')
    {
      if (cnt == 0) _str.append(".");
      ++cnt;
    }
    ++ptr;
  }
}

// Add to the string the call stack element string
void append_entry(std::string& _str, const Enter* const _entr, 
  const bool _cmpct, const bool _entr_nmbr, const Enter* const _prev_entr)
{
  if (_prev_entr != NULL && // there is a previous entry and it is the same?
    strcmp(_prev_entr->function(), _entr->function()) == 0)
  {// ... so do nothing
    return;
  }

  if (_prev_entr != NULL)
    _str.append("/");

  if (_cmpct)
    append_function(_str, _entr->function());
  else
    _str.append(_entr->function());

  if (_entr_nmbr)
  {
    _str.append(".");
    std::stringstream ss;
    ss << _entr->number();
    _str.append(ss.str());
  }
}

}//namespace


CallStack& CallStack::modify()
{
  static CallStack glbl_call_stck;
  return glbl_call_stck;
}

const CallStack& CallStack::query()
{
  return modify();
}

// Read a particular call stack element
//bool read(int _up, const char*& _funcname, int& _count)
//{
//  const Enter* fcs = call(_up);
//  if (fcs != NULL)
//  {
//    _funcname = fcs->function();
//    _count = fcs->number(); // Return most recent deb_enter_count
//    return true;
//  }
//  return false;
//}
//

void CallStack::append(std::string& _str, const bool _entr_nmbr) const
{
  const Enter* prev = NULL;
  for (size_t i = 0, n = depth(); i < n; prev = calls_[i++])
    append_entry(_str, calls_[i], true, _entr_nmbr, prev);
}

void CallStack::append_indent(std::string& _str, const int _indt, 
  const bool _html) const
{
  if (_indt == 0) 
    return;
  if (_html)
  {
    char buffer[64];
#ifndef WIN32
    sprintf(buffer, "<FONT SIZE=%i>", _indt);
#else
    sprintf_s(buffer, sizeof(buffer), "<FONT SIZE=%i>", _indt);
#endif
    _str.append(buffer);
  }
  int num = (int)calls_.size();
  int i0 = 0;
  if (!_html) 
    ++i0; // Don't waste whitespace on first level indent if .txt
  for (int i = i0; i < num; ++i)
    _str.append("  ");
  if (_html) 
    _str.append(":&nbsp;</FONT>\n");
}

}//namespace Debug

#endif//DEB_ON
