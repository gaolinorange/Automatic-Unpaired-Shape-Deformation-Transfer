// (C) Copyright 2016 by Autodesk, Inc.

#ifndef BASE_TESTOUTCOME_HH_INCLUDED
#define BASE_TESTOUTCOME_HH_INCLUDED

#ifdef TEST_ON

#include <Base/Test/types.hh>

namespace Test {

/*!
Representation of a result in the test system, e.g., for a checksum, test run, 
test list report, comparison.

This might hold more information in the future, such as number of errors and
warnings reported, etc. Hence it is more future-proof than an enum.
*/
class Result
{ 
public:
  enum Type // keep descriptions updated.
  {
    OK,   
    WARNING,
    ERROR,
    FAILURE,
    CRASH,
    HANG
  };

public:
  Result(Type _type = OK) : type_(_type) {}

  // Short Result description!
  template <class StreamT>
  friend inline StreamT& operator>>(StreamT& _is, Result& _rslt)
  {
    char c;
    _is >> c;
    size_t descr_nmbr;
    auto descr = descriptions(&descr_nmbr);
    for (size_t i = 0; i < descr_nmbr; ++i)
    {
      if (descr[i][0] == c)
      {
        _rslt = Result(Type(i));
        return _is;
      }
    }
    _rslt = Result(OK);
    return _is;
  }

  template <class StreamT>
  friend inline StreamT& operator<<(StreamT& _os, const Result& _rslt)
  {
    _os << descriptions()[_rslt.type()][0];
    return _os;
  }

  // Return a Result description!
  const char* message() const { return descriptions()[type_]; }

  Type type() const { return type_; }

  bool ok() const { return type() == OK; }

  //! Update the type to the more severe type from the input and this
  Result& operator+=(const Type _type)
  {
    if (type_ < _type)
      type_ = _type;
    return *this;
  }

  Result& operator+=(const Result& _rslt) { return *this += _rslt.type_; }

  friend inline bool operator<(const Result& _a, const Result& _b)
  {
    return _a.type() < _b.type();
  }

  friend inline bool operator!=(const Result& _a, const Result& _b)
  {
    return _a.type() != _b.type();
  }

private: 
  Type type_;

private:
  static const char** descriptions(size_t* _nmbr = NULL)
  {
    // There is a long and a short description. The short one is the first letter
    // for the long one. For OK it is desirable to have a blank as short description,
    // and this is the reason of the extra blank in " OK".
    static const char* descr[] =
    { " OK", "WARNING", "ERROR", "FAILURE" , "CRASH", "HANG" };
    if (_nmbr != NULL)
      *_nmbr = std::end(descr) - std::begin(descr);
    return descr;
  }
}; 

}//namespace Test

#endif//TEST_ON
#endif//BASE_TESTOUTCOME_HH_INCLUDED
