//=============================================================================
//
//  CLASS IPOPTSolver - IMPLEMENTATION
//
//=============================================================================

//== INCLUDES =================================================================

//== COMPILE-TIME PACKAGE REQUIREMENTS ========================================
#include <CoMISo/Config/config.hh>
#if COMISO_IPOPT_AVAILABLE
//=============================================================================


#include "IPOPTSolver.hh"

//== NAMESPACES ===============================================================

namespace COMISO {

//== IMPLEMENTATION ========================================================== 


// Constructor
IPOPTSolver::
IPOPTSolver()
{
  // Create an instance of the IpoptApplication
  app_ = IpoptApplicationFactory();

  // Switch to HSL if available in Comiso
  #if COMISO_HSL_AVAILABLE
    app_->Options()->SetStringValue("linear_solver", "ma57");
  #endif

  // Restrict memory to be able to run larger problems on windows
  // with the default mumps solver
  #ifdef WIN32
    app_->Options()->SetIntegerValue("mumps_mem_percent", 5);
  #endif

  // set default parameters
  app_->Options()->SetIntegerValue("max_iter", 100);
  //  app->Options()->SetStringValue("derivative_test", "second-order");
  //  app->Options()->SetIntegerValue("print_level", 0);
  //  app->Options()->SetStringValue("expect_infeasible_problem", "yes");

  print_level_ = 5;
}


//-----------------------------------------------------------------------------



int
IPOPTSolver::
solve(NProblemInterface* _problem, const std::vector<NConstraintInterface*>& _constraints)
{
  //----------------------------------------------------------------------------
  // 0. Check whether hessian_approximation is active
  //----------------------------------------------------------------------------
  bool hessian_approximation = false;
  std::string ha, p;
  app().Options()->GetStringValue("hessian_approximation", ha, p);
  if(ha != "exact")
  {
    if(print_level_>=2)
      std::cerr << "Hessian approximation is enabled" << std::endl;
    hessian_approximation = true;
  }

  //----------------------------------------------------------------------------
  // 1. Create an instance of IPOPT NLP
  //----------------------------------------------------------------------------
  Ipopt::SmartPtr<Ipopt::TNLP> np = new NProblemIPOPT(_problem, _constraints, hessian_approximation);
  NProblemIPOPT* np2 = dynamic_cast<NProblemIPOPT*> (Ipopt::GetRawPtr(np));

  //----------------------------------------------------------------------------
  // 2. exploit special characteristics of problem
  //----------------------------------------------------------------------------

  if(print_level_>=2)
    std::cerr << "exploit detected special properties: ";
  if(np2->hessian_constant())
  {
    if(print_level_>=2)
      std::cerr << "*constant hessian* ";
    app().Options()->SetStringValue("hessian_constant", "yes");
  }

  if(np2->jac_c_constant())
  {
    if(print_level_>=2)
      std::cerr << "*constant jacobian of equality constraints* ";
    app().Options()->SetStringValue("jac_c_constant", "yes");
  }

  if(np2->jac_d_constant())
  {
    if(print_level_>=2)
      std::cerr << "*constant jacobian of in-equality constraints*";
    app().Options()->SetStringValue("jac_d_constant", "yes");
  }

  if(print_level_>=2)
    std::cerr << std::endl;

  //----------------------------------------------------------------------------
  // 3. solve problem
  //----------------------------------------------------------------------------

  // Initialize the IpoptApplication and process the options
  Ipopt::ApplicationReturnStatus status;
  status = app_->Initialize();
  if (status != Ipopt::Solve_Succeeded)
  {
    if(print_level_>=2)
      printf("\n\n*** Error IPOPT during initialization!\n");
  }

  status = app_->OptimizeTNLP( np);

  //----------------------------------------------------------------------------
  // 4. output statistics
  //----------------------------------------------------------------------------
  if(print_level_>=2)
    if (status == Ipopt::Solve_Succeeded || status == Ipopt::Solved_To_Acceptable_Level)
    {
      // Retrieve some statistics about the solve
      Ipopt::Index iter_count = app_->Statistics()->IterationCount();
      printf("\n\n*** IPOPT: The problem solved in %d iterations!\n", iter_count);

      Ipopt::Number final_obj = app_->Statistics()->FinalObjective();
      printf("\n\n*** IPOPT: The final value of the objective function is %e.\n", final_obj);
    }

  return status;
}


//-----------------------------------------------------------------------------



int
IPOPTSolver::
solve(NProblemInterface*                        _problem,
      const std::vector<NConstraintInterface*>& _constraints,
      const std::vector<NConstraintInterface*>& _lazy_constraints,
      const double                              _almost_infeasible,
      const int                                 _max_passes        )
{
  //----------------------------------------------------------------------------
  // 0. Check whether hessian_approximation is active
  //----------------------------------------------------------------------------
  bool hessian_approximation = false;
  std::string ha, p;
  app().Options()->GetStringValue("hessian_approximation", ha, p);
  if(ha != "exact")
  {
    if(print_level_>=2)
      std::cerr << "Hessian approximation is enabled" << std::endl;
    hessian_approximation = true;
  }

  //----------------------------------------------------------------------------
  // 0. Initialize IPOPT Applicaiton
  //----------------------------------------------------------------------------

  StopWatch sw; sw.start();

  // Initialize the IpoptApplication and process the options
  Ipopt::ApplicationReturnStatus status;
  status = app_->Initialize();
  if (status != Ipopt::Solve_Succeeded)
  {
    printf("\n\n*** Error IPOPT during initialization!\n");
  }

  bool feasible_point_found = false;
  int  cur_pass = 0;
  double acceptable_tolerance = 0.01; // hack: read out from ipopt!!!
  // copy default constraints
  std::vector<NConstraintInterface*> constraints = _constraints;
  std::vector<bool> lazy_added(_lazy_constraints.size(),false);

  // cache statistics of all iterations
  std::vector<int> n_inf;
  std::vector<int> n_almost_inf;

  while(!feasible_point_found && cur_pass <(_max_passes-1))
  {

    ++cur_pass;
    //----------------------------------------------------------------------------
    // 1. Create an instance of current IPOPT NLP
    //----------------------------------------------------------------------------
    Ipopt::SmartPtr<Ipopt::TNLP> np = new NProblemIPOPT(_problem, constraints, hessian_approximation);
    NProblemIPOPT* np2 = dynamic_cast<NProblemIPOPT*> (Ipopt::GetRawPtr(np));
    // enable caching of solution
    np2->store_solution() = true;

    //----------------------------------------------------------------------------
    // 2. exploit special characteristics of problem
    //----------------------------------------------------------------------------

    if(print_level_>=2) std::cerr << "detected special properties which will be exploit: ";
    if(np2->hessian_constant())
    {
      if(print_level_>=2) std::cerr << "*constant hessian* ";
      app().Options()->SetStringValue("hessian_constant", "yes");
    }

    if(np2->jac_c_constant())
    {
      if(print_level_>=2) std::cerr << "*constant jacobian of equality constraints* ";
      app().Options()->SetStringValue("jac_c_constant", "yes");
    }

    if(np2->jac_d_constant())
    {
      if(print_level_>=2) std::cerr << "*constant jacobian of in-equality constraints*";
      app().Options()->SetStringValue("jac_d_constant", "yes");
    }
    if(print_level_>=2) std::cerr << std::endl;

    //----------------------------------------------------------------------------
    // 3. solve problem
    //----------------------------------------------------------------------------
    status = app_->OptimizeTNLP( np);

    // check lazy constraints
    n_inf.push_back(0);
    n_almost_inf.push_back(0);
    feasible_point_found = true;
    for(unsigned int i=0; i<_lazy_constraints.size(); ++i)
      if(!lazy_added[i])
      {
        NConstraintInterface* lc = _lazy_constraints[i];
        double v = lc->eval_constraint(&(np2->solution()[0]));
        bool inf        = false;
        bool almost_inf = false;

        if(lc->constraint_type() == NConstraintInterface::NC_EQUAL)
        {
          v = std::abs(v);
          if(v>acceptable_tolerance)
            inf = true;
          else
            if(v>_almost_infeasible)
              almost_inf = true;
        }
        else
          if(lc->constraint_type() == NConstraintInterface::NC_GREATER_EQUAL)
          {
            if(v<-acceptable_tolerance)
              inf = true;
            else
              if(v<_almost_infeasible)
                almost_inf = true;
          }
          else
            if(lc->constraint_type() == NConstraintInterface::NC_LESS_EQUAL)
            {
              if(v>acceptable_tolerance)
                inf = true;
              else
                if(v>-_almost_infeasible)
                  almost_inf = true;
            }

        // infeasible?
        if(inf)
        {
          constraints.push_back(lc);
          lazy_added[i] = true;
          feasible_point_found = false;
          ++n_inf.back();
        }

        // almost violated or violated? -> add to constraints
        if(almost_inf)
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
    ++cur_pass;

    std::cerr << "*************** could not find feasible point after " << _max_passes-1 << " -> solving with all lazy constraints..." << std::endl;
    for(unsigned int i=0; i<_lazy_constraints.size(); ++i)
      if(!lazy_added[i])
        constraints.push_back(_lazy_constraints[i]);

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

    if(print_level_>=2) std::cerr << "exploit detected special properties: ";
    if(np2->hessian_constant())
    {
      if(print_level_>=2) std::cerr << "*constant hessian* ";
      app().Options()->SetStringValue("hessian_constant", "yes");
    }

    if(np2->jac_c_constant())
    {
      if(print_level_>=2) std::cerr << "*constant jacobian of equality constraints* ";
      app().Options()->SetStringValue("jac_c_constant", "yes");
    }

    if(np2->jac_d_constant())
    {
      if(print_level_>=2) std::cerr << "*constant jacobian of in-equality constraints*";
      app().Options()->SetStringValue("jac_d_constant", "yes");
    }
    if(print_level_>=2) std::cerr << std::endl;

    //----------------------------------------------------------------------------
    // 3. solve problem
    //----------------------------------------------------------------------------
    status = app_->OptimizeTNLP( np);
  }

  const double overall_time = sw.stop()/1000.0;

  //----------------------------------------------------------------------------
  // 4. output statistics
  //----------------------------------------------------------------------------
  if (status == Ipopt::Solve_Succeeded || status == Ipopt::Solved_To_Acceptable_Level)
  {
    if(print_level_>=2)
    {
      // Retrieve some statistics about the solve
      Ipopt::Index iter_count = app_->Statistics()->IterationCount();
      printf("\n\n*** IPOPT: The problem solved in %d iterations!\n", iter_count);

      Ipopt::Number final_obj = app_->Statistics()->FinalObjective();
      printf("\n\n*** IPOPT: The final value of the objective function is %e.\n", final_obj);
    }
  }

  if(print_level_>=2)
  {
    std::cerr <<"############# IPOPT with lazy constraints statistics ###############" << std::endl;
    std::cerr << "overall time: " << overall_time << "s" << std::endl;
    std::cerr << "#passes     : " << cur_pass << "( of " << _max_passes << ")" << std::endl;
    for(unsigned int i=0; i<n_inf.size(); ++i)
      std::cerr << "pass " << i << " induced " << n_inf[i] << " infeasible and " << n_almost_inf[i] << " almost infeasible" << std::endl;
  }

  return status;
}


//-----------------------------------------------------------------------------


int
IPOPTSolver::
solve(NProblemInterface*    _problem)
{
  std::vector<NConstraintInterface*> constraints;
  return this->solve(_problem, constraints);
}


//-----------------------------------------------------------------------------


int
IPOPTSolver::
solve(NProblemGmmInterface* _problem, std::vector<NConstraintInterface*>& _constraints)
{
  std::cerr << "****** Warning: NProblemGmmInterface is deprecated!!! -> use NProblemInterface *******\n";

  //----------------------------------------------------------------------------
  // 1. Create an instance of IPOPT NLP
  //----------------------------------------------------------------------------
  Ipopt::SmartPtr<Ipopt::TNLP> np = new NProblemGmmIPOPT(_problem, _constraints);

  //----------------------------------------------------------------------------
  // 2. solve problem
  //----------------------------------------------------------------------------

  // Initialize the IpoptApplication and process the options
  Ipopt::ApplicationReturnStatus status;
  status = app_->Initialize();
  if (status != Ipopt::Solve_Succeeded)
  {
    printf("\n\n*** Error IPOPT during initialization!\n");
  }

  //----------------------------------------------------------------------------
  // 3. solve problem
  //----------------------------------------------------------------------------
  status = app_->OptimizeTNLP(np);

  //----------------------------------------------------------------------------
  // 4. output statistics
  //----------------------------------------------------------------------------
  if (status == Ipopt::Solve_Succeeded || status == Ipopt::Solved_To_Acceptable_Level)
  {
    // Retrieve some statistics about the solve
    Ipopt::Index iter_count = app_->Statistics()->IterationCount();
    printf("\n\n*** IPOPT: The problem solved in %d iterations!\n", iter_count);

    Ipopt::Number final_obj = app_->Statistics()->FinalObjective();
    printf("\n\n*** IPOPT: The final value of the objective function is %e.\n", final_obj);
  }

  return status;
}


//=============================================================================
} // namespace COMISO
//=============================================================================
#endif // COMISO_IPOPT_AVAILABLE
//=============================================================================
