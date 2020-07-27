//=============================================================================
//
//  CLASS LinearConstraintConverter
//
//=============================================================================


#ifndef COMISO_LINEARCONSTRAINTCONVERTER_HH
#define COMISO_LINEARCONSTRAINTCONVERTER_HH


//== INCLUDES =================================================================

#include <CoMISo/Config/CoMISoDefines.hh>
#include "NConstraintInterface.hh"
#include "LinearConstraint.hh"

#include <Base/Code/Quality.hh>
LOW_CODE_QUALITY_SECTION_BEGIN
#include <Eigen/StdVector>
#include <Eigen/Dense>
#include <Eigen/Sparse>
LOW_CODE_QUALITY_SECTION_END


//== FORWARDDECLARATIONS ======================================================

//== NAMESPACES ===============================================================

namespace COMISO {

//== CLASS DEFINITION =========================================================

	      

/** \class NProblemGmmInterface NProblemGmmInterface.hh <ACG/.../NPRoblemGmmInterface.hh>

    Brief Description.
  
    A more elaborate description follows.
*/
class COMISODLLEXPORT LinearConstraintConverter
{
public:

  // sparse vector type
  typedef NConstraintInterface::SVectorNC SVectorNC;

  typedef Eigen::SparseMatrix<double,Eigen::ColMajor> SparseMatrixC;
  typedef Eigen::SparseMatrix<double,Eigen::RowMajor> SparseMatrixR;
  typedef Eigen::VectorXd                             VectorXd;

  LinearConstraintConverter(SparseMatrixR& _A, VectorXd& _b)
  : A_(_A), b_(_b), linear_constraints_initialized_(false)
  {}

  LinearConstraintConverter(SparseMatrixC& _A, VectorXd& _b)
  : A_(_A), b_(_b), linear_constraints_initialized_(false)
  {}

  std::vector<NConstraintInterface*>& constraints_nsolver()
  {
    initialize_linear_constraints();
    return constraint_pointers_;
  }

private:

  void initialize_linear_constraints()
  {
    if(!linear_constraints_initialized_)
    {
      // tag as done
      linear_constraints_initialized_ = true;

      int m = A_.rows();
      int n = A_.cols();

      linear_constraints_.clear();
      linear_constraints_.resize(m);

      constraint_pointers_.clear();
      constraint_pointers_.resize(m);

      for( int i=0; i<m; ++i)
      {
        // convert i-th constraint
        linear_constraints_.resize(n);
        linear_constraints_[i].coeffs() =  A_.row(i);
        linear_constraints_[i].b()      = -b_[i];
        // store pointer
        constraint_pointers_[i] = &(linear_constraints_[i]);
      }
    }
  }

private:
  SparseMatrixR A_;
  VectorXd      b_;

  bool                               linear_constraints_initialized_;
  std::vector<LinearConstraint>      linear_constraints_;
  std::vector<NConstraintInterface*> constraint_pointers_;
};


//=============================================================================
} // namespace COMISO
//=============================================================================
//=============================================================================
#endif // ACG_LINEARCONSTRAINTCONVERTER_HH defined
//=============================================================================

