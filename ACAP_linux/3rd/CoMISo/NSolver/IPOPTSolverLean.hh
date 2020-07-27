//=============================================================================
//
//  CLASS IPOPTSolverLean
//
//=============================================================================


#ifndef COMISO_IPOPTLEANSOLVER_HH
#define COMISO_IPOPTLEANSOLVER_HH


//== COMPILE-TIME PACKAGE REQUIREMENTS ========================================
#include <CoMISo/Config/config.hh>
#if COMISO_IPOPT_AVAILABLE

//== INCLUDES =================================================================

#include <CoMISo/Config/CoMISoDefines.hh>
#include <vector>
#include <cstddef>


//== NAMESPACES ===============================================================

namespace COMISO {

//== FORWARDDECLARATIONS ======================================================
class NProblemGmmInterface; // deprecated
class NProblemInterface;
class NConstraintInterface;

//== CLASS DEFINITION =========================================================


/** \class NewtonSolver NewtonSolver.hh <ACG/.../NewtonSolver.hh>

    Brief Description.
  
    A more elaborate description follows.
*/
class COMISODLLEXPORT IPOPTSolverLean
{
public:
  /// Default constructor
  IPOPTSolverLean();
 
  /// Destructor
  ~IPOPTSolverLean();

  // *********** OPTIONS **************//

  /*!
  Set the maximum number of iterations
  */
  void set_max_iterations(const int _max_iterations);
  int max_iterations() const;

  /*! 
  Set the threshold on the lazy inequality constraint to decide if we are near 
  the  constraint boundary.
  */
  void set_almost_infeasible_threshold(const double _alm_infsb_thrsh);
  double almost_infeasible_threshold() const;

  /*!
  Set the max number of incremental lazy constraint iterations before switching 
  to the fully constrained problem.
  \note The default value is 5.
  */
  void set_incremental_lazy_constraint_max_iteration_number(const int 
    _incr_lazy_cnstr_max_iter_nmbr);
  int incremental_lazy_constraint_max_iteration_number() const;

  /*
  Turn on/off solving the fully constraint problem after exhausting the 
  incremental lazy constraint iterations. 

  \note The default value of this is true.
  */
  void set_enable_all_lazy_contraints(const bool _enbl_all_lzy_cnstr);
  bool enable_all_lazy_contraints() const;

  // ********** SOLVE **************** //
  
  //! \throws Outcome
  void solve(NProblemInterface* _problem, 
    const std::vector<NConstraintInterface*>& _constraints);

  //! Same as above with additional lazy constraints that are only added iteratively to the problem if not satisfied
  //! \throws Outcome
  void solve(NProblemInterface* _problem,
    const std::vector<NConstraintInterface*>& _constraints,
    const std::vector<NConstraintInterface*>& _lazy_constraints);

  // for convenience, if no constraints are given
  //! \throws Outcome
  void solve(NProblemInterface* _problem);

  // deprecated interface for backwards compatibility
  //! \throws Outcome
  void solve(NProblemGmmInterface* _problem, 
    std::vector<NConstraintInterface*>& _constraints);

  //! Get the computed solution energy 
  double energy();

private:
  class Impl;
  Impl* impl_;

  // inhibit copy
  IPOPTSolverLean(const IPOPTSolverLean&);
  IPOPTSolverLean& operator=(const IPOPTSolverLean&);
};


//=============================================================================
} // namespace COMISO

//=============================================================================
#endif // COMISO_IPOPT_AVAILABLE
//=============================================================================
#endif // ACG_IPOPTSOLVER_HH defined
//=============================================================================

