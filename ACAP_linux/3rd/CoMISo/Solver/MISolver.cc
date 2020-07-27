/*===========================================================================*\
 *                                                                           *
 *                               CoMISo                                      *
 *      Copyright (C) 2008-2009 by Computer Graphics Group, RWTH Aachen      *
 *                           www.rwth-graphics.de                            *
 *                                                                           *
 *---------------------------------------------------------------------------* 
 *  This file is part of CoMISo.                                             *
 *                                                                           *
 *  CoMISo is free software: you can redistribute it and/or modify           *
 *  it under the terms of the GNU General Public License as published by     *
 *  the Free Software Foundation, either version 3 of the License, or        *
 *  (at your option) any later version.                                      *
 *                                                                           *
 *  CoMISo is distributed in the hope that it will be useful,                *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
 *  GNU General Public License for more details.                             *
 *                                                                           *
 *  You should have received a copy of the GNU General Public License        *
 *  along with CoMISo.  If not, see <http://www.gnu.org/licenses/>.          *
 *                                                                           *
\*===========================================================================*/ 


#include <CoMISo/Config/config.hh>
#include "MISolver.hh"
#include "CoMISo/Utils/CoMISoError.hh"

#if(COMISO_QT_AVAILABLE)
#include <CoMISo/QtWidgets/MISolverDialogUI.hh>
#endif

#if COMISO_GUROBI_AVAILABLE
  #include <gurobi_c++.h>
#endif

#include <Base/Debug/DebOut.hh>
#include <Base/Utils/StopWatch.hh>

#include <gmm/gmm.h>
#include <float.h>

// hack for testing only
#include "SparseQRSolver.hh"
#include "UMFPACKSolver.hh"
#include "EigenLDLTSolver.hh"

#define ROUND(x) ((x)<0?int((x)-0.5):int((x)+0.5))

namespace COMISO {



// Constructor
MISolver::MISolver() 
{
  // default parameters
  initial_full_solution_ = true;
  iter_full_solution_    = true;
  final_full_solution_   = true;

  direct_rounding_   = false;
  no_rounding_       = false;
  multiple_rounding_ = true;
  gurobi_rounding_   = false;
  cplex_rounding_    = false;

  max_local_iters_ = 100000;
  max_local_error_ = 1e-3;
  max_cg_iters_    = 50;
  max_cg_error_    = 1e-3;

  multiple_rounding_threshold_ = 0.5;

  gurobi_max_time_ = 60;

  noisy_ = 0;
  stats_ = true;

  use_constraint_reordering_ = true;
}


//-----------------------------------------------------------------------------

void
MISolver::solve(
    CSCMatrix& _A,
    Vecd&      _x,
    Vecd&      _rhs,
    Veci&      _to_round,
    bool       _fixed_order )
{
  DEB_enter_func

  DEB_out(2, "# integer    variables: " << _to_round.size() 
    << "\n# continuous variables: " << _x.size()-_to_round.size() << "\n")

  // nothing to solve?
  if( gmm::mat_ncols(_A) == 0 || gmm::mat_nrows(_A) == 0)
          return;

  if( gurobi_rounding_)
    solve_gurobi(_A, _x, _rhs, _to_round);
  else if( cplex_rounding_)
    solve_cplex(_A, _x, _rhs, _to_round);
  else if( no_rounding_ || _to_round.size() == 0)
    solve_no_rounding( _A, _x, _rhs);
  else if( direct_rounding_)
    solve_direct_rounding( _A, _x, _rhs, _to_round);
  else if( multiple_rounding_)
    solve_multiple_rounding( _A, _x, _rhs, _to_round);
  else
    solve_iterative( _A, _x, _rhs, _to_round, _fixed_order);
}


//-----------------------------------------------------------------------------

// inline function...
void
MISolver::solve_cplex(
    CSCMatrix& _A,
    Vecd&      _x,
    Vecd&      _rhs,
    Veci&      _to_round)
{
  DEB_enter_func;
        DEB_out(2, "gurobi_max_time_: " << gurobi_max_time_ << "\n")

#if COMISO_CPLEX_AVAILABLE

  // get round-indices in set
  std::set<int> to_round;
  for(unsigned int i=0; i<_to_round.size();++i)
    to_round.insert(_to_round[i]);

  try {

    IloEnv env_;
    IloModel model(env_);

    // set time limite
    //    model.getEnv().set(GRB_DoubleParam_TimeLimit, gurobi_max_time_);

    unsigned int n = _rhs.size();

    // 1. allocate variables
    std::vector<IloNumVar> vars;
    for( unsigned int i=0; i<n; ++i)
      if( to_round.count(i))
        vars.push_back( IloNumVar(env_,  -IloIntMax,   IloIntMax, IloNumVar::Int)   );
      else
        vars.push_back( IloNumVar(env_,-IloInfinity, IloInfinity, IloNumVar::Float) );


    // 2. setup_energy

    // build objective function from linear system E = x^tAx - 2x^t*rhs
    IloExpr objective(env_);

    for(unsigned int i=0; i<_A.nc; ++i)
      for(unsigned int j=_A.jc[i]; j<_A.jc[i+1]; ++j)
      {
        objective += _A.pr[j]*vars[_A.ir[j]]*vars[i];
      }
    for(unsigned int i=0; i<n; ++i)
      objective -= 2*_rhs[i]*vars[i];

    // ToDo: objective correction!!!

    //    _A.jc[c+1]
    //    _A.pr[write]
    //    _A.ir[write]
    //    _A.nc
    //    _A.nr

    // minimize
    model.add(IloMinimize(env_,objective));

    // 4. solve
    IloCplex cplex(model);
    cplex.setParam(IloCplex::TiLim, gurobi_max_time_);

//    // set parameters comparable to CoMISo
//    {
//      cplex.setParam(IloCplex::MIPSearch  , 1);  // Traditional Branch-and-Cut
//      cplex.setParam(IloCplex::NodeSel    , 0);  // Depth-First
//      cplex.setParam(IloCplex::VarSel     , -1);  // closest to integer
//      cplex.setParam(IloCplex::MIPEmphasis, 1);  // concentrate on feasibility
//    }

    cplex.solve();

    // 5. store result
    _x.resize(n);
    for(unsigned int i=0; i<n; ++i)
      _x[i] = cplex.getValue(vars[i]);

    DEB_out(2, "CPLEX objective: " << cplex.getObjValue() <<"\n");

  }
  catch (IloException& e)
  {
    PROGRESS_RESUME_ABORT; // resume a processed abort request
    DEB_warning(2, "CPLEX Concert exception caught: " << e.getMessage() )
  }
  catch (...)
  {
    PROGRESS_RESUME_ABORT; // resume a processed abort request
    DEB_warning(1, "CPLEX Unknown exception caught" )
  }


#else
  DEB_out(1, "CPLEX solver is not available, please install it...\n")
#endif
}

//-----------------------------------------------------------------------------


void 
MISolver::solve_no_rounding( 
    CSCMatrix& _A, 
    Vecd&      _x, 
    Vecd&      _rhs )
{
  COMISO_THROW_if(!direct_solver_.calc_system_gmm(_A), UNSPECIFIED_EIGEN_FAILURE);
  COMISO_THROW_if(!direct_solver_.solve(_x, _rhs), UNSPECIFIED_EIGEN_FAILURE);
}


//-----------------------------------------------------------------------------


void 
MISolver::resolve(
    Vecd&      _x,
    Vecd&      _rhs )
{
  direct_solver_.solve(_x, _rhs);
}


//-----------------------------------------------------------------------------


void
MISolver::solve_direct_rounding( 
    CSCMatrix& _A, 
    Vecd&      _x, 
    Vecd&      _rhs, 
    Veci&      _to_round)
{
  DEB_enter_func
  Veci to_round(_to_round);
  // copy to round vector and make it unique
  std::sort(to_round.begin(), to_round.end());
  Veci::iterator last_unique;
  last_unique = std::unique(to_round.begin(), to_round.end());
  size_t r = (size_t)(last_unique - to_round.begin());
  to_round.resize( r);

  // initalize old indices
  Veci old_idx(_rhs.size());
  for(unsigned int i=0; i<old_idx.size(); ++i)
    old_idx[i] = i;
  direct_solver_.calc_system_gmm(_A);
  direct_solver_.solve(_x, _rhs);

  // check solver performance (only for testing!!!)
  {
    Base::StopWatch sw;

    // hack
    const bool enable_performance_test = false;

    // performance comparison code
#if(COMISO_SUITESPARSE_SPQR_AVAILABLE)
    if(enable_performance_test)
    {
      sw.start();
      COMISO::SparseQRSolver spqr;
      spqr.calc_system_gmm(_A);
      std::cerr << "SparseQR factor took: " << sw.stop()/1000.0 << "s\n";
      Vecd x2(_x);
      sw.start();
      spqr.solve(x2,_rhs);
      std::cerr << "SparseQR solve took: " << sw.stop()/1000.0 << "s\n";
      Vecd res(_x);
      gmm::add(_x,gmm::scaled(x2,-1.0),res);
      std::cerr << "DIFFERENCE IN RESULT: " << gmm::vect_norm2(res) << std::endl;
    }
#endif

    // performance comparison code
#if(COMISO_SUITESPARSE_AVAILABLE)
    if(enable_performance_test)
    {
      sw.start();
      COMISO::UMFPACKSolver umf;
      umf.calc_system_gmm(_A);
      std::cerr << "UMFPack factor took: " << sw.stop()/1000.0 << "s\n";
      Vecd x3(_x);
      sw.start();
      umf.solve(x3,_rhs);
      std::cerr << "UMFPack solve took: " << sw.stop()/1000.0 << "s\n";
      Vecd res2(_x);
      gmm::add(_x,gmm::scaled(x3,-1.0),res2);
      std::cerr << "UMFPACK DIFFERENCE IN RESULT: " << gmm::vect_norm2(res2) << std::endl;
    }

    // performance comparison code
    if(enable_performance_test)
    {
      sw.start();
      COMISO::CholmodSolver chol;
      chol.calc_system_gmm(_A);
      std::cerr << "Choldmod factor took: " << sw.stop()/1000.0 << "s\n";
      Vecd x4(_x);
      sw.start();
      chol.solve(x4,_rhs);
      std::cerr << "Choldmod solve took: " << sw.stop()/1000.0 << "s\n";
      Vecd res(_x);
      gmm::add(_x,gmm::scaled(x4,-1.0),res);
      std::cerr << "DIFFERENCE IN RESULT: " << gmm::vect_norm2(res) << std::endl;
    }
#endif

#if(COMISO_EIGEN3_AVAILABLE)
    // performance comparison code
    if(enable_performance_test)
    {
      sw.start();
      COMISO::EigenLDLTSolver ldlt;
      ldlt.calc_system_gmm(_A);
      DEB_out(2, "Eigen LDLT factor took: " << sw.stop()/1000.0 << "s\n")
      Vecd x5(_x);
      sw.start();
      ldlt.solve(x5,_rhs);
      DEB_out(2, "Eigen LDLT solve took: " << sw.stop()/1000.0 << "s\n")
      Vecd res(_x);
      gmm::add(_x,gmm::scaled(x5,-1.0),res);
      DEB_warning(2, "DIFFERENCE IN RESULT: " << gmm::vect_norm2(res) )
    }
#endif
  }

  // round and eliminate variables
  Vecui elim_i;
  Vecd  elim_v;
  for( unsigned int i=0; i < to_round.size(); ++i)
  {
    _x[to_round[i]] = ROUND(_x[to_round[i]]);
    elim_i.push_back(to_round[i]);
    elim_v.push_back(_x[to_round[i]]);
    // update old idx
    old_idx[to_round[i]] = -1;
  }

  Veci::iterator new_end = std::remove( old_idx.begin(), old_idx.end(), -1);
  old_idx.resize( new_end-old_idx.begin());
  // eliminate vars from linear system
  Vecd xr(_x);
  COMISO_GMM::eliminate_csc_vars2( elim_i, elim_v, _A, xr, _rhs);

  // std::cerr << "size A: " << gmm::mat_nrows(_A) << " " << gmm::mat_ncols(_A) 
  // 	    << std::endl;
  // std::cerr << "size x  : " << xr.size() << std::endl;
  // std::cerr << "size rhs: " << _rhs.size() << std::endl;

  // final full solution
  if( gmm::mat_ncols( _A) > 0)
  {
    //    direct_solver_.update_system_gmm(_A);
    direct_solver_.calc_system_gmm(_A);
    direct_solver_.solve( xr, _rhs);
  }

  // store solution values to result vector
  for(unsigned int i=0; i<old_idx.size(); ++i)
  {
    _x[ old_idx[i] ] = xr[i];
  }
}


//-----------------------------------------------------------------------------


void 
MISolver::solve_iterative( 
    CSCMatrix& _A, 
    Vecd&      _x, 
    Vecd&      _rhs, 
    Veci&      _to_round,
    bool       _fixed_order )
{
  DEB_enter_func;
  // StopWatch
  Base::StopWatch sw;
  double time_search_next_integer = 0;

  // some statistics
  n_local_ = 0;
  n_cg_    = 0;
  n_full_  = 0;

  // reset cholmod step flag
  cholmod_step_done_ = false;

  Veci to_round(_to_round);
  // if the order is not fixed, uniquify the indices
  if( !_fixed_order)
  {
    // copy to round vector and make it unique
    std::sort(to_round.begin(), to_round.end());
    Veci::iterator last_unique;
    last_unique = std::unique(to_round.begin(), to_round.end());
    size_t r = (size_t)(last_unique - to_round.begin());
    to_round.resize( r);
  }

  // initalize old indices
  Veci old_idx(_rhs.size());
  for(unsigned int i=0; i<old_idx.size(); ++i)
    old_idx[i] = i;

  if( initial_full_solution_)
  {
    DEB_out_if( noisy_ > 2, 2,  "initial full solution\n")
    direct_solver_.calc_system_gmm(_A);
    direct_solver_.solve(_x, _rhs);

    cholmod_step_done_ = true;

    ++n_full_;
  }

  // neighbors for local optimization
  Vecui neigh_i;

  // Vector for reduced solution
  Vecd xr(_x);

  // loop until solution computed
  for(unsigned int i=0; i<to_round.size(); ++i)
  {
    if( noisy_ > 0)
    {
      DEB_out(2, "Integer DOF's left: " << to_round.size()-(i+1) << " ")
      DEB_out_if( noisy_ > 1, 1, "residuum_norm: "
        << COMISO_GMM::residuum_norm( _A, xr, _rhs) << "\n")
    }

    // index to eliminate
    unsigned int i_best = 0;

    // position in round vector
    unsigned int tr_best = 0;

    if( _fixed_order ) // if order is fixed, simply get next index from to_round
    {
      i_best = to_round[i];
    }
    else               // else search for best rounding candidate
    {
      sw.start();
      // find index yielding smallest rounding error
      double       r_best = FLT_MAX;
      for(unsigned int j=0; j<to_round.size(); ++j)
      {
        if( to_round[j] != -1)
        {
          int cur_idx = to_round[j];
          double rnd_error = fabs( ROUND(xr[cur_idx]) - xr[cur_idx]);
          if( rnd_error < r_best)
          {
            i_best  = cur_idx;
            r_best  = rnd_error;
	    tr_best = j;
          }
        }
      }
      time_search_next_integer += sw.stop();
    }

    // store rounded value
    double rnd_x = ROUND(xr[i_best]);
    _x[ old_idx[i_best] ] = rnd_x;

    // compute neighbors
    neigh_i.clear();
    Col col = gmm::mat_const_col(_A, i_best);
    ColIter it  = gmm::vect_const_begin( col);
    ColIter ite = gmm::vect_const_end  ( col);
    for(; it!=ite; ++it)
      if(it.index() != i_best)
        neigh_i.push_back(static_cast<int>(it.index()));

    // eliminate var
    COMISO_GMM::fix_var_csc_symmetric(i_best, rnd_x, _A, xr, _rhs);
    to_round[tr_best] = -1;

    // 3-stage update of solution w.r.t. roundings
    // local GS / CG / SparseCholesky
    update_solution( _A, xr, _rhs, neigh_i);
  }

  // final full solution?
  if( final_full_solution_)
  {
    DEB_out_if( noisy_ > 2, 2, "final full solution\n")

    if( gmm::mat_ncols( _A) > 0)
    {
      if(cholmod_step_done_)
	direct_solver_.update_system_gmm(_A);
      else
	direct_solver_.calc_system_gmm(_A);

      direct_solver_.solve( xr, _rhs);
      ++n_full_;
    }
  }

  // store solution values to result vector
  for(unsigned int i=0; i<old_idx.size(); ++i)
  {
    _x[ old_idx[i] ] = xr[i];
  }

  // output statistics
  DEB_out_if( stats_, 2, " *** Statistics of MiSo Solver ***"
    << "\n Number of CG    iterations  = " << n_cg_
    << "\n Number of LOCAL iterations  = " << n_local_
    << "\n Number of FULL  iterations  = " << n_full_
    << "\n Number of ROUNDING          = " << _to_round.size()
    << "\n time searching next integer = " 
    << time_search_next_integer / 1000.0 << "s\n\n")
}


//-----------------------------------------------------------------------------


void 
MISolver::update_solution( 
    CSCMatrix& _A, 
    Vecd&      _x, 
    Vecd&      _rhs, 
    Vecui&     _neigh_i )
{
  DEB_enter_func
  // set to not converged
  bool converged = false;

  // compute new solution
  if(max_local_iters_ > 0)
  {
    DEB_out_if( noisy_ > 2, 2, "use local iteration ")

    int    n_its     = max_local_iters_;
    double tolerance = max_local_error_;
    converged = siter_.gauss_seidel_local(_A, _x, _rhs, _neigh_i, n_its, tolerance);

    ++n_local_;
  }


  // conjugate gradient
  if( !converged && max_cg_iters_ > 0)
  {
    DEB_out_if( noisy_ > 2, 2, ", cg ")

    int max_cg_iters = max_cg_iters_;
    double tolerance = max_cg_error_;
    converged = siter_.conjugate_gradient(_A, _x,_rhs, max_cg_iters, tolerance);

    DEB_out_if( noisy_ > 3, 3,  "( converged " << converged << " "
		<< " iters " << max_cg_iters   << " "
                << " res_norm " << tolerance << "\n")
    ++n_cg_;
  }

  if(!converged && iter_full_solution_)
  {
    DEB_out_if( noisy_ > 2, 2, ", full ")

    if( gmm::mat_ncols( _A) > 0)
    {
      if(cholmod_step_done_)
	direct_solver_.update_system_gmm(_A);
      else
      {
	direct_solver_.calc_system_gmm(_A);
	cholmod_step_done_ = true;
      }
      direct_solver_.solve(_x,_rhs);

      ++n_full_;
    }
  }

  DEB_out_if( noisy_ > 2, 2, "\n")
}

//-----------------------------------------------------------------------------


void MISolver::solve_multiple_rounding( 
    CSCMatrix& _A, 
    Vecd&      _x, 
    Vecd&      _rhs, 
        Veci&      _to_round 
        )
{
  DEB_enter_func
  // StopWatch
  Base::StopWatch sw;
  double time_search_next_integer = 0;

  // some statistics
  n_local_ = 0;
  n_cg_    = 0;
  n_full_  = 0;

  // reset cholmod step flag
  cholmod_step_done_ = false;

  Veci to_round(_to_round);
  // copy to round vector and make it unique
  std::sort(to_round.begin(), to_round.end());
  Veci::iterator last_unique;
  last_unique = std::unique(to_round.begin(), to_round.end());
  size_t r = (size_t)(last_unique - to_round.begin());
  to_round.resize( r);

        // initialize old indices
  Veci old_idx(_rhs.size());
  for(unsigned int i=0; i<old_idx.size(); ++i)
    old_idx[i] = i;

  if( initial_full_solution_)
  {
                DEB_out_if( noisy_ > 2, 2, "initial full solution\n") 
    // TODO: we can throw more specific outcomes in the body of the fucntions below
    COMISO_THROW_if(!direct_solver_.calc_system_gmm(_A), 
      UNSPECIFIED_EIGEN_FAILURE);
    COMISO_THROW_if(!direct_solver_.solve(_x, _rhs), 
      UNSPECIFIED_EIGEN_FAILURE);

    cholmod_step_done_ = true;

    ++n_full_;
  }

  // neighbors for local optimization
  Vecui neigh_i;

  // Vector for reduced solution
  Vecd xr(_x);

  // loop until solution computed
  for(unsigned int i=0; i<to_round.size(); ++i)
  {
    DEB_out_if(noisy_ > 0, 1, "Integer DOF's left: " << to_round.size()-(i+1) << " ")
        DEB_out_if(noisy_ > 1, 1, "residuum_norm: " << COMISO_GMM::residuum_norm( _A, xr, _rhs) << "\n")

    // position in round vector
    std::vector<int> tr_best;

    sw.start();

    RoundingSet rset;
    rset.set_threshold(multiple_rounding_threshold_);

    // find index yielding smallest rounding error
    for(unsigned int j=0; j<to_round.size(); ++j)
    {
      if( to_round[j] != -1)
      {
	int cur_idx = to_round[j];
	double rnd_error = fabs( ROUND(xr[cur_idx]) - xr[cur_idx]);

	rset.add(j, rnd_error);
      }
    }

    rset.get_ids( tr_best);

    time_search_next_integer += sw.stop();
  
    // nothing more to do?
    if( tr_best.empty() )
      break;

                DEB_out_if( noisy_ > 5, 5, 
      "round " << tr_best.size() << " variables simultaneously\n")

    // clear neigh for local update
    neigh_i.clear();

    for(unsigned int j = 0; j<tr_best.size(); ++j)
    {
      int i_cur = to_round[tr_best[j]];

      // store rounded value
      double rnd_x = ROUND(xr[i_cur]);
      _x[ old_idx[i_cur] ] = rnd_x;

      // compute neighbors
      Col col = gmm::mat_const_col(_A, i_cur);
      ColIter it  = gmm::vect_const_begin( col);
      ColIter ite = gmm::vect_const_end  ( col);
      for(; it!=ite; ++it)
	if(it.index() != (unsigned int)i_cur)
	  neigh_i.push_back(static_cast<int>(it.index()));

      // eliminate var
      COMISO_GMM::fix_var_csc_symmetric( i_cur, rnd_x, _A, xr, _rhs);
      to_round[tr_best[j]] = -1;
    }

    // 3-stage update of solution w.r.t. roundings
    // local GS / CG / SparseCholesky
    update_solution( _A, xr, _rhs, neigh_i);
  }

  // final full solution?
  if( final_full_solution_)
  {
                DEB_out_if( noisy_ > 2, 2, "final full solution\n")

    if( gmm::mat_ncols( _A) > 0)
    {
      if(cholmod_step_done_)
	direct_solver_.update_system_gmm(_A);
      else
	direct_solver_.calc_system_gmm(_A);

      direct_solver_.solve( xr, _rhs);
      ++n_full_;
    }
  }

  // store solution values to result vector
  for(unsigned int i=0; i<old_idx.size(); ++i)
  {
    _x[ old_idx[i] ] = xr[i];
  }

  // output statistics
        DEB_out_if( stats_, 2, " *** Statistics of MiSo Solver ***"
                << "\n Number of CG    iterations  = " << n_cg_ 
        << "\n Number of LOCAL iterations  = " << n_local_ 
                << "\n Number of FULL  iterations  = " << n_full_ 
                << "\n Number of ROUNDING          = " << _to_round.size()
                << "\n time searching next integer = " 
        << time_search_next_integer / 1000.0 <<"s\n\n")
}


//-----------------------------------------------------------------------------


void
MISolver::solve_gurobi(
    CSCMatrix& _A,
    Vecd&      _x,
    Vecd&      _rhs,
    Veci&      _to_round)
{
  DEB_enter_func
#if COMISO_GUROBI_AVAILABLE

  // get round-indices in set
  std::set<int> to_round;
  for(unsigned int i=0; i<_to_round.size();++i)
    to_round.insert(_to_round[i]);

  try {
    GRBEnv env = GRBEnv();

    GRBModel model = GRBModel(env);

    // set time limite
    model.getEnv().set(GRB_DoubleParam_TimeLimit, gurobi_max_time_);

    unsigned int n = _rhs.size();

    // 1. allocate variables
    std::vector<GRBVar> vars;
    for( unsigned int i=0; i<n; ++i)
      if( to_round.count(i))
        vars.push_back( model.addVar(-GRB_INFINITY, GRB_INFINITY, 0.0, GRB_INTEGER));
      else
        vars.push_back( model.addVar(-GRB_INFINITY, GRB_INFINITY, 0.0, GRB_CONTINUOUS));

    // Integrate new variables
    model.update();

    // 2. setup_energy

    // build objective function from linear system E = x^tAx - 2x^t*rhs
    GRBQuadExpr objective;

    for(unsigned int i=0; i<_A.nc; ++i)
      for(unsigned int j=_A.jc[i]; j<_A.jc[i+1]; ++j)
      {
        objective += _A.pr[j]*vars[_A.ir[j]]*vars[i];
      }
    for(unsigned int i=0; i<n; ++i)
      objective -= 2*_rhs[i]*vars[i];

//    _A.jc[c+1]
//    _A.pr[write]
//    _A.ir[write]
//    _A.nc
//    _A.nr

    // minimize
    model.set(GRB_IntAttr_ModelSense, 1);
    model.setObjective(objective);

    // 4. solve
    model.optimize();

    // 5. store result
    _x.resize(n);
    for(unsigned int i=0; i<n; ++i)
      _x[i] = vars[i].get(GRB_DoubleAttr_X);

    DEB_out(2, "GUROBI objective: " << model.get(GRB_DoubleAttr_ObjVal) << "\n")

  }
  catch(GRBException& e)
  {
    PROGRESS_RESUME_ABORT; // resume a processed abort request
    DEB_warning(2, "Error code = " << e.getErrorCode() << "[" << e.getMessage() << "]\n")
  }
  catch(...)
  {
    PROGRESS_RESUME_ABORT; // resume a processed abort request
    DEB_warning(1, "Exception during optimization")
  }

#else
  DEB_out(1,"GUROBI solver is not available, please install it...\n")
#endif
}


//----------------------------------------------------------------------------


void 
MISolver::
show_options_dialog()
{
  DEB_enter_func;
#if(COMISO_QT4_AVAILABLE)
  MISolverDialog* pd = new MISolverDialog(*this);
  pd->exec();
#else
  DEB_warning(1, "Qt not available to show solver dialog!!!")
#endif
}


// end namespace COMISO
}// ----------------------
