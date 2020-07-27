// (C) Copyright 2015 by Autodesk, Inc.

#ifndef BASE_ENVIRONMENT_HH_INCLUDED
#define BASE_ENVIRONMENT_HH_INCLUDED

#include <string>
#include <locale>

namespace System {
namespace Environment {

/*! 
Get the value of an environment variable. This wraps around getenv() in
a portable, secure (and in future) thread-safe way. If the variable is 
undefined, return an empty string.

Returns false if the variable is undefined, the input string is unchanged.
*/
bool variable(const char* const _vrbl_name, std::string& _str);

//! Same as above, but returns an empty string if the variable is undefined.
inline std::string variable(const char* const _vrbl_name)
{
  std::string vrbl;
  variable(_vrbl_name, vrbl);
  return vrbl;
}

//! Same as above, but returns a default string if the variable is undefined.
inline std::string variable(const char* const _vrbl_name, 
  const char* const _vrbl_dflt)
{
  std::string vrbl(_vrbl_dflt);
  variable(_vrbl_name, vrbl);
  return vrbl;
}

/*!
Define a "scoped" custom locale session. 
*/
class LocaleSession
{
public:
  //default session locale is "C" (i.e., "." is the decimal point)
  LocaleSession(const char* const _ssn_lcle = "C");
  ~LocaleSession();

private:
  std::locale lcl_bckp_;
  std::string c_lcl_bckp_;
};


//! Return the system time in a string
std::string time();

}//namespace Environment 
}//namespace System

#endif//REFORM_ENVIRONMENT_HH_INCLUDED
