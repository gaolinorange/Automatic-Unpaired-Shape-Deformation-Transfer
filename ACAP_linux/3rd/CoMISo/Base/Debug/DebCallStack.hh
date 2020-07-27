// (C) Copyright 2016 by Autodesk, Inc.

#ifndef BASE_DEBCALLSTACK_HH_INCLUDED
#define BASE_DEBCALLSTACK_HH_INCLUDED
#ifdef DEB_ON

#include <string>
#include <vector>

namespace Debug {
  
class Enter;
typedef unsigned int uint;

//! A stack of Debug entry points, a partial record of the real call stack
class CallStack
{
public: 
  static const CallStack& query();
  static CallStack& modify();
  
public:
  bool empty() const { return calls_.empty(); }
  size_t depth() const { return calls_.size(); }

  void push(const Enter* const _entr) { calls_.push_back(_entr); }
  void pop() { calls_.pop_back(); }

  const Enter* call(const int _up = 0) const
  {
    size_t n = calls_.size();
    if (_up < (int)n)
      return calls_[n - 1 - _up];
    else
      return NULL;
  }

  /*!
  Append the full call stack.
  */
  void append(std::string& _str, const bool _entr_nmbr = true) const;

  /*!
  Append and indent to the string
  */
  void append_indent(std::string& _str, const int _indt, const bool _html
    ) const;

private:
  std::vector<const Enter*> calls_;

private:
  CallStack() {}
  ~CallStack() {}

  CallStack(const CallStack&);
  CallStack& operator=(const CallStack&);
}; 

};//namespace Debug

#endif//DEB_ON
#endif//BASE_DEBCALLSTACK_HH_INCLUDED
