//=============================================================================
//
//  CLASS GUROBISolver - IMPLEMENTATION
//
//=============================================================================

//== INCLUDES =================================================================

//== COMPILE-TIME PACKAGE REQUIREMENTS ========================================
#include <CoMISo/Config/config.hh>
#if COMISO_GUROBI_AVAILABLE
//=============================================================================
 #include <gurobi_c++.h>

#include "GUROBISolver.hh"
#if (COMISO_QT_AVAILABLE)
#include "GurobiHelper.hh"
#endif//COMISO_QT_AVAILABLE
#include <CoMISo/Utils/CoMISoError.hh>
#include <CoMISo/Utils/StopWatch.hh>


#include <Base/Debug/DebTime.hh>
#include <stdexcept>

//== NAMESPACES ===============================================================

namespace COMISO {

//== IMPLEMENTATION ========================================================== 

double* P(std::vector<double>& _v)
{
  if( !_v.empty())
    return ((double*)&_v[0]);
  else
    return 0;
}


//-----------------------------------------------------------------------------


void add_constraint_to_model(COMISO::NConstraintInterface* _constraint, 
  GRBModel& _model, const std::vector<GRBVar>& _vars, const double* _x, int _lazy = 0)
{
  DEB_enter_func;
  DEB_warning_if(!_constraint->is_linear(), 1,
     "GUROBISolver received a problem with non-linear constraints!!!\n");

  GRBLinExpr lin_expr;
  NConstraintInterface::SVectorNC gc;
  _constraint->eval_gradient(_x, gc);

  NConstraintInterface::SVectorNC::InnerIterator v_it(gc);
  for(; v_it; ++v_it)
    lin_expr += _vars[v_it.index()]*v_it.value();

  double b = _constraint->eval_constraint(_x);

  GRBConstr constr;
  switch(_constraint->constraint_type())
  {
    case NConstraintInterface::NC_EQUAL         : constr = _model.addConstr(lin_expr + b == 0); break;
    case NConstraintInterface::NC_LESS_EQUAL    : constr = _model.addConstr(lin_expr + b <= 0); break;
    case NConstraintInterface::NC_GREATER_EQUAL : constr = _model.addConstr(lin_expr + b >= 0); break;
  }
  if (_lazy > 0)
    constr.set(GRB_IntAttr_Lazy, _lazy);
}


//-----------------------------------------------------------------------------

std::vector<GRBVar> allocate_variables(NProblemInterface* _problem, const std::vector<PairIndexVtype>& _discrete_constraints, GRBModel& _model)
{
  // determine variable types: 0->real, 1->integer, 2->bool
  std::vector<char> vtypes(_problem->n_unknowns(),0);
  for(unsigned int i=0; i<_discrete_constraints.size(); ++i)
    switch(_discrete_constraints[i].second)
    {
      case Integer: vtypes[_discrete_constraints[i].first] = 1; break;
      case Binary : vtypes[_discrete_constraints[i].first] = 2; break;
      default     : break;
    }

  // GUROBI variables
  std::vector<GRBVar> vars;
  // first all
  for( int i=0; i<_problem->n_unknowns(); ++i)
    switch(vtypes[i])
    {
      case 0 : vars.push_back( _model.addVar(-GRB_INFINITY, GRB_INFINITY, 0.0, GRB_CONTINUOUS) ); break;
      case 1 : vars.push_back( _model.addVar(-GRB_INFINITY, GRB_INFINITY, 0.0, GRB_INTEGER   ) ); break;
      case 2 : vars.push_back( _model.addVar(-GRB_INFINITY, GRB_INFINITY, 0.0, GRB_BINARY    ) ); break;
    }

  // Integrate new variables
  _model.update();

  return vars;
}


//-----------------------------------------------------------------------------


void set_start(NProblemInterface* _problem, GRBModel& _model, std::vector<GRBVar>& _vars, const std::string& _start_solution_output_path)
{
  // set start
  std::vector<double> start(_vars.size());
  _problem->initial_x(start.data());

  for (int i = 0; i < _problem->n_unknowns(); ++i)
    _vars[i].set(GRB_DoubleAttr_Start, start[i]);

  _model.update();

  if (!_start_solution_output_path.empty())
  {
    std::cout << "Writing problem's start solution into file \"" << _start_solution_output_path << "\"." << std::endl;
    _model.write(_start_solution_output_path);
  }
}


//-----------------------------------------------------------------------------


void setup_constraints(NProblemInterface* _problem, const std::vector<NConstraintInterface *> &_constraints, GRBModel& _model,  const std::vector<GRBVar>& _vars)
{
  DEB_enter_func;

  // get zero vector
  std::vector<double> x(_problem->n_unknowns(), 0.0);

  for(unsigned int i=0; i<_constraints.size();  ++i)
  {
    DEB_warning_if(!_constraints[i]->is_linear(), 1,
      "GUROBISolver received a problem with non-linear constraints!!!");

    add_constraint_to_model(_constraints[i], _model, _vars, P(x));
  }
  _model.update();
}


//-----------------------------------------------------------------------------


void setup_energy(NProblemInterface* _problem, GRBModel& _model,  const std::vector<GRBVar>& _vars)
{
  DEB_enter_func;

  DEB_warning_if(!_problem->constant_hessian(), 1,
    "GUROBISolver received a problem with non-constant hessian!!!");

  GRBQuadExpr objective;

  // get zero vector
  std::vector<double> x(_problem->n_unknowns(), 0.0);

  // add quadratic part
  NProblemInterface::SMatrixNP H;
  _problem->eval_hessian(P(x), H);
  for( int i=0; i<H.outerSize(); ++i)
  {
    for (NProblemInterface::SMatrixNP::InnerIterator it(H,i); it; ++it)
      objective += 0.5*it.value()*_vars[it.row()]*_vars[it.col()];
  }

  // add linear part
  std::vector<double> g(_problem->n_unknowns());
  _problem->eval_gradient(P(x), P(g));
  for(unsigned int i=0; i<g.size(); ++i)
    objective += g[i]*_vars[i];

  // add constant part
  objective += _problem->eval_f(P(x));

  _model.set(GRB_IntAttr_ModelSense, 1);
  _model.setObjective(objective);
  _model.update();
}

//-----------------------------------------------------------------------------


double solve_problem_two_phase(GRBModel& _model, double _time_limit0, double _gap0, double _time_limit1, double _gap1,
                             const std::string& _solution_input_path, const std::string& _problem_env_output_path, const std::string& _problem_output_path)
{
  DEB_enter_func;

  double final_gap = -1;
  if (_solution_input_path.empty())
  {
    if (!_problem_env_output_path.empty())
    {
      DEB_line(5, "Writing problem's environment into file \"" << _problem_env_output_path << "\".");
      _model.getEnv().writeParams(_problem_env_output_path);
    }
#if (COMISO_QT_AVAILABLE)
    if (!_problem_output_path.empty())
    {
      DEB_line(5, "Writing problem into file \"" << _problem_output_path << "\".");
      GurobiHelper::outputModelToMpsGz(_model, _problem_output_path);
    }
#endif//COMISO_QT_AVAILABLE

    // optimize
    _model.getEnv().set(GRB_DoubleParam_TimeLimit, _time_limit0);
    _model.getEnv().set(GRB_DoubleParam_MIPGap, _gap0);
    _model.optimize();
    final_gap = _model.get(GRB_DoubleAttr_MIPGap);

    // jump into phase 2?
    if(_model.get(GRB_DoubleAttr_MIPGap) > _gap1 && _time_limit1 > 0)
    {
      _model.getEnv().set(GRB_DoubleParam_TimeLimit, _time_limit1);
      _model.getEnv().set(GRB_DoubleParam_MIPGap, _gap1);
      _model.optimize();
      final_gap = _model.get(GRB_DoubleAttr_MIPGap);
    }
  }
  else
  {
      DEB_line(5, "Reading solution from file \"" << _solution_input_path << "\".");
  }

  return final_gap;
}


//-----------------------------------------------------------------------------


double solve_problem(GRBModel& _model, double _time_limit, double _gap, const std::string& _solution_input_path, const std::string& _problem_env_output_path, const std::string& _problem_output_path)
{
  return solve_problem_two_phase(_model, _time_limit, _gap, 0, 1000,
                                 _solution_input_path, _problem_env_output_path, _problem_output_path);
}



//-----------------------------------------------------------------------------


void store_result(NProblemInterface* _problem, GRBModel& _model, const std::vector<GRBVar>& _vars, const std::string& _solution_input_path)
{
  DEB_enter_func;

  // get zero vector
  std::vector<double> x(_problem->n_unknowns(), 0.0);

  if (_solution_input_path.empty())
  {
    // store computed result
    for(unsigned int i=0; i<_vars.size(); ++i)
      x[i] = _vars[i].get(GRB_DoubleAttr_X);
  }
  else
  {
#if (COMISO_QT_AVAILABLE)
      DEB_line(5, "Loading stored solution from \"" << _solution_input_path << "\".");
      // store loaded result
      const size_t oldSize = x.size();
      x.clear();
      GurobiHelper::readSolutionVectorFromSOL(x, _solution_input_path);
      COMISO_THROW_TODO_if(oldSize != x.size(),
                           "oldSize != x.size() <=> " << oldSize << " != " << x.size() <<
                           "\nLoaded solution vector doesn't have expected dimension.");
#endif//COMISO_QT_AVAILABLE
  }

  _problem->store_result(P(x));

  // ObjVal is only available if the optimize was called.
  DEB_out_if(_solution_input_path.empty(), 2,
      "GUROBI Objective: " << _model.get(GRB_DoubleAttr_ObjVal) << "\n");
}


//-----------------------------------------------------------------------------


static void process_gurobi_exception(const GRBException& _exc)
{
  DEB_enter_func;
  DEB_error("Gurobi exception error code = " << _exc.getErrorCode() << 
    " and message: [" << _exc.getMessage() << "]");
    
  // NOTE: we could propagate e.getMessage() either using std::exception, 
  // or a specialized Reform exception type

  // The GRB_ error codes are defined in gurobi_c.h Gurobi header. 
  switch (_exc.getErrorCode())
  {
  case GRB_ERROR_NO_LICENSE: 
    COMISO_THROW(GUROBI_LICENCE_ABSENT); 
  case GRB_ERROR_SIZE_LIMIT_EXCEEDED: 
    COMISO_THROW(GUROBI_LICENCE_MODEL_TOO_LARGE); break;
  default: 
    COMISO_THROW(UNSPECIFIED_GUROBI_EXCEPTION);
  }
}

bool
GUROBISolver::
solve(NProblemInterface*                  _problem,
      const std::vector<NConstraintInterface *> &_constraints,
      const std::vector<PairIndexVtype>&  _discrete_constraints,
      const double                        _time_limit,
      const double                        _gap)
{
  DEB_enter_func;
  try
  {
    GRBEnv   env   = GRBEnv();
    GRBModel model = GRBModel(env);

    auto vars = allocate_variables(_problem, _discrete_constraints, model);
    set_start(_problem, model, vars, start_solution_output_path_);
    setup_constraints(_problem, _constraints, model, vars);
    setup_energy(_problem, model, vars);
    solve_problem(model, _time_limit, _gap, solution_input_path_, problem_env_output_path_, problem_output_path_);
    store_result(_problem, model, vars, solution_input_path_);

    return true;
  }
  catch(GRBException& e)
  {
    //process_gurobi_exception(e);
    DEB_error("Error code = " << e.getErrorCode());
    DEB_error(e.getMessage());
    return false;
  }
  catch(...)
  {
    DEB_error("Exception during optimization");
    return false;
  }
}


//-----------------------------------------------------------------------------

bool
GUROBISolver::
solve_two_phase(NProblemInterface*                  _problem,                // problem instance
            const std::vector<NConstraintInterface*>& _constraints,            // linear constraints
            const std::vector<PairIndexVtype>&        _discrete_constraints,   // discrete constraints
            const double                        _time_limit0, // time limit phase 1 in seconds
            const double                        _gap0,     // MIP gap phase 1
            const double                        _time_limit1, // time limit phase 2 in seconds
            const double                        _gap1)       // MIP gap phase 2
{
  double dummy;
  return solve_two_phase(_problem, _constraints, _discrete_constraints, _time_limit0, _gap0, _time_limit1, _gap1, dummy);
}


bool
GUROBISolver::
solve_two_phase(NProblemInterface*                  _problem,                // problem instance
           const std::vector<NConstraintInterface *> &_constraints,            // linear constraints
           const std::vector<PairIndexVtype> &_discrete_constraints,   // discrete constraints
           const double                        _time_limit0, // time limit phase 1 in seconds
           const double                        _gap0,     // MIP gap phase 1
           const double                        _time_limit1, // time limit phase 2 in seconds
           const double                        _gap1,       // MIP gap phase 2
           double&                             _final_gap)  //return final gap
{
  DEB_enter_func;

  try
  {
    GRBEnv   env   = GRBEnv();
    GRBModel model = GRBModel(env);

    auto vars = allocate_variables(_problem, _discrete_constraints, model);
    set_start(_problem, model, vars, start_solution_output_path_);
    setup_constraints(_problem, _constraints, model, vars);
    setup_energy(_problem, model, vars);
    _final_gap = solve_problem_two_phase(model, _time_limit0, _gap0, _time_limit1, _gap1,
                                         solution_input_path_, problem_env_output_path_, problem_output_path_);
    store_result(_problem, model, vars, solution_input_path_);

    return true;
  }
  catch(GRBException& e)
  {
    //process_gurobi_exception(e);
    DEB_error("Error code = " << e.getErrorCode());
    DEB_error(e.getMessage());
    return false;
  }
  catch(...)
  {
    DEB_error("Exception during optimization");
    return false;
  }

  return false;
}


//-----------------------------------------------------------------------------


bool
GUROBISolver::
solve(NProblemInterface*                        _problem,
      const std::vector<NConstraintInterface*>& _constraints,
      const std::vector<NConstraintInterface*>& _lazy_constraints,
      std::vector<PairIndexVtype>&              _discrete_constraints,   // discrete constraints
      const double                              _almost_infeasible,
      const int                                 _max_passes,
      const double                              _time_limit,
      const bool                                _silent)
{
  DEB_time_func_def;
//  // hack! solve with all constraints
//  std::vector<NConstraintInterface*> all_constraints;
//  std::copy(_constraints.begin(),_constraints.end(),std::back_inserter(all_constraints));
//  std::copy(_lazy_constraints.begin(),_lazy_constraints.end(),std::back_inserter(all_constraints));
//
//  return solve(_problem, all_constraints, _discrete_constraints, _time_limit);

  StopWatch sw; sw.start();

  bool feasible_point_found = false;
  int  cur_pass = 0;
  double acceptable_tolerance = 0.01; // hack: read out from ipopt!!!
  std::vector<bool> lazy_added(_lazy_constraints.size(),false);

  // cache statistics of all iterations
  std::vector<int> n_inf;
  std::vector<int> n_almost_inf;

  try
  {
    //----------------------------------------------
    // 0. set up environment
    //----------------------------------------------

    GRBEnv   env   = GRBEnv();
    GRBModel model = GRBModel(env);

    auto vars = allocate_variables(_problem, _discrete_constraints, model);
    set_start(_problem, model, vars, start_solution_output_path_);
    setup_constraints(_problem, _constraints, model, vars);
    setup_energy(_problem, model, vars);

    //----------------------------------------------
    // 4. iteratively solve problem
    //----------------------------------------------

    // get zero vector
    std::vector<double> x(_problem->n_unknowns(), 0.0);
    model.getEnv().set(GRB_DoubleParam_TimeLimit, _time_limit);
    bool solution_found = false;

    while(!feasible_point_found && cur_pass <(_max_passes-1))
    {
      ++cur_pass;
      //----------------------------------------------------------------------------
      // 1. optimize current Model and get result
      //----------------------------------------------------------------------------

      model.update();
      model.optimize();
      // store computed result
      if(model.get(GRB_IntAttr_Status) == GRB_OPTIMAL)
        for(unsigned int i=0; i<vars.size(); ++i)
          x[i] = vars[i].get(GRB_DoubleAttr_X);

      //----------------------------------------------------------------------------
      // 2. Check lazy constraints
      //----------------------------------------------------------------------------
      // check lazy constraints
      n_inf.push_back(0);
      n_almost_inf.push_back(0);
      feasible_point_found = true;
      for(unsigned int i=0; i<_lazy_constraints.size(); ++i)
        if(!lazy_added[i])
        {
          NConstraintInterface* lc = _lazy_constraints[i];

          double v = lc->eval_constraint(P(x));

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
          else if(lc->constraint_type() == NConstraintInterface::NC_GREATER_EQUAL)
          {
            if(v<-acceptable_tolerance)
              inf = true;
            else
              if(v<_almost_infeasible)
                almost_inf = true;
          }
          else if(lc->constraint_type() == NConstraintInterface::NC_LESS_EQUAL)
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
            add_constraint_to_model( lc, model, vars, P(x));
            lazy_added[i] = true;
            feasible_point_found = false;
            ++n_inf.back();
          }
          else if(almost_inf) // almost violated or violated? -> add to constraints
          {
            add_constraint_to_model( lc, model, vars, P(x));
            lazy_added[i] = true;
            ++n_almost_inf.back();
          }
        }
    }

    // no termination after max number of passes?
    if(!feasible_point_found)
    {
      ++cur_pass;

      DEB_out(1, "*************** could not find feasible point after " 
        << _max_passes-1 << " -> solving with all lazy constraints...\n");
      for(unsigned int i=0; i<_lazy_constraints.size(); ++i)
        if(!lazy_added[i])
        {
          add_constraint_to_model( _lazy_constraints[i], model, vars, P(x));
        }

      model.update();
      model.optimize();

      // store computed result
      if(model.get(GRB_IntAttr_Status) == GRB_OPTIMAL)
        for(unsigned int i=0; i<vars.size(); ++i)
          x[i] = vars[i].get(GRB_DoubleAttr_X);
    }

    const double overall_time = sw.stop()/1000.0;

    //----------------------------------------------------------------------------
    // 4. output statistics
    //----------------------------------------------------------------------------
    // Make this code DEB_only
    DEB_line(2,
      "############# GUROBI with lazy constraints statistics ###############");
    DEB_line(2, "#passes     : " << cur_pass << "( of " << _max_passes << ")");
    for(unsigned int i=0; i<n_inf.size(); ++i)
    {
      DEB_line(2, "pass " << i << " induced " << n_inf[i]
        << " infeasible and " << n_almost_inf[i] << " almost infeasible");
    }
    DEB_line(2, "GUROBI Objective: " << model.get(GRB_DoubleAttr_ObjVal));

    //----------------------------------------------
    // 5. store result
    //----------------------------------------------
    //COMISO_THROW_TODO_if(model.get(GRB_IntAttr_Status) != GRB_OPTIMAL, 
    //  "Gurobi solution not optimal");

    if(model.get(GRB_IntAttr_Status) != GRB_OPTIMAL)
      return false;
    else
    {
      _problem->store_result(P(x));
      return true;
    }
  }
  catch(GRBException& e)
  {
    //process_gurobi_exception(e);
    DEB_error("Error code = " << e.getErrorCode());
    DEB_error(e.getMessage());
    return false;
  }
  catch(...)
  {
    DEB_error("Exception during optimization");
    return false;
  }

  return false;

//    //----------------------------------------------
//    // 4. iteratively solve problem
//    //----------------------------------------------
//    IloBool solution_found = IloFalse;
//
//    while(!feasible_point_found && cur_pass <(_max_passes-1))
//    {
//      ++cur_pass;
//      //----------------------------------------------------------------------------
//      // 1. Create an instance of current Model and optimize
//      //----------------------------------------------------------------------------
//
//      if(!_silent)
//        std::cerr << "cplex -> setup IloCPlex...\n";
//      IloCplex cplex(model);
//      cplex.setParam(IloCplex::TiLim, _time_limit);
//      // silent mode?
//      if(_silent)
//        cplex.setOut(env_.getNullStream());
//
//      if(!_silent)
//        std::cerr << "cplex -> optimize...\n";
//      solution_found = cplex.solve();
//
//      if(solution_found != IloFalse)
//      {
//        for(unsigned int i=0; i<vars.size(); ++i)
//          x[i] = cplex.getValue(vars[i]);
//
//        _problem->store_result(P(x));
//      }
//      else continue;
//
//      //----------------------------------------------------------------------------
//      // 2. Check lazy constraints
//      //----------------------------------------------------------------------------
//      // check lazy constraints
//      n_inf.push_back(0);
//      n_almost_inf.push_back(0);
//      feasible_point_found = true;
//      for(unsigned int i=0; i<_lazy_constraints.size(); ++i)
//        if(!lazy_added[i])
//        {
//          NConstraintInterface* lc = _lazy_constraints[i];
//
//          double v = lc->eval_constraint(P(x));
//
//          bool inf        = false;
//          bool almost_inf = false;
//
//          if(lc->constraint_type() == NConstraintInterface::NC_EQUAL)
//          {
//            v = std::abs(v);
//            if(v>acceptable_tolerance)
//              inf = true;
//            else
//              if(v>_almost_infeasible)
//                almost_inf = true;
//          }
//          else
//            if(lc->constraint_type() == NConstraintInterface::NC_GREATER_EQUAL)
//            {
//              if(v<-acceptable_tolerance)
//                inf = true;
//              else
//                if(v<_almost_infeasible)
//                  almost_inf = true;
//            }
//            else
//              if(lc->constraint_type() == NConstraintInterface::NC_LESS_EQUAL)
//              {
//                if(v>acceptable_tolerance)
//                  inf = true;
//                else
//                  if(v>-_almost_infeasible)
//                    almost_inf = true;
//              }
//
//          // infeasible?
//          if(inf)
//          {
//            add_constraint_to_model( lc, vars, model, env_, P(x));
//            lazy_added[i] = true;
//            feasible_point_found = false;
//            ++n_inf.back();
//          }
//          else // almost violated or violated? -> add to constraints
//            if(almost_inf)
//            {
//              add_constraint_to_model( lc, vars, model, env_, P(x));
//              lazy_added[i] = true;
//              ++n_almost_inf.back();
//            }
//        }
//    }
//
//    // no termination after max number of passes?
//    if(!feasible_point_found)
//    {
//      ++cur_pass;
//
//      std::cerr << "*************** could not find feasible point after " << _max_passes-1 << " -> solving with all lazy constraints..." << std::endl;
//      for(unsigned int i=0; i<_lazy_constraints.size(); ++i)
//        if(!lazy_added[i])
//        {
//          add_constraint_to_model( _lazy_constraints[i], vars, model, env_, P(x));
//        }
//
//      //----------------------------------------------------------------------------
//      // 1. Create an instance of current Model and optimize
//      //----------------------------------------------------------------------------
//
//      IloCplex cplex(model);
//      cplex.setParam(IloCplex::TiLim, _time_limit);
//      // silent mode?
//      if(_silent)
//        cplex.setOut(env_.getNullStream());
//      solution_found = cplex.solve();
//
//      if(solution_found != IloFalse)
//      {
//        for(unsigned int i=0; i<vars.size(); ++i)
//          x[i] = cplex.getValue(vars[i]);
//
//        _problem->store_result(P(x));
//      }
//    }
//
//    const double overall_time = sw.stop()/1000.0;
//
//    //----------------------------------------------------------------------------
//    // 4. output statistics
//    //----------------------------------------------------------------------------
////    if (solution_found != IloFalse)
////    {
////      // Retrieve some statistics about the solve
////      Ipopt::Index iter_count = app_->Statistics()->IterationCount();
////      printf("\n\n*** IPOPT: The problem solved in %d iterations!\n", iter_count);
////
////      Ipopt::Number final_obj = app_->Statistics()->FinalObjective();
////      printf("\n\n*** IPOPT: The final value of the objective function is %e.\n", final_obj);
////    }
//
//    std::cerr <<"############# CPLEX with lazy constraints statistics ###############" << std::endl;
//    std::cerr << "overall time: " << overall_time << "s" << std::endl;
//    std::cerr << "#passes     : " << cur_pass << "( of " << _max_passes << ")" << std::endl;
//    for(unsigned int i=0; i<n_inf.size(); ++i)
//      std::cerr << "pass " << i << " induced " << n_inf[i] << " infeasible and " << n_almost_inf[i] << " almost infeasible" << std::endl;
//
  //    return (solution_found != IloFalse);
}

bool
GUROBISolver::
solve(NProblemInterface* _problem,
      const std::vector<NConstraintInterface*>& _constraints,
      const std::vector<NConstraintInterface*>& _lazy_constraints,
      const std::vector<PairIndexVtype>& _discrete_constraints,
      const double _time_limit,
      const double _gap,
      const int _lazy_level)
{
  DEB_enter_func;
  try
  {
    //----------------------------------------------
    // 0. set up environment
    //----------------------------------------------

    GRBEnv   env   = GRBEnv();
    GRBModel model = GRBModel(env);

    //----------------------------------------------
    // 1. allocate variables
    //----------------------------------------------
    auto vars = allocate_variables(_problem, _discrete_constraints, model);
    set_start(_problem, model, vars, start_solution_output_path_);

    //----------------------------------------------
    // 2. setup constraints
    //----------------------------------------------

    // get zero vector
    std::vector<double> x(_problem->n_unknowns(), 0.0);

    for(unsigned int i=0; i<_constraints.size();  ++i)
    {
      add_constraint_to_model(_constraints[i], model, vars, P(x));
    }
    model.update();

    for(unsigned int i=0; i<_lazy_constraints.size();  ++i)
    {
      add_constraint_to_model(_lazy_constraints[i], model, vars, P(x), _lazy_level);
    }
    model.update();

    //----------------------------------------------
    // 3. setup energy
    //----------------------------------------------
    setup_energy(_problem, model, vars);

    //----------------------------------------------
    // 4. solve problem
    //----------------------------------------------
    solve_problem(model, _time_limit, _gap, solution_input_path_, problem_env_output_path_, problem_output_path_);

    //----------------------------------------------
    // 5. store result
    //----------------------------------------------
    store_result(_problem, model, vars, solution_input_path_);

    return true;
  }
  catch(GRBException& e)
  {
    //process_gurobi_exception(e);
    DEB_error("Error code = " << e.getErrorCode());
    DEB_error(e.getMessage());
    return false;
  }
  catch(...)
  {
    DEB_error("Exception during optimization");
    return false;
  }

  return false;
}


//-----------------------------------------------------------------------------


void
GUROBISolver::
set_problem_output_path( const std::string &_problem_output_path)
{
  problem_output_path_ = _problem_output_path;
}


//-----------------------------------------------------------------------------


void
GUROBISolver::
set_start_solution_output_path(const std::string& _start_solution_output_path)
{
  std::stringstream ss;
  ss << _start_solution_output_path << ".mst";
  start_solution_output_path_ = ss.str();
}


//-----------------------------------------------------------------------------


void
GUROBISolver::
set_problem_env_output_path( const std::string &_problem_env_output_path)
{
  problem_env_output_path_ = _problem_env_output_path;
}


//-----------------------------------------------------------------------------


void
GUROBISolver::
set_solution_input_path(const std::string &_solution_input_path)
{
  solution_input_path_ = _solution_input_path;
}


//=============================================================================
} // namespace COMISO
//=============================================================================
#endif // COMISO_GUROBI_AVAILABLE
//=============================================================================
