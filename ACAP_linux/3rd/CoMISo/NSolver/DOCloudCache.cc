// (C) Copyright 2015 by Autodesk, Inc.

//== INCLUDES =================================================================

//== COMPILE-TIME PACKAGE REQUIREMENTS ========================================
#include <CoMISo/Config/config.hh>
#if COMISO_DOCLOUD_AVAILABLE

//=============================================================================
#include "DOCloudCache.hh"
#include "DOCloudConfig.hh"

#include <Base/Debug/DebOut.hh>

#include <fstream>
#include <iomanip>
#include <cctype>
#include <functional>
#include <sstream>

#include <boost/filesystem.hpp>

// include windows.h without some of the excess
#define WIN32_LEAN_AND_MEAN
#include <windows.h> 

// ... and undefine ERROR 
#ifdef ERROR 
#undef ERROR
#endif//ERROR

//== NAMESPACES ===============================================================
namespace COMISO {

namespace DOcloud {

namespace {

// Create a new temporary exclusive file without extension that is used to 
// prevent write or read operation on files with the same name and extension
// .lp or .dat. while the cache is being written. This is the only class that
// uses windows specific APIs.
class FileLock
{
public:
  FileLock(const std::string& _filename)
  {
    file_hnd_ = CreateFile(_filename.c_str(),
      GENERIC_WRITE,
      0,            // ShareMode - 0 prevents any sharing
      nullptr,      // SecurityAttributes
      CREATE_NEW,   // Fails if file already exists.
      FILE_ATTRIBUTE_NORMAL | FILE_FLAG_DELETE_ON_CLOSE, // File attributes.
      NULL);
  }

  // We can write the DoCloud results only id the lock is successful.
  bool sucess() const { return file_hnd_ != INVALID_HANDLE_VALUE; }

  // If there is an active lock we can not read the related data because they 
  // are being written.
  static bool active(const std::string& _filename)
  {
    return GetFileAttributes(_filename.c_str()) != INVALID_FILE_ATTRIBUTES;
  }

  // The destructor removes the lock. from this moment the data can be freely
  // read and there should not be anyone who tries to rewrite them.
  ~FileLock()
  {
    if (sucess())
      CloseHandle(file_hnd_); // This will delete the file.
  }

private:
  HANDLE file_hnd_;
};


bool load_file(const std::string& _filename, std::string& _file_cnts)
{
  std::ifstream in_file_strm(_filename, std::ios::ate);
  if (!in_file_strm.is_open())
    return false;
  _file_cnts.reserve(in_file_strm.tellg());
  in_file_strm.seekg(0, std::ios::beg);
  _file_cnts.assign(std::istreambuf_iterator<char>(in_file_strm),
                    std::istreambuf_iterator<char>());
  return true;
}

bool save_file(const std::string& _filename, const std::string& _file_cnts)
{
  std::ofstream out_file_strm(_filename);
  if (!out_file_strm.is_open())
    return false;
  out_file_strm << _file_cnts;
  return true;
}

// Finds a key string from the file name. This string will be used as file name
// where to store the related cached data.
std::string string_to_hash(const std::string& _str)
{
  const std::hash<std::string> hash_fn;
  std::stringstream strm;
  strm << std::hex << hash_fn(_str);
  return strm.str();
}

const size_t NO_SOLUTION_CODE = UINT_MAX;

// Load variables and objective values from a file.
bool load_data(const std::string& _filename, 
               std::vector<double>& _x, double& _obj_val)
{
  std::ifstream in_file_strm(_filename);
  if (!in_file_strm.is_open())
    return false;

  size_t dim = std::numeric_limits<size_t>::max();
  in_file_strm >> dim;
  if (dim == NO_SOLUTION_CODE)
  {
    _x.clear();
    return true;
  }

  if (dim != _x.size())
    return false;

  for (auto& xi : _x)
    in_file_strm >> xi;
  in_file_strm >> _obj_val;
  return !in_file_strm.bad();
}

// Store variables and objective values in a file.
bool save_data(const std::string& _filename, 
               const std::vector<double>& _x, const double& _obj_val)
{
  std::ofstream out_file_strm(_filename);
  out_file_strm << std::setprecision(std::numeric_limits<double>::digits10 + 2);
  if (!out_file_strm.is_open())
    return false;

  if (_x.empty())
  {
    out_file_strm << NO_SOLUTION_CODE;
    return true;
  }
  out_file_strm << _x.size() << std::endl;
  for (const auto& xi : _x)
    out_file_strm << xi << std::endl;;
  out_file_strm << _obj_val;
  return !out_file_strm.bad();
}

} // namespace

Cache::Cache(const std::string& _mip_lp) 
  : mip_lp_(_mip_lp), hash_(string_to_hash(mip_lp_)), found_(false) 
{
  DEB_enter_func;
  DEB_line(2, "Cache hash: " << hash_);
}

bool Cache::restore_result(std::vector<double>& _x, double& _obj_val) 
{
  DEB_enter_func;
  const auto* cache_loc = Config::query().cache_location();
  if (cache_loc == nullptr) // cache location not provided, disabale the cache
    return false;

  for (size_t iter_nmbr = 0; iter_nmbr < 10; ++iter_nmbr)
  {
    filename_ = cache_loc + hash_ + '_' + std::to_string(iter_nmbr);

    std::string dat_filename(filename_ + ".dat");
    boost::system::error_code err_cod;
    if (!boost::filesystem::exists(
          boost::filesystem::path(dat_filename.c_str()), err_cod) ||
         err_cod.value() != boost::system::errc::success)
    {
      // If the .dat file does not exist it is not safe to check the lock because
      // it is possible that this process finds no lock, another process sets the
      // lock and start writing data, this process reads not fully written data.
      break;
    }

    if (FileLock::active(filename_))
      break;

    std::string cache_cnts;
    if (!load_file(filename_ + ".lp", cache_cnts))
      break;

    if (cache_cnts == mip_lp_)
    {
      found_ = load_data(filename_ + ".dat", _x, _obj_val);
      return found_;
    }
  }
  return false;
}

namespace {
// Save the couple of files fname.lp and fname.dat . They are an element of a 
// sort of map from file.lp to file.dat. So if there is an error saving the .dat
// file, the .lp file must also e deleted.
class CacheSaver
{
public:
  CacheSaver() : success_(false) {}

  ~CacheSaver()
  {
    if (success_)
      return;
    // Removes files eventually written if there has been any kind of failure.
    for (const auto& filename : used_files_)
    {
      if (!filename.empty())
        std::remove(filename.c_str());
    }
  }

  void save(const std::string& _filename, const std::vector<double>& _x,
    const double& _obj_val, const std::string& _lp_cnts)
  {
    DEB_enter_func;

    FileLock file_lock(_filename);
    if (file_lock.sucess())
    {
      used_files_[0] = _filename + ".lp";
      success_ = save_file(used_files_[0], _lp_cnts);
      if (success_)
      {
        used_files_[1] = _filename + ".dat";
        success_ = save_data(used_files_[1], _x, _obj_val);
      }
    }
  }

private:
  bool success_;
  std::string used_files_[2];
};

} // namespace

void Cache::store_result(const std::vector<double>& _x, const double& _obj_val)
{
  DEB_enter_func;
  if (filename_.empty() || found_)
  {// restore_result() either not called at all, or hit the cache
    DEB_error("store_result() called incorrectly");
    return;
  }
  CacheSaver saver;
  saver.save(filename_, _x, _obj_val, mip_lp_);
}

} // namespace DOcloud
} // namespace COMISO

#endif // COMISO_DOCLOUD_AVAILABLE
//=============================================================================

