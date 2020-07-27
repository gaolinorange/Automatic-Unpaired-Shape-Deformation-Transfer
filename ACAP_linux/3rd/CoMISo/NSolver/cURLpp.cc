// (C) Copyright 2015 by Autodesk, Inc.

//=============================================================================
//
//  namespace COMISO::cURLpp IMPLEMENTATION
//
//=============================================================================

#include "cURLpp.hh"
#if COMISO_DOCLOUD_AVAILABLE
#include "CoMISo/Utils/CoMISoError.hh"
#include <Base/Debug/DebUtils.hh>

#include <curl/curl.h>

#include <io.h>

DEB_module("cURLpp")

namespace COMISO {
namespace cURLpp { 

//////////////////////////////////////////////////////////////////////////
// Session
Session::Session() { curl_global_init(CURL_GLOBAL_DEFAULT); }
Session::~Session() { curl_global_cleanup(); }

//////////////////////////////////////////////////////////////////////////
// Request 
Request::Request() : hnd_(curl_easy_init()), http_hdr_(nullptr) 
{
  COMISO_THROW_if(hnd_ == nullptr, DOCLOUD_REQUEST_INIT_FAILED);
}

Request::~Request() 
{ 
  curl_easy_cleanup(hnd_); 
  if (http_hdr_ != nullptr)
    curl_slist_free_all(http_hdr_);
}

void Request::set_url(const char* _url)
{
  curl_easy_setopt(hnd_, CURLOPT_URL, _url);
}

void Request::add_http_header(const char* _hdr)
{
  http_hdr_ = curl_slist_append(http_hdr_, _hdr);
}

// TODO: REMOVE this security hole!!
void Request::disable_ssl_verification()
{
  /*
    * If you want to connect to a site who isn't using a certificate that is
    * signed by one of the certs in the CA bundle you have, you can skip the
    * verification of the server's certificate. This makes the connection
    * A LOT LESS SECURE.
    *
    * If you have a CA cert for the server stored someplace else than in the
    * default bundle, then the CURLOPT_CAPATH option might come handy for
    * you.
    */ 
  curl_easy_setopt(hnd_, CURLOPT_SSL_VERIFYPEER, 0L);
 
  /*
    * If the site you're connecting to uses a different host name that what
    * they have mentioned in their server certificate's commonName (or
    * subjectAltName) fields, libcurl will refuse to connect. You can skip
    * this check, but this will make the connection less secure.
    */ 
  curl_easy_setopt(hnd_, CURLOPT_SSL_VERIFYHOST, 0L);
}

void Request::perform()
{
  DEB_enter_func;

  prepare();

  // set the header we send to the server
  curl_easy_setopt(hnd_, CURLOPT_HTTPHEADER, http_hdr_);
  // set the write function to handle incoming data from the server
  curl_easy_setopt(hnd_, CURLOPT_WRITEFUNCTION, write_func);
  // set the string to store the incoming header data
  curl_easy_setopt(hnd_, CURLOPT_HEADERDATA, reinterpret_cast<void*>(&hdr_));
  // set the string to store the incoming main body (data)
  curl_easy_setopt(hnd_, CURLOPT_WRITEDATA, reinterpret_cast<void*>(&bdy_));
  // do the transmission
  for (int try_nmbr = 0; ; ++try_nmbr) // test exit conditions below
  {
    auto res = curl_easy_perform(hnd_);
    // Various errors can occur while performing the request, but a single 
    // retry is usually sufficient to recover, we do up to 10 attempts.
    // CURLE_SSL_CONNECT_ERROR happens a lot with the DOcloud service
    // CURLE_COULDNT_CONNECT happens occasionally.
    if (res == CURLE_OK) 
      break; // success, exit here
    else if (try_nmbr < 10) // retry
      DEB_warning(1, "curl_easy_perform() try #" << try_nmbr << " failed "
        "with code: " << res << ", message: " << curl_easy_strerror(res))
    else 
      COMISO_THROW(DOCLOUD_REQUEST_EXEC_FAILED)
  }

  DEB_line(6, "Received Header: " << hdr_);
  DEB_line(6, "Received Body: " << bdy_);

  finalize();
}

size_t Request::write_func(const char* _ptr, const size_t _size, 
  const size_t _nmemb, void* _str)
{
  // TODO: not sure how much exception-safe (e.g. out of memory this is!)
  size_t n_add = _size * _nmemb;
  if (n_add == 0)
    return 0;

  auto& str = *reinterpret_cast<std::string*>(_str);
  str.append(_ptr, n_add);
  return n_add;
}

//////////////////////////////////////////////////////////////////////////
// Post

void Post::prepare()
{
  // set the post fields
  curl_easy_setopt(hnd_, CURLOPT_POSTFIELDS, post_.data());
  curl_easy_setopt(hnd_, CURLOPT_POSTFIELDSIZE, post_.size());
}

//////////////////////////////////////////////////////////////////////////
// Upload

void Upload::prepare()
{
  /* tell it to "upload" to the URL */ 
  curl_easy_setopt(hnd_, CURLOPT_UPLOAD, 1L);

  size_t data_len = send_data();
  /* and give the size of the upload (optional) */ 
  curl_easy_setopt(hnd_, CURLOPT_INFILESIZE_LARGE, (curl_off_t)data_len);

  /* we want to use our own read function */
  //curl_easy_setopt(hnd_, CURLOPT_READFUNCTION, read_func);

  /* enable verbose for easier tracing */ 
  //curl_easy_setopt(hnd_, CURLOPT_VERBOSE, 1L);
}

void Upload::finalize()
{
  DEB_enter_func;

  double rate, time;

  /* now extract transfer info */ 
  curl_easy_getinfo(hnd_, CURLINFO_SPEED_UPLOAD, &rate);
  curl_easy_getinfo(hnd_, CURLINFO_TOTAL_TIME, &time);

  DEB_double_format("%.2f");
  DEB_line(2, 
    "Upload speed: " << rate / 1024. << "Kbps; Time: " << time << "s.");
}

//////////////////////////////////////////////////////////////////////////
// UploadFile

UploadFile::~UploadFile()
{
  if (file_ != nullptr)
    std::fclose(file_);
}

size_t UploadFile::send_data()
{
  /* set where to read from (on Windows you need to use READFUNCTION too) */ 
  file_ = std::fopen(filename_.data(), "rb");
  curl_easy_setopt(hnd_, CURLOPT_READDATA, file_);

  return _filelength(fileno(file_));
}

//////////////////////////////////////////////////////////////////////////
// UploadData

size_t UploadData::send_data()
{
  curl_easy_setopt(hnd_, CURLOPT_READFUNCTION, Buffer::copy);
  curl_easy_setopt(hnd_, CURLOPT_READDATA, &buf_);
  return buf_.length();
}

size_t UploadData::Buffer::copy(void* _target, const size_t _elem_size, 
                                const size_t _n_elem, void* _from_buf)
{
  auto dat = reinterpret_cast<Buffer *>(_from_buf);
  size_t char_to_write = dat->len_ - dat->pos_;
  size_t buffer_len = _elem_size * _n_elem;
  if (char_to_write > buffer_len)
    char_to_write = buffer_len;
  std::copy(dat->ptr_ + dat->pos_, dat->ptr_ + dat->pos_ + char_to_write,
    (char*)_target);
  dat->pos_ += char_to_write;
  return char_to_write;
}

//////////////////////////////////////////////////////////////////////////
// Get
void Get::prepare() { curl_easy_setopt(hnd_, CURLOPT_HTTPGET, 1L); }

//////////////////////////////////////////////////////////////////////////
// Delete
void Delete::prepare() 
{ 
  curl_easy_setopt(hnd_, CURLOPT_CUSTOMREQUEST, "DELETE"); 
}

} // namespace cURLpp 
} // namespace COMISO 

#endif // COMISO_DOCLOUD_AVAILABLE
