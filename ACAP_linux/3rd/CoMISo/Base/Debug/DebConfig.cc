// (C) Copyright 2016 by Autodesk, Inc.

#ifdef DEB_ON

#include "DebConfig.hh"
#include "DebDefault.hh"
#include "DebFile.hh"
#include "Base/Utils/Environment.hh"

#include <fstream>
#include <sstream>
#include <list>
#include <string>
#include <map>

namespace Debug {
namespace {
// We use this data to decide the debug level of a function in a file.
class FilterLevelSelector
{
public:
  void add_file_string(const std::string& _str) 
  { 
    file_selct_strngs_.push_back(_str); 
  }

  void add_func_string(const std::string& _str) 
  { 
    func_selct_strngs_.push_back(_str); 
  }

  bool select_file(const char* _flnm) const
  {
    // TODO: this should be possible to implement w/o a copy
    std::string flnm(_flnm);
    // TODO: this code below only works in ReForm, should be made to work
    // for IGM, CoMISo, etc
    const std::string root_dir("ReForm");
    size_t pos = flnm.rfind(root_dir);
    if (pos != std::string::npos)
      flnm = flnm.substr(pos + root_dir.size());

    return search(flnm, file_selct_strngs_);
  }

  bool select_function(const char* _func) const
  {
    return search(_func, func_selct_strngs_);
  }

private:
  static bool search(const std::string& _flnm, 
    const std::list<std::string>& _sel_strings)
  {
    for (std::list<std::string>::const_iterator sel_it = _sel_strings.begin(); sel_it != _sel_strings.end(); ++sel_it)
    {
      const std::string& sel = *sel_it;
      if (_flnm.find(sel) != std::string::npos)
        return true;
    }
    return false;
  }

private:
  // list of strings to be found inside the file name.
  std::list<std::string> file_selct_strngs_; 
  // list of strings to be found inside the function name.
  std::list<std::string> func_selct_strngs_; 
};

}//namespace 

class Config::Impl
{
public:
  Impl() : dflt_lvl_(Default::LEVEL) { read(); }

  void read();
  int level(const char* const _flnm, const char* const _fnct) const;

private:
  int dflt_lvl_;
  typedef std::map<int, FilterLevelSelector> LevelFilterMap;
  LevelFilterMap lvl_fltrs_; // filters for each level
};

void Config::Impl::read()
{
  const std::string flnm =
    System::Environment::variable("REFORM_DEB_CONFIG", "reform_deb.cfg");

  std::ifstream cnfg_strm(flnm.c_str());
  std::string line;
  while (std::getline(cnfg_strm, line))
  {
    std::stringstream line_stream(line);
    std::string type;
    line_stream >> type;
    
    void (FilterLevelSelector::*add_string)(const std::string&) = NULL;
    
    if (type == "all") 
      {}
    else if (type == "file")
      add_string = &FilterLevelSelector::add_file_string;
    else if (type == "func")
      add_string = &FilterLevelSelector::add_func_string;
    else
      continue;

    int lvl;
    line_stream >> lvl;
    if (add_string == NULL)
    {
      dflt_lvl_ = lvl; // We have read the default level.
      continue;
    }
    char colon;
    line_stream >> colon;
    if (colon != ':')
      continue;
    std::string select_str;
    while(line_stream >> select_str)
      (lvl_fltrs_[lvl].*add_string)(select_str);
  }
}

int Config::Impl::level(const char* const _flnm, const char* const _fnct) const
{
  int lvl = dflt_lvl_;
  for (LevelFilterMap::const_iterator fltr_it = lvl_fltrs_.begin(); 
    fltr_it != lvl_fltrs_.end(); ++fltr_it)
  {// continue this iteration until the maximum allowed level if found
    const LevelFilterMap::value_type& fltr = *fltr_it;
    if (lvl >= fltr.first) // can this filter increase the current level?
      continue;      
    if (fltr.second.select_file(_flnm) || fltr.second.select_function(_fnct))
      lvl = fltr.first;
  }
  return lvl;
}

//////////////////////////////////////////////////////////////////////////
Config& Config::modify()
{
  static Config glbl_cnfg;
  return glbl_cnfg;
}

const Config& Config::query()
{
  return modify();
}

Config::Config() : impl_(new Impl) {}

Config::~Config() { delete impl_; }

int Config::level(const char* const _flnm, const char* const _fnct) const
{
  return impl_->level(_flnm, _fnct);
}

void Config::set_console(const bool _on) { File::modify().set_console(_on); }
bool Config::console() const { return File::query().console(); }

void Config::set_logfile(bool _on) { File::modify().set_logfile(_on); }
bool Config::logfile() const { return File::query().logfile(); }

}//namespace Debug

#endif//DEB_ON
