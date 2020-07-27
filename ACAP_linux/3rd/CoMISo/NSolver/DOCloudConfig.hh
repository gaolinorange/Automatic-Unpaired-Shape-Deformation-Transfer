// (C) Copyright 2015 by Autodesk, Inc.

//=============================================================================
//
//  CLASS DOCloudSolver
//
//=============================================================================
#ifndef COMISO_DOCLOUDCONFIG_HH
#define COMISO_DOCLOUDCONFIG_HH

//== COMPILE-TIME PACKAGE REQUIREMENTS ========================================
#include <CoMISo/Config/config.hh>
#if COMISO_DOCLOUD_AVAILABLE

#include <string>

//== NAMESPACES ===============================================================
namespace COMISO {
namespace DOcloud {

//== CLASS DEFINITION =========================================================

/**
    Configuration options for the IBM Decision Optimization Cloud.
*/
class Config
{
public:
  // access the configuration singleton
  static const Config& query();
  static Config& modify();

  void set_root_url(const char* const _root_url);
  const char* root_url() const { return root_url_.data(); }

  void set_api_key(const char* _api_key);
  const char* api_key() const { return api_key_.data(); }

  void set_infeasible_timeout(const int _infs_time);
  int infeasible_timeout() const { return infs_time_; }

  void set_feasible_timeout(const int _fsbl_time); 
  int feasible_timeout() const { return fsbl_time_; }

  void set_cache_location(const char* const _cache_loc);
  const char* cache_location() const 
  { 
    return cache_loc_.empty() ? nullptr : cache_loc_.data(); 
  }

private:
  std::string root_url_;
  std::string api_key_;
  int infs_time_;
  int fsbl_time_;
  std::string cache_loc_;

private:
  Config();
  static Config& object();
};

//=============================================================================
} // namespace DOcloud
} // namespace COMISO

//=============================================================================
#endif // COMISO_DOCLOUD_AVAILABLE
//=============================================================================
#endif // COMISO_DOCLOUDCONFIG_HH
//=============================================================================

