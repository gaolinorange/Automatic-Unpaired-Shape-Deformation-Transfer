// (C) Copyright 2015 by Autodesk, Inc.

//=============================================================================
//
//  namespace COMISO::cURLpp
//
//=============================================================================

//== COMPILE-TIME PACKAGE REQUIREMENTS ========================================
#ifndef COMISO_CURLPP_HH
#define COMISO_CURLPP_HH

#include <CoMISo/Config/config.hh>
#if COMISO_DOCLOUD_AVAILABLE

#include <string>
#include <stdio.h>

struct curl_slist; // cURL type forward declaration
typedef void CURL; // cURL type forward declaration

namespace COMISO {
namespace cURLpp { // some classes to wrap around the libcurl C data

struct Session
{
  Session();
  ~Session();
};

// TODO: This inheritance could be restrictive in the future for now is OK
class Request
{
public:
  Request();
  virtual ~Request();

  void set_url(const char* _url);

  void add_http_header(const char* _hdr);

  // TODO: REMOVE this security hole!!
  void disable_ssl_verification();

  void perform();

  const std::string& header() const { return hdr_; }
  const std::string& body() const { return bdy_; }

protected:
  static size_t write_func(const char* _ptr, const size_t _size, 
    const size_t _nmemb, void* _str);

protected:
  CURL* hnd_;

protected:
  //virtual functions to control the perform() behavior
  virtual void prepare() {}
  virtual void finalize() {}

private:
  curl_slist* http_hdr_;
  std::string hdr_;
  std::string bdy_;
};

class Post : public Request
{
public: 
  Post(const char* _post) : post_(_post) {}
  Post(const std::string& _post) : post_(_post) {}

protected:
  virtual void prepare();

private:
  std::string post_;
};

class Upload : public Request
{
protected:
  virtual void prepare();
  virtual void finalize();

  virtual size_t send_data() = 0;
};

class UploadFile : public Upload
{
public:
  UploadFile(const char* _filename) : filename_(_filename) {}
  UploadFile(const std::string& _filename) : filename_(_filename) {}

  virtual ~UploadFile();

protected:
  virtual size_t send_data();

private:
  std::string filename_;
  FILE* file_;
};

class UploadData : public Upload
{
public:
  explicit UploadData(const std::string& _buf) : buf_(_buf)  {}

protected:
  virtual size_t send_data();

private:
  class Buffer
  {
  public:
    explicit Buffer(const std::string& _data)
      : ptr_(_data.c_str()), len_(_data.size()), pos_(0) 
    {}

    static size_t copy(void* _target, const size_t _elem_size, 
      const size_t _n_elem, void* _from_buf);

    size_t length() const { return len_; }

  private:
    const char* ptr_;
    size_t len_;
    size_t pos_;
  };

  Buffer buf_;
};

class Get : public Request
{
protected:
  virtual void prepare();
};

class Delete : public Request
{
protected:
  virtual void prepare();
};

} // namespace cURLpp 
} // namespace COMISO 

#endif//COMISO_DOCLOUD_AVAILABLE
#endif//COMISO_CURLPP_HH
