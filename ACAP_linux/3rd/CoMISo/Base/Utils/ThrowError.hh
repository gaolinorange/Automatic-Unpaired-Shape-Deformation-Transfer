// (C) Copyright 2015 by Autodesk, Inc.

#ifndef BASE_THROWINFO_HH_INCLUDED
#define BASE_THROWINFO_HH_INCLUDED

// Define the relevant macros to throw
#ifdef DEB_ON

#include <Base/Debug/DebOut.hh>
#include <Base/Code/CodeLink.hh>

namespace Base {
  class IOutputStream;
}

namespace Debug {

//! Basic class to define auxiliary debug info about a throw event 
struct ThrowInfo
{
  typedef Base::CodeLink CodeLink;

  ThrowInfo(const char* _modl, const CodeLink& _lnk)
    : modl(_modl), lnk(_lnk)
  {}

  const char* modl;
  CodeLink lnk;
};

Base::IOutputStream& operator<<(Base::IOutputStream& _ds, 
  const ThrowInfo& _thrw_info);

//! Extend a generic (code-based) Error exception with ThrowInfo
template <class ErrorT>
struct ErrorExtT : public ErrorT, public ThrowInfo
{
  ErrorExtT(const typename ErrorT::Index _err_idx, const ThrowInfo& _thrw_info)
    : ErrorT(_err_idx), ThrowInfo(_thrw_info)
  {}
};

}//namespace Debug


#define THROW_ERROR_MODULE_VAR(MODULE, VAR) { \
  throw Debug::ErrorExtT<MODULE::Error>(VAR, \
    Debug::ThrowInfo(#MODULE, BASE_CODELINK)); }

#define THROW_ERROR_MODULE(MODULE, INDEX) \
  THROW_ERROR_MODULE_VAR(MODULE, MODULE::Error::INDEX)

#define THROW_ERROR_DEB_MODULE(MODULE, INDEX, EXPR) { DEB_error(EXPR); \
  THROW_ERROR_MODULE(MODULE, INDEX) }

#else// DEB_ON

#define THROW_ERROR_MODULE_VAR(MODULE, VAR) { throw MODULE::Error(VAR); }

#define THROW_ERROR_MODULE(MODULE, INDEX) \
  THROW_ERROR_MODULE_VAR(MODULE, MODULE::Error::INDEX)

#define THROW_ERROR_DEB_MODULE(MODULE, INDEX, EXPR) \
  THROW_ERROR_MODULE(MODULE, INDEX)

#endif// DEB_ON

#define THROW_ERROR_TODO_MODULE(MODULE, EXPR) \
  THROW_ERROR_DEB_MODULE(MODULE, TODO, EXPR)


#endif//BASE_THROWINFO_HH_INCLUDED
