//=============================================================================
//
//  CLASS IPOPTSolver
//
//=============================================================================


#ifndef COMISO_IPOPTSOLVER_HH
#define COMISO_IPOPTSOLVER_HH


//== COMPILE-TIME PACKAGE REQUIREMENTS ========================================
#include <CoMISo/Config/config.hh>
#if COMISO_IPOPT_AVAILABLE

//== INCLUDES =================================================================

#include <CoMISo/Config/CoMISoDefines.hh>
#include <CoMISo/Utils/StopWatch.hh>
#include <vector>
#include <cstddef>
#include <gmm/gmm.h>
#include "NProblemGmmInterface.hh"
#include "NProblemInterface.hh"
#include "NProblemIPOPT.hh"
#include "NConstraintInterface.hh"
#include "BoundConstraint.hh"
#include <IpTNLP.hpp>
#include <IpIpoptApplication.hpp>
#include <IpSolveStatistics.hpp>

//== FORWARDDECLARATIONS ======================================================

//== NAMESPACES ===============================================================

namespace COMISO {

//== CLASS DEFINITION =========================================================

	      

/** \class NewtonSolver NewtonSolver.hh <ACG/.../NewtonSolver.hh>

    Brief Description.
  
    A more elaborate description follows.
*/
class COMISODLLEXPORT IPOPTSolver
{
public:
   
  /// Default constructor -> set up IpOptApplication
  IPOPTSolver();
 
  // ********** SOLVE **************** //
  // solve -> returns ipopt status code
//------------------------------------------------------
//  enum ApplicationReturnStatus
//    {
//      Solve_Succeeded=0,
//      Solved_To_Acceptable_Level=1,
//      Infeasible_Problem_Detected=2,
//      Search_Direction_Becomes_Too_Small=3,
//      Diverging_Iterates=4,
//      User_Requested_Stop=5,
//      Feasible_Point_Found=6,
//
//      Maximum_Iterations_Exceeded=-1,
//      Restoration_Failed=-2,
//      Error_In_Step_Computation=-3,
//      Maximum_CpuTime_Exceeded=-4,
//      Not_Enough_Degrees_Of_Freedom=-10,
//      Invalid_Problem_Definition=-11,
//      Invalid_Option=-12,
//      Invalid_Number_Detected=-13,
//
//      Unrecoverable_Exception=-100,
//      NonIpopt_Exception_Thrown=-101,
//      Insufficient_Memory=-102,
//      Internal_Error=-199
//    };
//------------------------------------------------------

  int solve(NProblemInterface*    _problem, const std::vector<NConstraintInterface*>& _constraints);

  // same as above with additional lazy constraints that are only added iteratively to the problem if not satisfied
  int solve(NProblemInterface*                        _problem,
            const std::vector<NConstraintInterface*>& _constraints,
            const std::vector<NConstraintInterface*>& _lazy_constraints,
            const double                              _almost_infeasible = 0.5,
            const int                                 _max_passes        = 5   );


  // for convenience, if no constraints are given
  int solve(NProblemInterface*    _problem);

  // deprecated interface for backwards compatibility
  int solve(NProblemGmmInterface* _problem, std::vector<NConstraintInterface*>& _constraints);

  // ********* CONFIGURATION ********************* //
  // access the ipopt-application (for setting parameters etc.)
  // examples: app().Options()->SetIntegerValue("max_iter", 100);
  //           app().Options()->SetStringValue("derivative_test", "second-order");
  //           app().Options()->SetStringValue("hessian_approximation", "limited-memory");

  Ipopt::IpoptApplication& app() {return (*app_); }


  void set_print_level(const int _level)
  {
    app().Options()->SetIntegerValue("print_level", _level);
    print_level_ = _level;
  }

protected:
  double* P(std::vector<double>& _v)
  {
    if( !_v.empty())
      return ((double*)&_v[0]);
    else
      return 0;
  }

private:

  // smart pointer to IpoptApplication to set options etc.
  Ipopt::SmartPtr<Ipopt::IpoptApplication> app_;

  int print_level_;
};


//=============================================================================
} // namespace COMISO

//=============================================================================
#endif // COMISO_IPOPT_AVAILABLE
//=============================================================================
#endif // ACG_IPOPTSOLVER_HH defined
//=============================================================================

