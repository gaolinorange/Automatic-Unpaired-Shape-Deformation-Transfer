//=============================================================================
//
//  CLASS IPOPTSolverLean - IMPLEMENTATION
//
//=============================================================================

//== INCLUDES =================================================================

//== COMPILE-TIME PACKAGE REQUIREMENTS ========================================
#include <CoMISo/Config/config.hh>
#if COMISO_IPOPT_AVAILABLE
//=============================================================================


#include "IPOPTSolverLean.hh"
#include "NProblemGmmInterface.hh"
#include "NProblemInterface.hh"
#include "NProblemIPOPT.hh"
#include "NConstraintInterface.hh"
#include "BoundConstraint.hh"
#include "CoMISo/Utils/CoMISoError.hh"

#include <Base/Debug/DebConfig.hh>
#include <Base/Debug/DebTime.hh>

#include <gmm/gmm.h>

#include <IpTNLP.hpp>
#include <IpIpoptApplication.hpp>
#include <IpSolveStatistics.hpp>

//== NAMESPACES ===============================================================

namespace COMISO {

//== IMPLEMENTATION ========================================================== 


// smart pointer to IpoptApplication to set options etc.
class IPOPTSolverLean::Impl 
{// Create an instance of the IpoptApplication
public:
  Impl() 
    : app_(IpoptApplicationFactory()), max_iter_(200), alm_infsb_thrsh_(0.5),
    incr_lazy_cnstr_max_iter_nmbr_(5), enbl_all_lzy_cnstr_(true) 
  {}

public:
  Ipopt::SmartPtr<Ipopt::IpoptApplication> app_;

  int max_iter_;
  double alm_infsb_thrsh_; 
  int incr_lazy_cnstr_max_iter_nmbr_;
  bool enbl_all_lzy_cnstr_;
};

// Constructor
IPOPTSolverLean::IPOPTSolverLean()
  : impl_(new Impl)
{

  // Switch to HSL if available
#if COMISO_HSL_AVAILABLE
  impl_->app_->Options()->SetStringValue("linear_solver", "ma57");
#else
  impl_->app_->Options()->SetStringValue("linear_solver", "mumps");
#endif

#ifdef DEB_ON
  if (!Debug::Config::query().console())
#endif
  {// Block any output on cout and cerr from Ipopt.
    impl_->app_->Options()->SetStringValue("suppress_all_output", "yes");
  }

#ifdef WIN32
  // Restrict memory to be able to run larger problems on windows
  // with the default mumps solver
  // TODO: find out what this does and whether it makes sense to do it
  impl_->app_->Options()->SetIntegerValue("mumps_mem_percent", 5);
#endif

  // set default parameters
  impl_->app_->Options()->SetIntegerValue("max_iter", 100);
  //  app->Options()->SetStringValue("derivative_test", "second-order");
  //  app->Options()->SetIntegerValue("print_level", 0);
  //  app->Options()->SetStringValue("expect_infeasible_problem", "yes");
}

IPOPTSolverLean::~IPOPTSolverLean()
{ delete impl_; }

double IPOPTSolverLean::energy()
{
  return impl_->app_->Statistics()->FinalObjective();
}

//-----------------------------------------------------------------------------

void IPOPTSolverLean::set_max_iterations(const int _max_iterations)
{
	impl_->max_iter_ = _max_iterations;
}

int IPOPTSolverLean::max_iterations() const
{
	return impl_->max_iter_;
}

double IPOPTSolverLean::almost_infeasible_threshold() const
{
  return impl_->alm_infsb_thrsh_;
}

void IPOPTSolverLean::set_almost_infeasible_threshold(const double _alm_infsb_thrsh)
{
  impl_->alm_infsb_thrsh_ = _alm_infsb_thrsh;
}

int IPOPTSolverLean::incremental_lazy_constraint_max_iteration_number() const
{
  return impl_->incr_lazy_cnstr_max_iter_nmbr_;
}

void IPOPTSolverLean::set_incremental_lazy_constraint_max_iteration_number(
  const int _incr_lazy_cnstr_max_iter_nmbr)
{
  impl_->incr_lazy_cnstr_max_iter_nmbr_ = _incr_lazy_cnstr_max_iter_nmbr;
}

bool IPOPTSolverLean::enable_all_lazy_contraints() const
{
  return impl_->enbl_all_lzy_cnstr_;
}

void IPOPTSolverLean::set_enable_all_lazy_contraints(const bool 
  _enbl_all_lzy_cnstr)
{
  impl_->enbl_all_lzy_cnstr_ = _enbl_all_lzy_cnstr;
}

//-----------------------------------------------------------------------------

static void throw_ipopt_solve_failure(Ipopt::ApplicationReturnStatus const status)
{
  DEB_enter_func
  DEB_warning(1, " IPOPT solve failure code is " << status)
  // TODO: we could translate these return codes, but will not do it for now
  //  enum ApplicationReturnStatus
  //    {
  //      Solve_Succeeded=0,
  //      Solved_To_Acceptable_Level=1,
  //      Infeasible_Problem_Detected=2,
  //      Search_Direction_Becomes_Too_Small=3,
  //      Diverging_Iterates=4,
  //      User_Requested_Stop=5,
  //      Feasible_Point_Found=6,
  //
  //      Maximum_Iterations_Exceeded=-1,
  //      Restoration_Failed=-2,
  //      Error_In_Step_Computation=-3,
  //      Maximum_CpuTime_Exceeded=-4,
  //      Not_Enough_Degrees_Of_Freedom=-10,
  //      Invalid_Problem_Definition=-11,
  //      Invalid_Option=-12,
  //      Invalid_Number_Detected=-13,
  //
  //      Unrecoverable_Exception=-100,
  //      NonIpopt_Exception_Thrown=-101,
  //      Insufficient_Memory=-102,
  //      Internal_Error=-199
  //    };
  //------------------------------------------------------
  switch (status) 
  {
  case Ipopt::Maximum_Iterations_Exceeded:
    COMISO_THROW(IPOPT_MAXIMUM_ITERATIONS_EXCEEDED);
  case Ipopt::NonIpopt_Exception_Thrown:  
    // this could be due to a thrown PROGRESS_ABORTED exception, ...
    PROGRESS_RESUME_ABORT; // ... so check if we need to resume it
  default:
    COMISO_THROW(IPOPT_OPTIMIZATION_FAILED);
  }
}

static void check_ipopt_status(Ipopt::ApplicationReturnStatus const _stat)
{
  if (_stat != Ipopt::Solve_Succeeded && _stat != Ipopt::Solved_To_Acceptable_Level)
    throw_ipopt_solve_failure(_stat);
}

void IPOPTSolverLean::solve(NProblemInterface* _problem, 
  const std::vector<NConstraintInterface*>& _constraints)
{
  DEB_time_func_def;
  //----------------------------------------------------------------------------
  // 1. Create an instance of IPOPT NLP
  //----------------------------------------------------------------------------
  Ipopt::SmartPtr<Ipopt::TNLP> np = new NProblemIPOPT(_problem, _constraints);
  NProblemIPOPT* np2 = dynamic_cast<NProblemIPOPT*> (Ipopt::GetRawPtr(np));

  //----------------------------------------------------------------------------
  // 2. exploit special characteristics of problem
  //----------------------------------------------------------------------------

  DEB_out(2,"exploit detected special properties: ");
  if(np2->hessian_constant())
  {
    DEB_out(2,"*constant hessian* ");
    impl_->app_->Options()->SetStringValue("hessian_constant", "yes");
  }

  if(np2->jac_c_constant())
  {
    DEB_out(2, "*constant jacobian of equality constraints* ");
    impl_->app_->Options()->SetStringValue("jac_c_constant", "yes");
  }

  if(np2->jac_d_constant())
  {
    DEB_out(2, "*constant jacobian of in-equality constraints*");
    impl_->app_->Options()->SetStringValue("jac_d_constant", "yes");
  }
  DEB_out(2,"\n");

  //----------------------------------------------------------------------------
  // 3. solve problem
  //----------------------------------------------------------------------------

  // Initialize the IpoptApplication and process the options
  Ipopt::ApplicationReturnStatus status = impl_->app_->Initialize();
  if (status != Ipopt::Solve_Succeeded) 
    COMISO_THROW(IPOPT_INITIALIZATION_FAILED);

  status = impl_->app_->OptimizeTNLP( np);

  //----------------------------------------------------------------------------
  // 4. output statistics
  //----------------------------------------------------------------------------
  check_ipopt_status(status);
  
  // Retrieve some statistics about the solve
  Ipopt::Index iter_count = impl_->app_->Statistics()->IterationCount();
  DEB_out(1,"\n*** IPOPT: The problem solved in " 
    << iter_count << " iterations!\n");

  Ipopt::Number final_obj = impl_->app_->Statistics()->FinalObjective();
  DEB_out(1,"\n*** IPOPT: The final value of the objective function is "
    << final_obj << "\n");
}


//-----------------------------------------------------------------------------


void IPOPTSolverLean::solve(
      NProblemInterface*                        _problem,
      const std::vector<NConstraintInterface*>& _constraints,
      const std::vector<NConstraintInterface*>& _lazy_constraints)
{
  DEB_time_func_def;
  //----------------------------------------------------------------------------
  // 0. Initialize IPOPT Application
  //----------------------------------------------------------------------------

  // Initialize the IpoptApplication and process the options
  Ipopt::ApplicationReturnStatus status;
  status = impl_->app_->Initialize();
  if (status != Ipopt::Solve_Succeeded)
    COMISO_THROW(IPOPT_INITIALIZATION_FAILED);

  bool feasible_point_found = false;
  int  cur_pass = impl_->enbl_all_lzy_cnstr_ ? 1 : 0;
  const int max_passes = impl_->incr_lazy_cnstr_max_iter_nmbr_;

  double acceptable_tolerance = 0.01; // hack: read out from ipopt!!!
  // copy default constraints
  std::vector<NConstraintInterface*> constraints = _constraints;
  std::vector<bool> lazy_added(_lazy_constraints.size(),false);

  // cache statistics of all iterations
  std::vector<int> n_inf;
  std::vector<int> n_almost_inf;

  // set max iterations
  impl_->app_->Options()->SetIntegerValue("max_iter", impl_->max_iter_);

  while(!feasible_point_found && cur_pass < max_passes)
  {
    ++cur_pass;
    //----------------------------------------------------------------------------
    // 1. Create an instance of current IPOPT NLP
    //----------------------------------------------------------------------------
    Ipopt::SmartPtr<Ipopt::TNLP> np = new NProblemIPOPT(_problem, constraints);
    NProblemIPOPT* np2 = dynamic_cast<NProblemIPOPT*> (Ipopt::GetRawPtr(np));
    // enable caching of solution
    np2->store_solution() = true;

    //----------------------------------------------------------------------------
    // 2. exploit special characteristics of problem
    //----------------------------------------------------------------------------

    DEB_out(2, "detected special properties which will be exploit: ");
    if(np2->hessian_constant())
    {
      DEB_out(2, "*constant hessian* ");
      impl_->app_->Options()->SetStringValue("hessian_constant", "yes");
    }

    if(np2->jac_c_constant())
    {
      DEB_out(2, "*constant jacobian of equality constraints* ");
      impl_->app_->Options()->SetStringValue("jac_c_constant", "yes");
    }

    if(np2->jac_d_constant())
    {
      DEB_out(2, "*constant jacobian of in-equality constraints*");
      impl_->app_->Options()->SetStringValue("jac_d_constant", "yes");
    }
    DEB_out(2, "\n");

    //----------------------------------------------------------------------------
    // 3. solve problem
    //----------------------------------------------------------------------------
    {
      DEB_time_session_def("IPOPT App OptimizeTNLP(np)");
      status = impl_->app_->OptimizeTNLP(np);
    }

    check_ipopt_status(status);

    // check lazy constraints
    n_inf.push_back(0);
    n_almost_inf.push_back(0);
    feasible_point_found = true;
    for (unsigned int i = 0; i < _lazy_constraints.size(); ++i)
    {
      if (lazy_added[i])
        continue;
      NConstraintInterface* lc = _lazy_constraints[i];

      double v = lc->eval_constraint(&(np2->solution()[0]));

      bool inf = false;
      bool almost_inf = false;

      if (lc->constraint_type() == NConstraintInterface::NC_EQUAL)
      {
        v = std::abs(v);
        if (v > acceptable_tolerance)
          inf = true;
        else
          if (v > impl_->alm_infsb_thrsh_)
            almost_inf = true;
      }
      else
        if (lc->constraint_type() == NConstraintInterface::NC_GREATER_EQUAL)
        {
          if (v < -acceptable_tolerance)
            inf = true;
          else
            if (v < impl_->alm_infsb_thrsh_)
              almost_inf = true;
        }
        else
          if (lc->constraint_type() == NConstraintInterface::NC_LESS_EQUAL)
          {
            if (v > acceptable_tolerance)
              inf = true;
            else
              if (v > -impl_->alm_infsb_thrsh_)
                almost_inf = true;
          }

      // infeasible?
      if (inf)
      {
        constraints.push_back(lc);
        lazy_added[i] = true;
        feasible_point_found = false;
        ++n_inf.back();
      }

      // almost violated or violated? -> add to constraints
      if (almost_inf)
      {
        constraints.push_back(lc);
        lazy_added[i] = true;
        ++n_almost_inf.back();
      }
    }
  }

  // no termination after max number of passes?
  if(!feasible_point_found)
  {
    DEB_warning(2, "Could not find a feasible point after " << max_passes - 1 << 
      " incremental lazy constraint iterations");
    if (!impl_->enbl_all_lzy_cnstr_)
      throw_ipopt_solve_failure(Ipopt::Maximum_Iterations_Exceeded);

    DEB_line(2, "Solving with ALL lazy constraints...");
    ++cur_pass;
    for (unsigned int i = 0; i < _lazy_constraints.size(); ++i)
    {
      if (!lazy_added[i])
        constraints.push_back(_lazy_constraints[i]);
    }
    //----------------------------------------------------------------------------
    // 1. Create an instance of current IPOPT NLP
    //----------------------------------------------------------------------------
    Ipopt::SmartPtr<Ipopt::TNLP> np = new NProblemIPOPT(_problem, constraints);
    NProblemIPOPT* np2 = dynamic_cast<NProblemIPOPT*> (Ipopt::GetRawPtr(np));
    // enable caching of solution
    np2->store_solution() = true;

    //----------------------------------------------------------------------------
    // 2. exploit special characteristics of problem
    //----------------------------------------------------------------------------

    DEB_out(2, "exploit detected special properties: ");
    if(np2->hessian_constant())
    {
      DEB_out(2, "*constant hessian* ");
      impl_->app_->Options()->SetStringValue("hessian_constant", "yes");
    }

    if(np2->jac_c_constant())
    {
      DEB_out(2, "*constant jacobian of equality constraints* ");
      impl_->app_->Options()->SetStringValue("jac_c_constant", "yes");
    }

    if(np2->jac_d_constant())
    {
      DEB_out(2, "*constant jacobian of in-equality constraints*");
      impl_->app_->Options()->SetStringValue("jac_d_constant", "yes");
    }
    std::cerr << std::endl;

    //----------------------------------------------------------------------------
    // 3. solve problem
    //----------------------------------------------------------------------------
    status = impl_->app_->OptimizeTNLP( np);
  }

  //----------------------------------------------------------------------------
  // 4. output statistics
  //----------------------------------------------------------------------------
  check_ipopt_status(status);

  // Retrieve some statistics about the solve
  Ipopt::Index iter_count = impl_->app_->Statistics()->IterationCount();
  DEB_out(1, "\n*** IPOPT: The problem solved in " 
    << iter_count << " iterations!\n");

  Ipopt::Number final_obj = impl_->app_->Statistics()->FinalObjective();
  DEB_out(1, "\n*** IPOPT: The final value of the objective function is "
    << final_obj << "\n");

  DEB_out(2, "############# IPOPT with lazy constraints statistics ###############\n");
  DEB_out(2, "#passes     : " << cur_pass << "( of " << max_passes << ")\n");
  for(unsigned int i=0; i<n_inf.size(); ++i)
    DEB_out(3, "pass " << i << " induced " << n_inf[i] 
      << " infeasible and " << n_almost_inf[i] << " almost infeasible\n")
}


//-----------------------------------------------------------------------------


void IPOPTSolverLean::solve(NProblemInterface*    _problem)
{
  std::vector<NConstraintInterface*> constraints;
  solve(_problem, constraints);
}


//-----------------------------------------------------------------------------


void IPOPTSolverLean::solve(NProblemGmmInterface* _problem, std::vector<NConstraintInterface*>& _constraints)
{
  DEB_enter_func;
  DEB_warning(1,"******NProblemGmmInterface is deprecated!!! -> use NProblemInterface *******");

  //----------------------------------------------------------------------------
  // 1. Create an instance of IPOPT NLP
  //----------------------------------------------------------------------------
  Ipopt::SmartPtr<Ipopt::TNLP> np = new NProblemGmmIPOPT(_problem, _constraints);

  //----------------------------------------------------------------------------
  // 2. solve problem
  //----------------------------------------------------------------------------

  // Initialize the IpoptApplication and process the options
  Ipopt::ApplicationReturnStatus status = impl_->app_->Initialize();
  if (status != Ipopt::Solve_Succeeded)
     COMISO_THROW(IPOPT_INITIALIZATION_FAILED);

  //----------------------------------------------------------------------------
  // 3. solve problem
  //----------------------------------------------------------------------------
  status = impl_->app_->OptimizeTNLP(np);

  //----------------------------------------------------------------------------
  // 4. output statistics
  //----------------------------------------------------------------------------
  check_ipopt_status(status);

  // Retrieve some statistics about the solve
  Ipopt::Index iter_count = impl_->app_->Statistics()->IterationCount();
  DEB_out(1,"\n*** IPOPT: The problem solved in " << iter_count << " iterations!\n");

  Ipopt::Number final_obj = impl_->app_->Statistics()->FinalObjective();
  DEB_out(1, "\n*** IPOPT: The final value of the objective function is "
    << final_obj << "\n");
}


//=============================================================================
} // namespace COMISO
//=============================================================================
#endif // COMISO_IPOPT_AVAILABLE
//=============================================================================
