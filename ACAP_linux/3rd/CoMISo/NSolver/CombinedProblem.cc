//== COMPILE-TIME PACKAGE REQUIREMENTS ========================================
#include <CoMISo/Config/config.hh>
#if COMISO_EIGEN3_AVAILABLE

//== INCLUDES =================================================================
#include "CombinedProblem.hh"

#include <Base/Debug/DebOut.hh>

namespace COMISO {

CombinedProblem::CombinedProblem (NProblemInterface* _p1, NProblemInterface* _p2, const double _c1, const double _c2)
  : p1_(_p1) , p2_(_p2), c1_(_c1), c2_(_c2)
{
  DEB_enter_func;
  DEB_warning_if(p1_->n_unknowns() != p2_->n_unknowns(), 1,
    "CombinedProblem received two problems with different unknowns #");

  g_temp_.resize(p1_->n_unknowns());
}

int  CombinedProblem::n_unknowns()
{
  return p1_->n_unknowns();
}

void CombinedProblem::initial_x(double* _x)
{
  // use initial_x of first problem
  p1_->initial_x(_x);
}

double CombinedProblem::eval_f( const double* _x)
{
  return c1_*p1_->eval_f(_x) + c2_*p2_->eval_f(_x);
}

void CombinedProblem::eval_gradient( const double* _x, double* _g)
{
  p1_->eval_gradient(_x, _g);
  p2_->eval_gradient(_x, g_temp_.data());

  for( int i=0; i<this->n_unknowns(); ++i)
    _g[i] = c1_*_g[i]+c2_*g_temp_[i];
}

void CombinedProblem::eval_hessian ( const double* _x, SMatrixNP& _H)
{
  SMatrixNP H1, H2;
  p1_->eval_hessian(_x, H1);
  p2_->eval_hessian(_x, H2);

  _H.resize(n_unknowns(), n_unknowns());
  _H = c1_*H1 + c2_*H2;
}

void CombinedProblem::store_result ( const double* _x )
{
  p1_->store_result(_x);
  p2_->store_result(_x);
}


bool CombinedProblem::constant_gradient() const
{
  return (p1_->constant_gradient() && p2_->constant_gradient());
}

bool CombinedProblem::constant_hessian()  const
{
  return (p1_->constant_hessian() && p2_->constant_hessian());
}

double CombinedProblem::max_feasible_step ( const double* _x, const double* _v)
{
  return std::min(p1_->max_feasible_step(_x,_v), p2_->max_feasible_step(_x,_v));
}


//=============================================================================
} // namespace COMISO
//=============================================================================
#endif // COMISO_EIGEN3_AVAILABLE
//=============================================================================

