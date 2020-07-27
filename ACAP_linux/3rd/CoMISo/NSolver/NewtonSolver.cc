//=============================================================================
//
//  CLASS NewtonSolver - IMPLEMENTATION
//
//=============================================================================

//== INCLUDES =================================================================

#include "NewtonSolver.hh"
#include <CoMISo/Solver/CholmodSolver.hh>
#include <Base/Debug/DebTime.hh>
//== NAMESPACES ===============================================================

namespace COMISO {

//== IMPLEMENTATION ========================================================== 


// solve
int
NewtonSolver::
solve(NProblemGmmInterface* _problem)
{
  DEB_enter_func;
#if COMISO_SUITESPARSE_AVAILABLE  
  
  // get problem size
  int n = _problem->n_unknowns();

  // hesse matrix
  NProblemGmmInterface::SMatrixNP H;
  // gradient
  std::vector<double> x(n), x_new(n), dx(n), g(n);

  // get initial x, initial grad and initial f
  _problem->initial_x(P(x));
  double f = _problem->eval_f(P(x));

  double reg = 1e-3;
  COMISO::CholmodSolver chol;

  for(int i=0; i<max_iters_; ++i)
  {
    _problem->eval_gradient(P(x), P(g));
    // check for convergence
    if( gmm::vect_norm2(g) < eps_)
    {
      DEB_line(2, "Newton Solver converged after " << i << " iterations");
      _problem->store_result(P(x));
      return true;
    }

    // get current hessian
    _problem->eval_hessian(P(x), H);

    // regularize
    double reg_comp = reg*gmm::mat_trace(H)/double(n);
    for(int j=0; j<n; ++j)
      H(j,j) += reg_comp;

    // solve linear system
    bool factorization_ok = false;
    if(constant_hessian_structure_ && i != 0)
      factorization_ok = chol.update_system_gmm(H);
    else
      factorization_ok = chol.calc_system_gmm(H);

    bool improvement = false;
    if(factorization_ok)
      if(chol.solve( dx, g))
      {
        gmm::add(x, gmm::scaled(dx,-1.0),x_new);
        double f_new = _problem->eval_f(P(x_new));

        if( f_new < f)
        {
          // swap x and x_new (and f and f_new)
          x_new.swap(x);
          f = f_new;
          improvement = true;

          DEB_line(2, "energy improved to " << f);
        }
      }

    // adapt regularization
    if(improvement)
    {
      if(reg > 1e-9)
        reg *= 0.1;
    }
    else
    {
      if(reg < 1e4)
        reg *= 10.0;
      else
      {
        _problem->store_result(P(x));
        DEB_line(2, "Newton solver reached max regularization but did not "
          "converge");
        return false;
      }
    }
  }
  _problem->store_result(P(x));
  DEB_line(2, "Newton Solver did not converge!!! after iterations.");
  return false;

#else
  DEB_warning(1,"NewtonSolver requires not-available CholmodSolver");
  return false;
#endif	    
}


//-----------------------------------------------------------------------------


int NewtonSolver::solve(NProblemInterface* _problem, const SMatrixD& _A, 
  const VectorD& _b)
{
  DEB_time_func_def;

  const double KKT_res_eps = 1e-6;
  const int    max_KKT_regularization_iters = 40;
        double regularize_constraints_limit = 1e-6;
  const double max_allowed_constraint_violation2 = 1e-12;

  // number of unknowns
  size_t n = _problem->n_unknowns();
  // number of constraints
  size_t m = _b.size();

  DEB_line(2, "optimize via Newton with " << n << " unknowns and " << m << 
    " linear constraints");

  // initialize vectors of unknowns
  VectorD x(n);
  _problem->initial_x(x.data());

  double initial_constraint_violation2 = (_A*x-_b).squaredNorm();

  // storage of update vector dx and rhs of KKT system
  VectorD dx(n+m), rhs(n+m), g(n);
  rhs.setZero();

  // resize temp vector for line search (and set to x1 to approx Hessian correctly if problem is non-quadratic!)
  x_ls_ = x;

  // indicate that system matrix is symmetric
  lu_solver_.isSymmetric(true);

  // start with no regularization
  double regularize_hessian(0.0);
  double regularize_constraints(0.0);
  int iter=0;
  bool first_factorization = true;
  while( iter < max_iters_)
  {
    double kkt_res2(0.0);
    double constraint_res2(0.0);
    int    reg_iters(0);
    do
    {
      // get Newton search direction by solving LSE
      bool fact_ok = factorize(_problem, _A, _b, x, regularize_hessian, regularize_constraints, first_factorization);
      first_factorization = false;

      if(fact_ok)
      {
        // get rhs
        _problem->eval_gradient(x.data(), g.data());
        rhs.head(n) = -g;
        rhs.tail(m) = _b - _A*x;

        // solve KKT system
        solve_kkt_system(rhs, dx);

        // check numerical stability of KKT system and regularize if necessary
        kkt_res2 = (KKT_*dx-rhs).squaredNorm();
        constraint_res2 = (_A*dx.head(n)-rhs.tail(m)).squaredNorm();
      }

      if(!fact_ok || kkt_res2 > KKT_res_eps || constraint_res2 > max_allowed_constraint_violation2)
      {
        DEB_warning(2, "Numerical issues in KKT system"); 
        // alternate hessian and constraints regularization
        if(reg_iters % 2 == 0 || regularize_constraints >= regularize_constraints_limit)
        {
          DEB_line(2, "residual ^ 2 " << kkt_res2 << "->regularize hessian");
          if(regularize_hessian == 0.0)
            regularize_hessian = 1e-6;
          else
            regularize_hessian *= 2.0;
        }
        else
        {
          DEB_line(2, "residual^2 " << kkt_res2 << " -> regularize constraints");
          if(regularize_constraints == 0.0)
            regularize_constraints = 1e-8;
          else
            regularize_constraints *= 2.0;
        }
      }
      ++reg_iters;
    }
    while( (kkt_res2 > KKT_res_eps || constraint_res2 > max_allowed_constraint_violation2) && reg_iters < max_KKT_regularization_iters);

    // no valid step could be found?
    if(kkt_res2 > KKT_res_eps || constraint_res2 > max_allowed_constraint_violation2 || reg_iters >= max_KKT_regularization_iters)
    {
      DEB_error("numerical issues in KKT system could not be resolved "
        "-> terminating NewtonSolver with current solution");
      _problem->store_result(x.data());
      return 0;
    }

    // get maximal reasonable step
    double t_max  = std::min(1.0, 
      0.5 * _problem->max_feasible_step(x.data(), dx.data()));

    // perform line-search
    double newton_decrement(0.0);
    double fx(0.0);
    double t = backtracking_line_search(_problem, x, g, dx, newton_decrement, fx, t_max);

    // perform update
    x += dx.head(n)*t;

    double constraint_violation2 = (_A*x-_b).squaredNorm();

    if(constraint_violation2 > 2*initial_constraint_violation2 && constraint_violation2 > max_allowed_constraint_violation2)
    {
      DEB_warning(2, "Numerical issues in KKT system lead to "
        "constraint violation -> recovery phase");
      // restore old solution
      x -= dx.head(n)*t;

      regularize_constraints *= 0.5;
      regularize_constraints_limit = regularize_constraints;
    }


    DEB_line(2, "iter: " << iter
      << ", f(x) = " << fx << ", t = " << t << " (tmax=" << t_max << ")"
      << (t < t_max ? " _clamped_" : "")
      << ", eps = [Newton decrement] = " << newton_decrement
      << ", constraint violation prior = " << rhs.tail(m).norm()
      << ", after = " << (_b - _A*x).norm()
      << ", KKT residual^2 = " << kkt_res2);

    // converged?
    if(newton_decrement < eps_ || std::abs(t) < eps_ls_)
      break;

    ++iter;
  }

  // store result
  _problem->store_result(x.data());

  // return success
  return 1;
}


//-----------------------------------------------------------------------------


bool NewtonSolver::factorize(NProblemInterface* _problem,
  const SMatrixD& _A, const VectorD& _b, const VectorD& _x, double& _regularize_hessian, double& _regularize_constraints,
  const bool _first_factorization)
{
  DEB_enter_func;
  const int n  = _problem->n_unknowns();
  const int m  = _A.rows();
  const int nf = n+m;

  // get hessian of quadratic problem
  SMatrixD H(n,n);
  _problem->eval_hessian(_x.data(), H);

  // set up KKT matrix
  // create sparse matrix
  std::vector< Triplet > trips;
  trips.reserve(H.nonZeros() + 2*_A.nonZeros());

  // add elements of H
  for (int k=0; k<H.outerSize(); ++k)
    for (SMatrixD::InnerIterator it(H,k); it; ++it)
      trips.push_back(Triplet(it.row(),it.col(),it.value()));

  // add elements of _A
  for (int k=0; k<_A.outerSize(); ++k)
    for (SMatrixD::InnerIterator it(_A,k); it; ++it)
    {
      // insert _A block below
      trips.push_back(Triplet(it.row()+n,it.col(),it.value()));

      // insert _A^T block right
      trips.push_back(Triplet(it.col(),it.row()+n,it.value()));
    }

  // regularize constraints
//  if(_regularize_constraints != 0.0)
    for( int i=0; i<m; ++i)
      trips.push_back(Triplet(n+i,n+i,_regularize_constraints));

  // regularize Hessian
//  if(_regularize_hessian != 0.0)
  {
    double ad(0.0);
    for( int i=0; i<n; ++i)
      ad += H.coeffRef(i,i);
    ad *= _regularize_hessian/double(n);
    for( int i=0; i<n; ++i)
      trips.push_back(Triplet(i,i,ad));
  }

  // create KKT matrix
  KKT_.resize(nf,nf);
  KKT_.setFromTriplets(trips.begin(), trips.end());

  // compute LU factorization
  if(_first_factorization)
    analyze_pattern(KKT_);

  return numerical_factorization(KKT_);
}


//-----------------------------------------------------------------------------


double NewtonSolver::backtracking_line_search(NProblemInterface* _problem, 
  VectorD& _x, VectorD& _g, VectorD& _dx, double& _newton_decrement, 
  double& _fx, const double _t_start)
{
  DEB_enter_func;
  size_t n = _x.size();

  // pre-compute objective
  double fx = _problem->eval_f(_x.data());

  // pre-compute dot product
  double gtdx = _g.transpose()*_dx.head(n);
  _newton_decrement = std::abs(gtdx);

  // current step size
  double t = _t_start;

  // backtracking (stable in case of NAN and with max 100 iterations)
  for(int i=0; i<100; ++i)
  {
    // current update
    x_ls_ = _x + _dx.head(n)*t;
    double fx_ls = _problem->eval_f(x_ls_.data());

    if( fx_ls <= fx + alpha_ls_*t*gtdx )
    {
      _fx = fx_ls;
      return t;
    }
    else
      t *= beta_ls_;
  }

  DEB_warning(1, "line search could not find a valid step within 100 "
    "iterations");
  _fx = fx;
  return 0.0;
}


//-----------------------------------------------------------------------------


void NewtonSolver::analyze_pattern(SMatrixD& _KKT)
{
  DEB_enter_func;
  switch(solver_type_)
  {
    case LS_EigenLU:      lu_solver_.analyzePattern(_KKT); break;
#if COMISO_SUITESPARSE_AVAILABLE
    case LS_Umfpack: umfpack_solver_.analyzePattern(_KKT); break;
#endif
    default: DEB_warning(1, "selected linear solver not availble");
  }
}


//-----------------------------------------------------------------------------


bool NewtonSolver::numerical_factorization(SMatrixD& _KKT)
{
  DEB_enter_func;
  switch(solver_type_)
  {
    case LS_EigenLU:      
      lu_solver_.factorize(_KKT); 
      return (lu_solver_.info() == Eigen::Success);
#if COMISO_SUITESPARSE_AVAILABLE
    case LS_Umfpack: 
      umfpack_solver_.factorize(_KKT); 
      return (umfpack_solver_.info() == Eigen::Success);
#endif
    default: 
      DEB_warning(1, "selected linear solver not availble!"); 
      return false;
  }
}


//-----------------------------------------------------------------------------


void NewtonSolver::solve_kkt_system(const VectorD& _rhs, VectorD& _dx)
{
  DEB_enter_func;
  switch(solver_type_)
  {
    case LS_EigenLU: _dx =      lu_solver_.solve(_rhs); break;
#if COMISO_SUITESPARSE_AVAILABLE
    case LS_Umfpack: _dx = umfpack_solver_.solve(_rhs); break;
#endif
    default: DEB_warning(1, "selected linear solver not availble"); break;
  }
}

//=============================================================================
} // namespace COMISO
//=============================================================================
