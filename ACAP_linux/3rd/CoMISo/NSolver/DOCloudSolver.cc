// (C) Copyright 2015 by Autodesk, Inc.

//=============================================================================
//
//  CLASS DOCloudSolver - IMPLEMENTATION
//
//=============================================================================

//== INCLUDES =================================================================

//== COMPILE-TIME PACKAGE REQUIREMENTS ========================================
#include "DOCloudSolver.hh"
//=============================================================================
#if COMISO_DOCLOUD_AVAILABLE
#include "DOCloudCache.hh"
#include "DOCloudJob.hh"
#include "cURLpp.hh"
#include "CoMISo/Utils/CoMISoError.hh"

#include <Base/Debug/DebUtils.hh>

#include <stdexcept>
#include <algorithm>
#include <fstream>
#include <iomanip>

DEB_module("DOCloudSolver")

//== NAMESPACES ===============================================================

namespace COMISO {

//== IMPLEMENTATION ==========================================================
namespace DOcloud {

namespace {

#define P(X) ((X).data())
#define XVAR(IDX) "x" << IDX

class WriteExpression
{
  // lp format allows a max line length = 560.
  // For simplicity we put end line when the number of written characters on the
  // same line exceeds LINE_TRESHOLD_LEN.
  enum { LINE_TRESHOLD_LEN = 100 };

public:
  WriteExpression(std::ostringstream& _out_str) : out_str_stream(_out_str)
  {
    start();
  }

  void start()
  {
    f_size_ = out_str_stream.tellp();
    at_start_ = true;
  }
  
  // Writes a monomial.
  void add_monomial(const double _coeff, const size_t _i_var)
  {
    if (_coeff == 0)
      return;
    add_monomial_internal(_coeff, _i_var);
    wrap_long_line();
  }

  // Writes a binomial.
  void add_binomial(const double _coeff, const size_t _i_var, const size_t _j_var)
  {
    if (_coeff == 0)
      return;
    add_monomial_internal(_coeff, _i_var);
    if (_j_var == _i_var)
      out_str_stream << "^2";
    else
      out_str_stream << " * " << XVAR(_j_var);
    wrap_long_line();
  }

private:

  void wrap_long_line()
  {
    const auto new_f_size = out_str_stream.tellp();
    if (new_f_size - f_size_ > LINE_TRESHOLD_LEN)
    {
      out_str_stream << std::endl;
      f_size_ = new_f_size;
    }
  }

  void add_monomial_internal(const double _coeff, const size_t _i_var)
  {
    if (_coeff == 1)
    {
      if (!at_start_)
        out_str_stream << " + ";
    }
    else if (_coeff == -1)
      out_str_stream << " - ";
    else
    {
      if (!at_start_)
      {
        if (_coeff > 0)
          out_str_stream << " + ";
        else
          out_str_stream << ' ';
      }
      out_str_stream << _coeff << ' ';
    }
    out_str_stream << XVAR(_i_var);
    at_start_ = false;
  }

private:
  std::ostringstream& out_str_stream;
  std::fstream::pos_type f_size_;
  bool at_start_;
};

// Create a lp file for the given constraints and object function.
// Here is the lp format specifications:
// http://www-01.ibm.com/support/knowledgecenter/SSSA5P_12.6.1/ilog.odms.cplex.help/CPLEX/FileFormats/topics/LP.html
std::string create_lp_string(
  NProblemInterface* _problem,
  const std::vector<NConstraintInterface*>& _constraints,
  const std::vector<PairIndexVtype>& _discrete_constraints,
  const std::vector<double>& _x
  )
{
  const int n_cols = _problem->n_unknowns(); // Unknowns #

  std::ostringstream lp_str_stream;

  // Set the ofstream options.
  lp_str_stream << std::setprecision(std::numeric_limits<double>::digits10 + 2);

  lp_str_stream << "\\Problem name: " << std::endl << std::endl;
  lp_str_stream << "Minimize" << std::endl;

  // Writes objective function.
  lp_str_stream << "obj: ";

  WriteExpression wrte_expr(lp_str_stream);
  // 1. Linear part.
  std::vector<double> objective(n_cols);
  _problem->eval_gradient(P(_x), P(objective));
  for (size_t i = 0; i < objective.size(); ++i)
    wrte_expr.add_monomial(objective[i], i);

  // 2. Quadratic part (if requested).
  if (!_problem->constant_gradient())
  {
    NProblemInterface::SMatrixNP H;
    _problem->eval_hessian(P(_x), H);
    lp_str_stream << " + [ ";
    for (int i = 0; i < H.outerSize(); ++i)
    {
      for (NProblemInterface::SMatrixNP::InnerIterator it(H, i); it; ++it)
        wrte_expr.add_binomial(it.value(), it.row(), it.col());
    }
    lp_str_stream << " ] / 2";
  }


  // Writes constraints.
  lp_str_stream << std::endl << "Subject To" << std::endl;
  for (const auto& cstr : _constraints)
  {
    NConstraintInterface::SVectorNC gc;
    cstr->eval_gradient(P(_x), gc);

    wrte_expr.start();
    for (NConstraintInterface::SVectorNC::InnerIterator v_it(gc); v_it; ++v_it)
    {
      auto coeff = v_it.value();
      wrte_expr.add_monomial(coeff, v_it.index());
    }
    switch (cstr->constraint_type())
    {
    case NConstraintInterface::NC_EQUAL:
      lp_str_stream << " = ";
      break;
    case NConstraintInterface::NC_GREATER_EQUAL:
      lp_str_stream << " >= ";
      break;
    case NConstraintInterface::NC_LESS_EQUAL:
      lp_str_stream << " <= ";
      break;
    }
    lp_str_stream << -cstr->eval_constraint(P(_x)) << std::endl;
  }

  // Writes the variables.
  lp_str_stream << "Bounds" << std::endl;
  for (size_t i = 0; i < n_cols; ++i)
    lp_str_stream << XVAR(i) << " Free" << std::endl;

  // Integer and binary variables.
  std::vector<unsigned int> int_var, bin_var;
  for (const auto& dc : _discrete_constraints)
  {
    if (dc.second == Integer)
      int_var.push_back(dc.first);
    else if (dc.second == Binary)
      bin_var.push_back(dc.first);
  }
  auto write_var_set = [&lp_str_stream](const std::vector<unsigned int>& _vars,
    const char* _type)
  {
    if (_vars.empty())
      return;
    // Writes integer variables.
    lp_str_stream << _type << std::endl;
    auto var_it = _vars.begin();
    lp_str_stream << XVAR(*var_it);
    size_t n_wrt_var = 1;
    while (++var_it != _vars.end())
    {
      if (n_wrt_var++ % 16) // 16 variables per line. Lines length must be < 560.
        lp_str_stream << ' ';
      else
        lp_str_stream << std::endl;
      lp_str_stream << XVAR(*var_it);
    }
    lp_str_stream << std::endl;

  };
  // Writes integer variables.
  write_var_set(int_var, "Integers");

  // Writes Binary variables.
  write_var_set(bin_var, "Binary");

  lp_str_stream << "End";

  return std::string(lp_str_stream.str());
}

#undef XVAR

} // namespace

} // namespace DOcloud

void DOCloudSolver::solve(
  NProblemInterface*                        _problem,
  const std::vector<NConstraintInterface*>& _constraints,
  const std::vector<PairIndexVtype>&        _discrete_constraints,
  const double                              _time_limit
)
{
  DEB_enter_func;
  DEB_warning_if(!_problem->constant_hessian(), 1,
    "DOCloudSolver received a problem with non-constant hessian!");
  DEB_warning_if(!_problem->constant_gradient(), 1,
    "DOCloudSolver received a problem with non-constant gradient!");

  std::vector<double> x(_problem->n_unknowns(), 0.0); // solution
  const std::string mip_lp = DOcloud::create_lp_string(_problem, _constraints,  
    _discrete_constraints, x);

  double obj_val;
  DOcloud::Cache cache(mip_lp);
  if (cache.restore_result(x, obj_val))
    DEB_line(3, "MIP cached.")
  else
  {
    DEB_line(1, "MIP not cached, computing optimization.");
    const std::string lp_hash = cache.hash() + ".lp";
    DOcloud::Job job(lp_hash, mip_lp);
    job.setup();
    job.wait();
    obj_val = job.solution(x);
    cache.store_result(x, obj_val);
  }
  COMISO_THROW_if(x.empty(), MIPS_NO_SOLUTION);

  DEB_only(
  // The lp problem ignores the constant term in the objective function.
  const std::vector<double> x_zero(_problem->n_unknowns(), 0.0);
  const auto zero_val = _problem->eval_f(P(x_zero));
  const auto test_obj_val = _problem->eval_f(P(x));
  DEB_error_if(fabs(obj_val + zero_val - test_obj_val) > (1e-8 + zero_val * 0.01),
               "DOCloudSolver solved a wrong problem.");
  )
  
  _problem->store_result(P(x));
}

#undef P

//=============================================================================
} // namespace COMISO
//=============================================================================

#endif // COMISO_DOCLOUD_AVAILABLE
//=============================================================================

