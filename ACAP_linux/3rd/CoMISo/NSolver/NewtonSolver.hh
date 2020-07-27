//=============================================================================
//
//  CLASS NewtonSolver
//
//=============================================================================


#ifndef COMISO_NEWTONSOLVER_HH
#define COMISO_NEWTONSOLVER_HH

//== COMPILE-TIME PACKAGE REQUIREMENTS ========================================
#include <CoMISo/Config/config.hh>

//== INCLUDES =================================================================

#include <CoMISo/Config/CoMISoDefines.hh>
#include <CoMISo/Utils/StopWatch.hh>
#include "NProblemInterface.hh"
#include "NProblemGmmInterface.hh"

//#include <Base/Debug/DebTime.hh>

#if COMISO_SUITESPARSE_AVAILABLE
  #include <Eigen/UmfPackSupport>
  #include <Eigen/CholmodSupport>
#endif

// ToDo: why is Metis not working yet?
//#if COMISO_METIS_AVAILABLE
//  #include <Eigen/MetisSupport>
//#endif

//== FORWARDDECLARATIONS ======================================================

//== NAMESPACES ===============================================================

namespace COMISO {

//== CLASS DEFINITION =========================================================

	      

/** \class NewtonSolver NewtonSolver.hh <ACG/.../NewtonSolver.hh>

    Brief Description.
  
    A more elaborate description follows.
*/
class COMISODLLEXPORT NewtonSolver
{
public:

  enum LinearSolver {LS_EigenLU, LS_Umfpack, LS_MUMPS};

  typedef Eigen::VectorXd             VectorD;
  typedef Eigen::SparseMatrix<double> SMatrixD;
  typedef Eigen::Triplet<double>      Triplet;

  /// Default constructor
  NewtonSolver(const double _eps = 1e-6, const double _eps_line_search = 1e-9, const int _max_iters = 200, const double _alpha_ls=0.2, const double _beta_ls = 0.6)
    : eps_(_eps), eps_ls_(_eps_line_search), max_iters_(_max_iters), alpha_ls_(_alpha_ls), beta_ls_(_beta_ls), solver_type_(LS_EigenLU), constant_hessian_structure_(false)
  {
//#if COMISO_SUITESPARSE_AVAILABLE
//    solver_type_ = LS_Umfpack;
//#endif
  }

  // solve without linear constraints
  int solve(NProblemInterface* _problem)
  {
    SMatrixD A(0,_problem->n_unknowns());
    VectorD b(VectorD::Index(0));
    return solve(_problem, A, b);
  }

  // solve with linear constraints
  // Warning: so far only feasible starting points with (_A*_problem->initial_x() == b) are supported!
  // Extension to infeasible starting points is planned
  int solve(NProblemInterface* _problem, const SMatrixD& _A, const VectorD& _b);

  // select internal linear solver
  void set_linearsolver(LinearSolver _st)
  {
    solver_type_ = _st;
  }

protected:

  bool factorize(NProblemInterface* _problem, const SMatrixD& _A,
    const VectorD& _b, const VectorD& _x, double& _regularize_hessian,
    double& _regularize_constraints, const bool _first_factorization);

  double backtracking_line_search(NProblemInterface* _problem, VectorD& _x, 
    VectorD& _g, VectorD& _dx, double& _newton_decrement, double& _fx, 
    const double _t_start = 1.0);

  void analyze_pattern(SMatrixD& _KKT);

  bool numerical_factorization(SMatrixD& _KKT);

  void solve_kkt_system(const VectorD& _rhs, VectorD& _dx);

  // deprecated function!
  // solve
  int solve(NProblemGmmInterface* _problem);

  // deprecated function!
  // solve specifying parameters
  int solve(NProblemGmmInterface* _problem, int _max_iter, double _eps)
  {
    max_iters_ = _max_iter;
    eps_ = _eps;
    return solve(_problem);
  }

  // deprecated function!
  bool& constant_hessian_structure() { return constant_hessian_structure_; }

protected:
  double* P(std::vector<double>& _v)
  {
    if( !_v.empty())
      return ((double*)&_v[0]);
    else
      return 0;
  }

private:

  double eps_;
  double eps_ls_;
  int    max_iters_;
  double alpha_ls_;
  double beta_ls_;

  VectorD x_ls_;

  LinearSolver solver_type_;

  // cache KKT Matrix
  SMatrixD KKT_;

  // Sparse LU decomposition
  Eigen::SparseLU<SMatrixD> lu_solver_;

#if COMISO_SUITESPARSE_AVAILABLE
  Eigen::UmfPackLU<SMatrixD> umfpack_solver_;
#endif

  // deprecated
  bool   constant_hessian_structure_;
};


//=============================================================================
} // namespace COMISO
//=============================================================================
#endif // ACG_NEWTONSOLVER_HH defined
//=============================================================================

