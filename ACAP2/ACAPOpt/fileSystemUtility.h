#pragma once

#include <string>
#include <vector>

class FileSystemUtility
{

public:

	static void getAllFileRec(const std::string & path, const std::string & exd, std::vector<std::string> & files);
	//@param files 

	static void getAllFile(std::string path, std::string exd, std::vector<std::string> & files);

	//sort file name by 
	// 1.obj 10.obj 2.obj  -> 1.obj 2.obj 10.obj
	static void sortFileName(std::vector<std::string> & fileList);


	static std::string& yj_replace_all_distinct(std::string& str, const std::string& old_value, const std::string& new_value);


};
