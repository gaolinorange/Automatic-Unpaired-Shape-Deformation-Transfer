// (C) Copyright 2017 by Autodesk, Inc.

#ifndef BASE_PROGRESS_NODE_HH_INCLUDED
#define BASE_PROGRESS_NODE_HH_INCLUDED

#ifdef PROGRESS_ON

#include <Base/Utils/Thread.hh>
#include <stddef.h>

namespace Base {
class IOutputStream;
}

namespace Progress {

//! Progress stream type
typedef Base::IOutputStream IOutputStream;


//! Progress tick counter type
typedef size_t TickNumber;

/*!
The name of the progress log file, uses the .csv format.

Each line contains comma-separated values for a progress node as follows:
node_name, tick#, ms#, param0, param1, ...
*/
const char* const LOG_FILENAME = "progress.csv";

//! The log value separator
const char* const LOG_COMMA = ", ";

//! Represent an operational node in the progress graph 
class Node
{
public:
  typedef Node* (*MakeNodeFunction)(Node* _next);

  static inline Node* make_child_list() { return nullptr; }

  template <typename... ArgT>
  static inline Node* make_child_list(MakeNodeFunction _make_node_fnct, 
    const ArgT&... _args)
  {
    return (*_make_node_fnct)(make_child_list(_args...));
  }

public:
  const char* const name;
  
  Node(const char* const _name, 
    Node* _next = nullptr, Node* _chld = nullptr) 
    : name(_name), next_(_next), chld_(_chld), prnt_(nullptr), 
    tick_nmbr_(0), done_chld_nmbr_(0), time_ns_(0)
  {}

  virtual ~Node();

  const Node* next() const { return next_; };
  const Node* child() const { return chld_; };

  void tick() { ++tick_nmbr_; }

  //! Number of ticks recorded
  TickNumber tick_number() const { return tick_nmbr_; }
  
  //! Number of children that have been completed 
  int done_child_number() const { return done_chld_nmbr_; }

  /*
  Relative weight of this node in the overall computation process, default is 1. 
  */
  virtual int weight() const; 

  /*! 
  Estimate the maximum number of ticks in this node, default is 1.
  The function is virtual so that the node can make use of some custom logic 
  and specific parameters. 
  */
  virtual TickNumber tick_number_max() const;

  /*! 
  Log custom logic parameter values used by the estimation in tick_number_max().
  The values have to be separated by \ref LOG_COMMA.
  */
  virtual void log(IOutputStream& _os) const;

protected:
  Node* next_; //!< next node on the same level, last if nullptr
  Node* chld_; //!< first child node, leaf if nullptr
  Node* prnt_; //!< parent node, set temporarily
  TickNumber tick_nmbr_; /*!< number of ticks in the node, exclusive of the 
    ticks in child nodes */
  int done_chld_nmbr_; //!< Number of child nodes that made their exit

private:
  TickNumber time_ns_; /*!< Time in nanoseconds spent in the node since it was 
    entered, exclusive of the time spent in child nodes. This is recorded only 
    if the Context is logging, and is accessible only by the Context. */

  friend class Context;
};

class Context
{
public:
  //! Default constructor
  Context();
  ~Context();

  /*!
  Get if the progress is "faking" tracking, i.e., using a phony root node.
  Phony tracking is faster than checking if a root has been set. 
  */
  bool phony() const { return actv_node_ == &phny_root_; }

  /*! 
  Get the currently active node, only makes sense to use that if not phony().
  */
  const Node* active_node() const { return actv_node_; }

  /*! 
  Get the root node, only makes sense to use that if not phony().
  */
  const Node* root_node() const { return root_node_; }
  
  /*!
  Get if the context is active, i.e., there is an active operation in progress.
  */
  bool active() const { return actv_node_ != root_node_; }

  /*! 
  Request progress abort, intended to be called asynchronously from a "tracking" 
  thread.
  The abort will be triggered on the next \ref tick() in the context thread. 
  */
  void request_abort() { abrt_stte_ = AST_REQUEST; }

  /*! 
  Resume an interrupted abort if the exception could be eaten unintentionally, 
  e.g., by a 3P library. This throws only if the progress is being aborted and 
  does nothing otherwise. 
  */
  void resume_abort(); 

  /*!
  Call this to finish the abort process and indicate that the thrown exception 
  has been handled. 
  */
  void end_abort();

  void tick(const char* const /*_fnct*/)
  {
    actv_node_->tick();
    if (abrt_stte_ == AST_REQUEST)
      abort();
  }

  void enter_node(Node* const _node);
  void exit_node();

  void set_abort_allowed(bool _abrt_alwd) { abrt_alwd_ = _abrt_alwd; }
  bool abort_allowed() const { return abrt_alwd_; }

  //! Turn on/off progress logging
  void set_logging(const bool _lgng);

  //! Check if progress logging is on/off 
  bool logging() const { return log_data_ != nullptr; }

private:
  enum AbortStateType
  {
    AST_NONE,
    AST_REQUEST,
    AST_PROCESSING
  };

  struct LogData;

private:
  AbortStateType abrt_stte_; //!< Abort state
  bool abrt_alwd_; //!< Abort allowed flag, ignores AST_REQUEST if false
  Node phny_root_; //!< "phony" root
  Node* root_node_; //!< Root node
  Node* actv_node_; //!< Active node
  LogData* log_data_; //!< Log data, exists only when logging

private:
  //! Throws PROGRESS_ABORTED to abort the current operation.
  void abort();
};

//! the current thread Progress context
extern BASE_THREAD_LOCAL Context cntx;

/*! 
Enable (or disable) the Context::abort() processing, use to enable abort() 
processing which is disabled by default.

Use this to set up a scope in which the progress can be aborted. Such scope
should be exception-handled, unless exiting the application on abort is desired.
*/
class AbortAllowedSession
{
public:
  AbortAllowedSession(const bool _abrt_alwd)
    : abrt_alwd_bckp_(cntx.abort_allowed())
  {
    cntx.set_abort_allowed(_abrt_alwd);
  }

  ~AbortAllowedSession()
  {
    cntx.set_abort_allowed(abrt_alwd_bckp_);
  }

private:
  bool abrt_alwd_bckp_;
};

} //namespace Progress

#define PROGRESS_NODE_NAME(OPRT) OPRT##_node
#define PROGRESS_MAKE_NODE_NAME(OPRT) make_##OPRT##_node

#define PROGRESS_DECLARE_NODE(OPRT) \
  namespace Progress { \
  extern BASE_THREAD_LOCAL Node* PROGRESS_NODE_NAME(OPRT); \
  Node* PROGRESS_MAKE_NODE_NAME(OPRT)(Node* _next = nullptr); \
  } //namespace Progress {


#define PROGRESS_DEFINE_NODE_CUSTOM(TYPE, OPRT, NAME, ...) \
  BASE_THREAD_LOCAL Progress::Node* Progress::PROGRESS_NODE_NAME(OPRT) = nullptr; \
  Progress::Node* Progress::PROGRESS_MAKE_NODE_NAME(OPRT) \
    (Progress::Node* _next) \
  { \
    static BASE_THREAD_LOCAL TYPE node(NAME, _next, \
      Node::make_child_list( __VA_ARGS__ ));\
    return OPRT##_node = &node; \
  }

#define PROGRESS_DEFINE_NODE(OPRT, NAME, ...) \
  PROGRESS_DEFINE_NODE_CUSTOM(Node, OPRT, NAME, ##__VA_ARGS__)

#if defined(_MSC_VER)  
#define __PROGRESS_FUNCTION__ __FUNCTION__ // works in VC well 
#else 
#define __PROGRESS_FUNCTION__ __PRETTY_FUNCTION__ // needed for gcc & xcode
#endif// _MSC_VER

#define PROGRESS_TICK { Progress::cntx.tick(__PROGRESS_FUNCTION__); } 
#define PROGRESS_RESUME_ABORT { Progress::cntx.resume_abort(); }
#define PROGRESS_END_ABORT { Progress::cntx.end_abort(); }
#define PROGRESS_ALLOW_ABORT Progress::AbortAllowedSession abrt_alwd_sssn(true)

#else

#define PROGRESS_DECLARE_NODE(OPRT) 
#define PROGRESS_DEFINE_NODE_CUSTOM(TYPE, OPRT, ...)
#define PROGRESS_DEFINE_NODE(OPRT, ...) 
#define PROGRESS_TICK
#define PROGRESS_RESUME_ABORT
#define PROGRESS_END_ABORT
#define PROGRESS_ALLOW_ABORT

#endif// PROGRESS_ON

#endif//BASE_PROGRESS_NODE_HH_INCLUDED
