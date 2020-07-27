//=============================================================================
//
//  CLASS AcceleratedQuadraticProxy
//
//=============================================================================


#ifndef COMISO_ACCELERATEDQUADRATICPROXY_HH
#define COMISO_ACCELERATEDQUADRATICPROXY_HH

//== COMPILE-TIME PACKAGE REQUIREMENTS ========================================
#include <CoMISo/Config/config.hh>

//== INCLUDES =================================================================

#include <CoMISo/Config/CoMISoDefines.hh>
#include <CoMISo/Utils/StopWatch.hh>
#include "NProblemInterface.hh"

//== FORWARDDECLARATIONS ======================================================

//== NAMESPACES ===============================================================

namespace COMISO {

//== CLASS DEFINITION =========================================================

	      

/** \class AcceleratedQuadraticProxy AcceleratedQuadraticProxy.hh <CoMISo/NSolver/AcceleratedQuadraticProxy.hh>

    Brief Description.
  
    A more elaborate description follows.
*/
class AcceleratedQuadraticProxy
{
public:

  typedef Eigen::VectorXd             VectorD;
  typedef Eigen::SparseMatrix<double> SMatrixD;
  typedef Eigen::Triplet<double>      Triplet;
   
  /// Default constructor
  AcceleratedQuadraticProxy(const double _eps = 1e-6, const int _max_iters = 1000, const double _accelerate = 100.0, const double _alpha_ls=0.2, const double _beta_ls = 0.6)
    : eps_(_eps), max_iters_(_max_iters), accelerate_(_accelerate), alpha_ls_(_alpha_ls), beta_ls_(_beta_ls) {}
 
  // solve without linear constraints
  int solve(NProblemInterface* _quadratic_problem, NProblemInterface* _nonlinear_problem, bool _update_factorization = true)
  {
    SMatrixD A(0,_quadratic_problem->n_unknowns());
    VectorD b(VectorD::Index(0));
    return solve(_quadratic_problem, _nonlinear_problem, A, b, _update_factorization);
  }

  // solve with linear constraints
  int solve(NProblemInterface* _quadratic_problem, NProblemInterface* _nonlinear_problem, const SMatrixD& _A, const VectorD& _b, bool _update_factorization = true)
  {
    // time solution procedure
    COMISO::StopWatch sw; sw.start();

    // number of unknowns
    size_t n = _quadratic_problem->n_unknowns();
    // number of constraints
    size_t m = _b.size();

    std::cerr << "optmize via AQP with " << n << " unknowns and " << m << " linear constraints" << std::endl;

    // initialize vectors of unknowns (cache last two steps)
    VectorD x1(n);
    _quadratic_problem->initial_x(x1.data());
    VectorD x2 = x1;

    // storage of acceleration vector and update vector dx and rhs of KKT system
    VectorD dx(n+m), rhs(n+m), g(n);
    rhs.setZero();

    // resize temp vector for line search (and set to x1 to approx Hessian correctly if _quadratic problem is non-quadratic!)
    x_ls_ = x1;
    // resize temp gradient vector for line search
    g_.resize(n);

    const double theta = (1.0-std::sqrt(1.0/accelerate_))/(1.0+std::sqrt(1.0/accelerate_));

    // pre-factorize linear system
    if(_update_factorization)
      pre_factorize(_quadratic_problem, _A, _b);

    int iter=0;
    while( iter < max_iters_)
    {
      dx.head(n) = x1-x2;
      double t_max  = std::min(theta, 0.5*_nonlinear_problem->max_feasible_step(x1.data(), dx.data()));

      // accelerate and update x1 and x2 (x1 will be accelerated point and x2 the previous x1)
      x2 = x1 + dx.head(n)*t_max;
      x2.swap(x1);

      // solve KKT
      _quadratic_problem->eval_gradient(x1.data(), rhs.data());
      _nonlinear_problem->eval_gradient(x1.data(), g.data());
      // get gradient
      g += rhs.head(n);
      rhs.head(n) = -g;
      dx = lu_solver_.solve(rhs);

      t_max  = std::min(1.0, 0.5*_nonlinear_problem->max_feasible_step(x1.data(), dx.data()));
      double rel_df(0.0);
      double t = backtracking_line_search(_quadratic_problem, _nonlinear_problem, x1, g, dx, rel_df, t_max);

      x1 += dx.head(n)*t;

      std::cerr << "iter: " << iter << " eps = [f(x_old)-f(x_new)]/f(x_old) = " << rel_df << std::endl;

      // converged?
      if(rel_df < eps_)
        break;

      ++iter;
    }

    // store result
    _quadratic_problem->store_result(x1.data());

    double solution_time = sw.stop();
    std::cerr << "Accelerated Quadratic Proxy finished in " << solution_time/1000.0 << "s" << std::endl;

    // return success
    return 1;
  }

protected:

  void pre_factorize(NProblemInterface* _quadratic_problem, const SMatrixD& _A, const VectorD& _b)
  {
    const int n  = _quadratic_problem->n_unknowns();
    const int m  = _A.rows();
    const int nf = n+m;

    // get hessian of quadratic problem
    SMatrixD H(n,n);
    _quadratic_problem->eval_hessian(x_ls_.data(), H);

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

    // create KKT matrix
    SMatrixD KKT(nf,nf);
    KKT.setFromTriplets(trips.begin(), trips.end());

    // compute LU factorization
    lu_solver_.compute(KKT);

    if(lu_solver_.info() != Eigen::Success)
    {
//      DEB_line(2, "Eigen::SparseLU reported problem: " << lu_solver_.lastErrorMessage());
//      DEB_line(2, "-> re-try with regularized constraints...");
      std::cerr << "Eigen::SparseLU reported problem while factoring KKT system: " << lu_solver_.lastErrorMessage() << std::endl;

      for( int i=0; i<m; ++i)
        trips.push_back(Triplet(n+i,n+i,1e-9));

      // create KKT matrix
      KKT.setFromTriplets(trips.begin(), trips.end());

      // compute LU factorization
      lu_solver_.compute(KKT);

//      IGM_THROW_if(lu_solver_.info() != Eigen::Success, QGP_BOUNDED_DISTORTION_FAILURE);
    }
  }

  double backtracking_line_search(NProblemInterface* _quadratic_problem, NProblemInterface* _nonlinear_problem, VectorD& _x, VectorD& _g, VectorD& _dx, double& _rel_df, double _t_start = 1.0)
  {
    size_t n = _x.size();

    // pre-compute objective
    double fx = _quadratic_problem->eval_f(_x.data()) + _nonlinear_problem->eval_f(_x.data());

    // pre-compute dot product
    double gtdx = _g.transpose()*_dx.head(n);

    std::cerr << "Newton decrement: " << gtdx << std::endl;

    // current step size
    double t = _t_start;

    // backtracking (stable in case of NAN and with max 100 iterations)
    for(int i=0; i<100; ++i)
    {
      // current update
      x_ls_ = _x + _dx.head(n)*t;
      double fx_ls = _quadratic_problem->eval_f(x_ls_.data()) + _nonlinear_problem->eval_f(x_ls_.data());

      if( fx_ls <= fx + alpha_ls_*t*gtdx )
      {
        _rel_df = std::abs(1.0-fx_ls/fx);

        std::cerr << "LS improved objective function " << fx << " -> " << fx_ls << std::endl;

        return t;
      }
      else
        t *= beta_ls_;
    }

    std::cerr << "Warning: line search could not find a valid step within 100 iterations..." << std::endl;
    _rel_df = 0.0;
    return 0.0;
  }

private:
  double eps_;
  int    max_iters_;
  double accelerate_;
  double alpha_ls_;
  double beta_ls_;

  VectorD x_ls_;
  VectorD g_;

  // Sparse LU decomposition
  Eigen::SparseLU<SMatrixD> lu_solver_;
};


//=============================================================================
} // namespace COMISO
//=============================================================================
#endif // ACG_NEWTONSOLVER_HH defined
//=============================================================================

