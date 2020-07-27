// (C) Copyright 2017 by Autodesk, Inc.

#include "Base/Security/Mandatory.hh"
#include "Base/Code/Quality.hh"

#include "ProgressNode.hh"
#include "Base/Utils/BaseError.hh"
#include "Base/Utils/IOutputStream.hh"
#include "Base/Debug/DebOut.hh"

#include <exception>
#include <fstream>
#include <chrono>

#ifdef PROGRESS_ON
namespace Progress {

struct Context::LogData
{
  typedef Base::OutputStreamAdaptT<std::ofstream> FileOutputStream;

  FileOutputStream strm; 

  typedef std::chrono::high_resolution_clock Clock;
  typedef Clock::time_point Time;
  typedef std::chrono::seconds Seconds;
  typedef std::chrono::milliseconds Milliseconds;
  typedef std::chrono::nanoseconds Nanoseconds;

  LogData() : strm(LOG_FILENAME) {}

  void record_entry_time()  { entr_time = Clock::now(); }

  TickNumber elapsed_nanoseconds() const
  {
    const auto exit_time = Clock::now();
    const auto elpd_time = exit_time - entr_time;
    const auto elpd_mlsc = std::chrono::duration_cast<Nanoseconds>(elpd_time);
    return elpd_mlsc.count();
  }

private:
  Time entr_time;
};
/// Context implementation
Context::Context()
  : abrt_stte_(AST_NONE), abrt_alwd_(false), phny_root_("phony"), 
  root_node_(&phny_root_), actv_node_(&phny_root_), log_data_(nullptr)
{}

Context::~Context()
{
  delete log_data_;
}

void Context::abort() 
{ 
  // Don't throw if abort is not permitted or 
  // if there is stack unwinding currently in progress in this thread.

  // Note that stack unwinding can be active, and it could be still possible to
  // throw safely, see here:
  // https://akrzemi1.wordpress.com/2011/09/21/destructors-that-throw/
  // But we rather just wait until the current exception is handled. 

  if (!abrt_alwd_ || std::uncaught_exception())
    return;
  abrt_stte_ = AST_PROCESSING;
  BASE_THROW_ERROR(PROGRESS_ABORTED);
}

void Context::resume_abort()
{
  if (abrt_stte_ == AST_PROCESSING)
    BASE_THROW_ERROR(PROGRESS_ABORTED);
}

void Context::end_abort()
{
  abrt_stte_ = AST_NONE;
}

void Context::enter_node(Node* const _node)
{ // NOTE: this function is written carefully to allow asynchronous inspection
  // of the node graph from a tracking thread. It assumes the tracking thread
  // does not use the node parent.
  _node->tick_nmbr_ = 0;
  _node->done_chld_nmbr_ = 0;
  if (phony()) // no active node, ...
    root_node_ = _node; // ... so the node to enter is the root
  else // the currently active node becomes the parent of the node to enter
  {
    _node->prnt_ = actv_node_; // make the parent the active node
    int done_chld_nmbr = 0;
    // seek the "active" child in the parent's list of children
    auto chld = actv_node_->child();
    for (; chld != nullptr && chld != _node; chld = chld->next())
      ++done_chld_nmbr;
    DEB_error_if(chld != _node, "Could not find the child node " << _node->name 
      << " in the children list of the parent node " << actv_node_->name);
    actv_node_->done_chld_nmbr_ = done_chld_nmbr; // update the parent data
  }
  if (logging()) // record the entry time
  {
    if (actv_node_ != nullptr) // add the elapsed time to the active node
      actv_node_->time_ns_ += log_data_->elapsed_nanoseconds();
    _node->time_ns_ = 0; // restart counting in the active node
    log_data_->record_entry_time(); // record the entry time into _node
  }
  actv_node_ = _node;
}

void Context::exit_node()
{ // NOTE: this function is written carefully to allow asynchronous inspection
  // of the node graph from a tracking thread. It assumes the tracking thread
  // does not use the node parent.
  if (phony())
  {// this is a bug, but guard against it anyway since it's difficult to test
    DEB_error("Context exits from the phony() node!");
    return; 
  }

  if (logging())
  {// log the node on exit
    actv_node_->time_ns_ += log_data_->elapsed_nanoseconds();

    auto& os = log_data_->strm;
    os << actv_node_->name << LOG_COMMA << 
      actv_node_->tick_number() << LOG_COMMA <<
      actv_node_->time_ns_ / 1000000 << LOG_COMMA; // convert to milliseconds
    actv_node_->log(os);
    os << Base::ENDL;

    // record the re-entry time into the parent _node
    log_data_->record_entry_time();
  }

  auto* const prnt = actv_node_->prnt_;
  actv_node_->prnt_ = nullptr;
  if (prnt == nullptr)
  {
    actv_node_ = &phny_root_;
    return;
  }
  ++prnt->done_chld_nmbr_; // this node is now done as well
  actv_node_ = prnt; // activate the parent
}

void Context::set_logging(const bool _lgng)
{
  if (_lgng == logging())
    return;
  if (_lgng)
    log_data_ = new LogData; // turn on logging
  else
  {// turn off logging.
    delete log_data_;
    log_data_ = nullptr;
  }
}

BASE_THREAD_LOCAL Context cntx;

/// Node implementation
Node::~Node() 
{}

int Node::weight() const
{
  return 1;
}

TickNumber Node::tick_number_max() const
{
  return 1;
}

void Node::log(IOutputStream& /*_os*/) const
{}

} //namespace Progress
#endif//PROGRESS_ON
