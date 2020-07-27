//=============================================================================
//
//  CLASS CoonstraintTools
//
//=============================================================================


#ifndef COMISO_CONSTRAINTTOOLS_HH
#define COMISO_CONSTRAINTTOOLS_HH


//== COMPILE-TIME PACKAGE REQUIREMENTS ========================================
#include <CoMISo/Config/config.hh>
#if COMISO_EIGEN3_AVAILABLE

//== INCLUDES =================================================================

#include <stdio.h>
#include <iostream>
#include <vector>

#include <gmm/gmm.h>

#include <CoMISo/Config/CoMISoDefines.hh>
#include <CoMISo/NSolver/NConstraintInterface.hh>

//== FORWARDDECLARATIONS ======================================================

//== NAMESPACES ===============================================================

namespace COMISO {

//== CLASS DEFINITION =========================================================

	      

/** \class ConstraintTools ConstraintTools.hh <CoMISo/NSolver/ConstraintTools.hh>
  
    A more elaborate description follows.
*/
class COMISODLLEXPORT ConstraintTools
{
public:
  
  // gmm types
  typedef gmm::wsvector<double>         SVectorGMM;
  typedef gmm::row_matrix< SVectorGMM > RMatrixGMM;
  typedef gmm::col_matrix< SVectorGMM > CMatrixGMM;

  // remove all linear dependent linear equality constraints. the remaining constraints are a subset of the original ones
  // nonlinear or equality constraints are preserved.
  static void remove_dependent_linear_constraints(std::vector<NConstraintInterface*>& _constraints, const double _eps = 1e-8);

  // same as above but assumes already that all constraints are linear equality constraints
  static void remove_dependent_linear_constraints_only_linear_equality(std::vector<NConstraintInterface*>& _constraints, const double _eps = 1e-8);

private:

  static gmm::size_type find_max_abs_coeff(SVectorGMM& _v);

  static void add_row_simultaneously( gmm::size_type _row_i,
                                      double      _coeff,
                                      SVectorGMM& _row,
                                      RMatrixGMM& _rmat,
                                      CMatrixGMM& _cmat,
                                      const double _eps );


};


//=============================================================================
} // namespace COMISO
//=============================================================================
#endif // COMISO_EIGEN3_AVAILABLE
//=============================================================================
#endif // COMISO_CONSTRAINTTOOLS_HH defined
//=============================================================================

