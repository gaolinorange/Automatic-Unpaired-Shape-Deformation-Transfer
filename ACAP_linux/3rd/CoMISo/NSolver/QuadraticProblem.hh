#ifndef QUADRATICPROBLEM_H
#define QUADRATICPROBLEM_H

#include <CoMISo/Config/config.hh>
#include <CoMISo/Utils/StopWatch.hh>
#include <vector>
#include <CoMISo/NSolver/NProblemInterface.hh>
#include <Base/Code/Quality.hh>
LOW_CODE_QUALITY_SECTION_BEGIN
#include <Eigen/Eigen>
#include <Eigen/Core>
#include <Eigen/Sparse>
LOW_CODE_QUALITY_SECTION_END


//== NAMESPACES ===============================================================

namespace COMISO {

// this problem optimizes the quadratic functional 0.5*x^T A x -x^t b + c
class QuadraticProblem : public COMISO::NProblemInterface
{
public:

  // Sparse Matrix Type
  //  typedef Eigen::DynamicSparseMatrix<double,Eigen::ColMajor> SMatrixNP;

  QuadraticProblem()
  : A_(0,0), b_(Eigen::VectorXd::Index(0)), c_(0.0)
  {

  }

  QuadraticProblem(SMatrixNP& _A, Eigen::VectorXd& _b, const double _c)
  : A_(_A), c_(_c)
  {
    if(A_.rows() != A_.cols())
      std::cerr << "Warning: matrix not square in QuadraticProblem" << std::endl;
    b_ = _b;
    x_ = Eigen::VectorXd::Zero(A_.cols());
  }


  // number of unknowns
  virtual int n_unknowns()
  {
     return A_.rows();
  }

  // initial value where the optimization should start from
  virtual void initial_x(double* _x)
  {
        for( int i=0; i<this->n_unknowns(); ++i)
            _x[i] = x_[i];
  }

  // function evaluation at location _x
  virtual double eval_f( const double* _x )
  {
    Eigen::Map<const Eigen::VectorXd> x(_x, this->n_unknowns());

    return (double)(x.transpose()*A_*x)*0.5 - (double)(x.transpose()*b_) + c_;
  }

  // gradient evaluation at location _x
  virtual void   eval_gradient( const double* _x, double*    _g)
  {
    Eigen::Map<const Eigen::VectorXd> x(_x, this->n_unknowns());
    Eigen::Map<Eigen::VectorXd> g(_g, this->n_unknowns());

    g = A_*x - b_;
   }

  // hessian matrix evaluation at location _x
  virtual void   eval_hessian ( const double* _x, SMatrixNP& _H)
  {
    _H = A_;
  }

  // print result
  virtual void   store_result ( const double* _x               )
  {
    Eigen::Map<const Eigen::VectorXd> x(_x, this->n_unknowns());
    x_ = x;
  }

  // get current solution
  Eigen::VectorXd& x() { return x_;}

  // advanced properties
  virtual bool   constant_hessian() const { return true; }

  void set_A(const SMatrixNP& _A)
  {
    A_ = _A;
    if(A_.rows() != A_.cols())
        std::cerr << "Warning: matrix not square in QuadraticProblem" << std::endl;
    x_ = Eigen::VectorXd::Zero(A_.cols());
  }

  void set_b(const Eigen::VectorXd& _b)
  {
    b_ = _b;
  }

  void set_c( const double _c)
  {
    c_ = _c;
  }

private:

  // quadratic problem 0.5*x^T A x -x^t b + c
 SMatrixNP       A_;
 Eigen::VectorXd b_;
 double          c_;
 // current solution, which is also used as initial value
 Eigen::VectorXd x_;
};

//=============================================================================
} // namespace COMISO
//=============================================================================

#endif // QUADRATICPROBLEM_H
