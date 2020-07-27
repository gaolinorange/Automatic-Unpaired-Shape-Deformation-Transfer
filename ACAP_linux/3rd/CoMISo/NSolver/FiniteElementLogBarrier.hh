//=============================================================================
//
//  CLASS FiniteElementLogBarrier
//
//=============================================================================


#ifndef COMISO_FINITEELEMENTLOGBARRIER_HH
#define COMISO_FINITEELEMENTLOGBARRIER_HH


//== INCLUDES =================================================================

#include <CoMISo/Config/CoMISoDefines.hh>
#include "NProblemInterface.hh"


//== FORWARDDECLARATIONS ======================================================

//== NAMESPACES ===============================================================

namespace COMISO { 

//== CLASS DEFINITION =========================================================

	      
/** \class FiniteElementLogBarrierLowerBound

    Implements function of the type f(x) = -c1*log(x-c0) with constants (c0,c1)
  
    A more elaborate description follows.
*/

class FiniteElementLogBarrierLowerBound
{
public:

  // define dimensions
  const static int NV = 1;
  const static int NC = 2;

  typedef Eigen::Matrix<size_t,NV,1> VecI;
  typedef Eigen::Matrix<double,NV,1> VecV;
  typedef Eigen::Matrix<double,NC,1> VecC;
  typedef Eigen::Triplet<double> Triplet;

  inline double eval_f       (const VecV& _x, const VecC& _c) const
  {
    return -_c[1]*std::log(_x[0]-_c[0]);
  }

  inline void   eval_gradient(const VecV& _x, const VecC& _c, VecV& _g) const
  {
    _g[0] = -_c[1]/(_x[0]-_c[0]);
  }

  inline void   eval_hessian (const VecV& _x, const VecC& _c, std::vector<Triplet>& _triplets) const
  {
    _triplets.clear();
    _triplets.push_back(Triplet(0,0,_c[1]/std::pow(_x[0]-_c[0],2)));
  }

  inline double max_feasible_step(const VecV& _x, const VecV& _v, const VecC& _c)
  {
    if(_v[0] >=0.0)
      return DBL_MAX;
    else
      return 0.999*(_c[0]-_x[0])/_v[0];
  }

};

/** \class FiniteElementLogBarrierUpperBound

    Implements function of the type f(x) = -c1*log(c0-x) with constants (c0,c1)

    A more elaborate description follows.
*/


class FiniteElementLogBarrierUpperBound
{
public:

  // define dimensions
  const static int NV = 1;
  const static int NC = 2;

  typedef Eigen::Matrix<size_t,NV,1> VecI;
  typedef Eigen::Matrix<double,NV,1> VecV;
  typedef Eigen::Matrix<double,NC,1> VecC;
  typedef Eigen::Triplet<double> Triplet;

  inline double eval_f       (const VecV& _x, const VecC& _c) const
  {
    return -_c[1]*std::log(_c[0]-_x[0]);
  }

  inline void   eval_gradient(const VecV& _x, const VecC& _c, VecV& _g) const
  {
    _g[0] = _c[1]/(_c[0]-_x[0]);
  }

  inline void   eval_hessian (const VecV& _x, const VecC& _c, std::vector<Triplet>& _triplets) const
  {
    _triplets.clear();
    _triplets.push_back(Triplet(0,0,_c[1]/std::pow(_c[0]-_x[0],2)));
  }

  inline double max_feasible_step(const VecV& _x, const VecV& _v, const VecC& _c)
  {
    if(_v[0] <=0.0)
      return DBL_MAX;
    else
      return 0.999*(_c[0]-_x[0])/_v[0];
  }

};


//=============================================================================
} // namespace COMISO
//=============================================================================
#endif // COMISO_FINITEELEMENTLOGBARRIER_HH defined
//=============================================================================

