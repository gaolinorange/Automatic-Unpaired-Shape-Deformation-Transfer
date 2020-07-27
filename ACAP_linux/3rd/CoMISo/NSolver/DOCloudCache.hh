// (C) Copyright 2015 by Autodesk, Inc.

#ifndef COMISO_DOCloudCache_HH
#define COMISO_DOCloudCache_HH

//== COMPILE-TIME PACKAGE REQUIREMENTS ========================================
#include <CoMISo/Config/config.hh>
#if COMISO_DOCLOUD_AVAILABLE

//== INCLUDES =================================================================

#include <string>
#include <vector>

//== NAMESPACES ===============================================================
namespace COMISO {
namespace DOcloud {

// given a .lp file, checks if we have saved the result for the same problem.
// If so they are returned, so the calling function can avoid to compute them.
//

class Cache
{
public:
  // Manage the cache for an optimization problem stored in .lp format
  Cache(const std::string& _mip_lp);

  const std::string& hash() const { return hash_; }

  bool restore_result(
    std::vector<double>& _x,  // result.
    double& _obj_val // objective function value.
    ); 

  // We can store the result for the given .lp file. This makes sense only we have
  // not found cache data for the given .lp file, and in order to avoid data
  // corruption this function fails if the data have been found.
  void store_result(const std::vector<double>& _x, const double& _obj_val);


private:
  const std::string& mip_lp_; // The MIP represented in the .lp format 
  const std::string hash_; // hask for the lp_ problem
  bool found_; // Remembers if we have found a cache for the input problem
  std::string filename_; // File name to access the cached data (no extension)
};

} // namespace DOcloud
} // namespace COMISO
//=============================================================================

#endif // COMISO_DOCLOUD_AVAILABLE
//=============================================================================
#endif // COMISO_DOCloudCache_HH
//=============================================================================
