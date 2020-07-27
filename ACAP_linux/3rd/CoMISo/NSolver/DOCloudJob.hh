// (C) Copyright 2015 by Autodesk, Inc.

//=============================================================================
//
//  namespace COMISO::DOcloud
//
//=============================================================================

//== COMPILE-TIME PACKAGE REQUIREMENTS ========================================
#ifndef COMISO_DOCLOUDJOB_HH
#define COMISO_DOCLOUDJOB_HH

#include <CoMISo/Config/config.hh>
#if COMISO_DOCLOUD_AVAILABLE

#include "cURLpp.hh"

#include <Base/Utils/StopWatch.hh>
#include <string>
#include <vector>

namespace COMISO {
namespace DOcloud { 

class JsonTokens;

class Job : public cURLpp::Session
{
public:
  Job(const std::string& _filename, const std::string& _file_buf) 
    : filename_(_filename), file_buf_(_file_buf), stts_(nullptr) 
  {}
  ~Job();

  void setup()
  {
    make();
    upload();
    start();
  }

  void wait();
  void sync_status();
  void sync_log();
  bool active() const; // requires synchronized status
  bool stalled() const;

  void abort();
  double solution(std::vector<double>& _x) const;

protected:
  void make();
  void start();
  void upload(cURLpp::Upload& _upld);
  void upload();

private:
  const std::string filename_;
  const std::string file_buf_;
  std::string url_;
  JsonTokens* stts_;
  // these variables are initialized in start() 
  int log_seq_idx_; // the log sequence number, used to get DOcloud log entires
  int sol_nmbr_; // number of solutions found so far, according to the log
  int sol_sec_nmbr_; // number of seconds at the last new solution 
  int stld_sec_nmbr_; // number of seconds since the last new solution 
  Base::StopWatch stop_wtch_; // time since last successful log synchronization.
};

} // namespace DOcloud 
} // namespace COMISO 

#endif//COMISO_DOCLOUD_AVAILABLE
#endif//COMISO_DOCLOUDJOB_HH
