//=============================================================================
//
//  CLASS CombinedProblem
//
//=============================================================================


#ifndef COMISO_COMBINEDPROBLEM_HH
#define COMISO_COMBINEDPROBLEM_HH


//== COMPILE-TIME PACKAGE REQUIREMENTS ========================================
#include <CoMISo/Config/config.hh>
#if COMISO_EIGEN3_AVAILABLE

//== INCLUDES =================================================================

#include <stdio.h>
#include <iostream>
#include <vector>

#include <CoMISo/Config/CoMISoDefines.hh>
#include <CoMISo/NSolver/NProblemInterface.hh>

//== FORWARDDECLARATIONS ======================================================

//== NAMESPACES ===============================================================

namespace COMISO {

//== CLASS DEFINITION =========================================================

	      

/** \class CombinedProblem CombinedProblem.hh <CoMISo/NSolver/CombinedProblem.hh>

    Create Problem of the type f(x) = _c1*_p1(x) + _c2*_p2(x), which is the linear combination
    of two other problems (specified in the constructor).

    Note: the initial_x is taken from _p1! And the result is stored for both _p1 and _p2
  
    A more elaborate description follows.
*/
class COMISODLLEXPORT CombinedProblem : public NProblemInterface
{
public:
  
  /// Default constructor
  CombinedProblem (NProblemInterface* _p1, NProblemInterface* _p2, const double _c1 = 1.0, const double _c2 = 1.0);
 
  // problem definition
  virtual int    n_unknowns();

  virtual void   initial_x(double* _x);

  virtual double eval_f( const double* _x);

  virtual void   eval_gradient( const double* _x, double* _g);

  virtual void   eval_hessian ( const double* _x, SMatrixNP& _H);

  virtual void   store_result ( const double* _x );

  // advanced properties
  virtual bool   constant_gradient() const;
  virtual bool   constant_hessian()  const;
  virtual double max_feasible_step ( const double* _x, const double* _v);

private:

  // pointer to two original problems
  NProblemInterface* p1_;
  NProblemInterface* p2_;
  // weighting coefficients
  double c1_;
  double c2_;

  // temporary variables
  std::vector<double> g_temp_;
};


//=============================================================================
} // namespace COMISO
//=============================================================================
#endif // COMISO_EIGEN3_AVAILABLE
//=============================================================================
#endif // COMISO_NPROBLEMGMMINTERFACE_HH defined
//=============================================================================

