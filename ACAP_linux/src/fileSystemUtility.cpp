#include "fileSystemUtility.h"

//#include <io.h>
#include <string>
#include <vector>
#include <iostream>
#include <algorithm>
#include <boost/filesystem.hpp>
#include "natsort.hpp"


bool endswith(const std::string & path, const std::string & exd)
{
  return path.length() >= exd.length() && path.substr(path.length()-exd.length()) == exd;
}

void FileSystemUtility::getAllFileRec(const std::string & path, const std::string & exd, std::vector<std::string> & files)
{
  using namespace boost::filesystem;
  directory_iterator end_itr;
  for(directory_iterator itr(path); itr != end_itr; ++itr)
    if(is_regular_file(itr->path()) && endswith(itr->path().string(),exd))
      files.push_back(itr->path().string());
    else if(is_directory(itr->path()))
      getAllFileRec(itr->path().string(),exd,files);

}

void FileSystemUtility::getAllFile(std::string path, std::string exd, std::vector<std::string> & files)
{
  using namespace boost::filesystem;
  directory_iterator end_itr;
  for(directory_iterator itr(path); itr != end_itr; ++itr)
    if(is_regular_file(itr->path()) && endswith(itr->path().string(),exd))
      files.push_back(itr->path().string());

}

std::string& FileSystemUtility::yj_replace_all_distinct(std::string& str, const std::string& old_value, const std::string& new_value)
{
  for (std::string::size_type pos(0); pos != std::string::npos; pos += new_value.length()) {
    if ((pos = str.find(old_value, pos)) != std::string::npos)
      str.replace(pos, old_value.length(), new_value);
    else break;
  }
  return str;
}

bool cmp(std::string const &arg_a, std::string const &arg_b)
{
  return arg_a.size() < arg_b.size() || (arg_a.size() == arg_b.size() && arg_a < arg_b);
}

void FileSystemUtility::sortFileName(std::vector<std::string> & fileList)
{
  std::sort(fileList.begin(), fileList.end());

  std::sort(fileList.begin(), fileList.end(), cmp);
  //for (int i = 0; i != fileList.size(); ++i)
  //	std::cout << fileList[i] << "\n";
}
