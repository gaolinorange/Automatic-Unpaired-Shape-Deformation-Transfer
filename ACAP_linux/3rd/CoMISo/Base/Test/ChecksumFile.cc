// (C) Copyright 2016 by Autodesk, Inc.

#ifdef TEST_ON

#include "ChecksumFile.hh"
#include "Base/Utils/OStringStream.hh"

#include <boost/functional/hash.hpp>
#include <boost/filesystem.hpp>

#include <fstream>

namespace Test {
namespace Checksum {

void File::record(const char* _flnm)
{
  // Reads the file, makes a has number form its data and finally record 
  // hash_number and filename.
  std::ifstream fstr(_flnm);
  std::array<char, 1000> buf;
  size_t file_hash = 0;
  size_t file_size = 0;
  while(fstr)
  {
    fstr.read(buf.data(), buf.size());
    auto buf_hash = boost::hash_range(buf.data(), buf.data() + fstr.gcount());
    file_size += fstr.gcount();
    boost::hash_combine(file_hash, buf_hash);
  }
  Base::OStringStream strm;
  strm << "\"" << _flnm << '\"' << " (Size: " << file_size << ")" <<  
    "(Hash: " << Base::format_hex(file_hash) << ")";

  add(Result::OK, strm.str);
}

}//namespace Checksum
}//namespace Test

#endif//TEST_ON
