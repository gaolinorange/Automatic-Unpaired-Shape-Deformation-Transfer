// (C) Copyright 2015 by Autodesk, Inc.

#include "Base/Security/Mandatory.hh"
#include "Environment.hh"
#include "Base/Debug/DebOut.hh"

#include <cstdlib>
#include <cstring>
#include <locale.h>
#include <locale>
#include <time.h>

namespace System {
namespace Environment {

bool variable(const char* const _vrbl_name, std::string& _vrbl)
{
#ifdef WIN32 // The Windows implementation uses the secure getenv_s()
  size_t char_nmbr;
  getenv_s(&char_nmbr, NULL, 0, _vrbl_name);
  if (char_nmbr == 0)
    return false;
  _vrbl.resize(char_nmbr);
  getenv_s(&char_nmbr, &_vrbl[0], char_nmbr, _vrbl_name);
  _vrbl.resize(char_nmbr - 1); // remove the trailing \0 char
#else
  const char* const vrbl_env = std::getenv(_vrbl_name);
  if (vrbl_env == NULL)
    return false;
  _vrbl = vrbl_env;
#endif
  return true;
}

//default session locale is "C" (i.e., "." is the decimal point)
LocaleSession::LocaleSession(const char* const _ssn_lcle)
{
  c_lcl_bckp_ = ::setlocale(LC_ALL, NULL);
  // std::locale::global set the C++ and C locale at the same time
  // only if the locale has a name (as in the following call).
  // See http://www.cplusplus.com/reference/locale/locale/global/
  lcl_bckp_ = std::locale::global(std::locale(_ssn_lcle));
  DEB_only(char* lcle = )::setlocale(LC_ALL, NULL);
  DEB_error_if(lcle == NULL,
    "std::locale::global() failed to set " << _ssn_lcle);
  DEB_error_if(lcle != NULL && strcmp(lcle,  _ssn_lcle) != 0,
    "set_locale() was asked to set " << _ssn_lcle << 
    ", but actually set " << lcle);
}

LocaleSession::~LocaleSession()
{
  std::locale::global(lcl_bckp_);
  // We can not be sure that lcl_bckp_ had a name, so we must reset the
  // C locale explicitly.
  ::setlocale(LC_ALL, c_lcl_bckp_.c_str());
}

std::string time()
{
  time_t rawtime;
  ::time(&rawtime);

#ifdef WIN32
  struct tm timeinfo;
  int err = ::localtime_s(&timeinfo, &rawtime);
  if (err == 0)
  {
    char buffer[256];
    err = asctime_s(buffer, sizeof(buffer), &timeinfo);
    if (err == 0)
      return std::string(buffer);
  }
  return std::string("time() failed");
#else//WIN32
  char str[100];
  // Format: <Abbreviated weekday name> <abbreviated month name> <day of month> <HH>:<MM>:<SS> <Year>
  std::strftime(str, sizeof(str), "%a %b %d %H:%M:%S %Y", std::localtime(&rawtime));
  return std::string(str);
#endif//WIN32
}

}//namespace Environment 
}//namespace System
