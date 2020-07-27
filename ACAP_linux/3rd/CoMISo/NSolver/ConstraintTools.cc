//== COMPILE-TIME PACKAGE REQUIREMENTS ========================================
#include <CoMISo/Config/config.hh>
#if COMISO_EIGEN3_AVAILABLE

//== INCLUDES =================================================================
#include "ConstraintTools.hh"

#include <CoMISo/Utils/MutablePriorityQueueT.hh>

#include <Base/Debug/DebOut.hh>



namespace COMISO {

//-----------------------------------------------------------------------------

void
ConstraintTools::
remove_dependent_linear_constraints( std::vector<NConstraintInterface*>& _constraints, const double _eps )
{
  // split into linear and nonlinear
  std::vector<NConstraintInterface*> lin_const, nonlin_const;

  for(unsigned int i=0; i<_constraints.size(); ++i)
  {
    if(_constraints[i]->is_linear() && _constraints[i]->constraint_type() == NConstraintInterface::NC_EQUAL)
      lin_const.push_back(_constraints[i]);
    else
      nonlin_const.push_back(_constraints[i]);
  }

  remove_dependent_linear_constraints_only_linear_equality( lin_const);

  for(unsigned int i=0; i<lin_const.size(); ++i)
    nonlin_const.push_back(lin_const[i]);

  // return filtered constraints
  _constraints.swap(nonlin_const);
}


//-----------------------------------------------------------------------------

void
ConstraintTools::
remove_dependent_linear_constraints_only_linear_equality( std::vector<NConstraintInterface*>& _constraints, const double _eps)
{
  DEB_enter_func;
  // make sure that constraints are available
  if(_constraints.empty()) return;

  // 1. copy (normalized) data into gmm dynamic sparse matrix
  size_t n(_constraints[0]->n_unknowns());
  size_t m(_constraints.size());
  std::vector<double> x(n, 0.0);
  NConstraintInterface::SVectorNC g;
  RMatrixGMM A;
  gmm::resize(A,m, n+1);
  for(unsigned int i=0; i<_constraints.size(); ++i)
  {
    // store rhs in last column
    A(i,n) = _constraints[i]->eval_constraint(x.data());
    // get and store coefficients
    _constraints[i]->eval_gradient(x.data(), g);
    double v_max(0.0);
    for (NConstraintInterface::SVectorNC::InnerIterator it(g); it; ++it)
    {
      A(i,it.index()) = it.value();
      v_max = std::max(v_max, std::abs(it.value()));
    }
    // normalize row
    if(v_max != 0.0)
      gmm::scale(A.row(i), 1.0/v_max);
  }

  // 2. get additionally column matrix to exploit column iterators
  CMatrixGMM Ac;
  gmm::resize(Ac, gmm::mat_nrows(A), gmm::mat_ncols(A));
  gmm::copy(A, Ac);

  // 3. initialize priorityqueue for sorting
  // init priority queue
  MutablePriorityQueueT<gmm::size_type, gmm::size_type> queue;
  queue.clear(m);
  for (gmm::size_type i = 0; i<m; ++i)
  {
    gmm::size_type cur_nnz = gmm::nnz( gmm::mat_row(A,i));
    if (A(i,n) != 0.0)
      --cur_nnz;

    queue.update(i, cur_nnz);
  }

  // track row status -1=undecided, 0=remove, 1=keep
  std::vector<int> row_status(m, -1);
  std::vector<gmm::size_type> keep;
//  std::vector<int> remove;

  // for all conditions
  while(!queue.empty())
  {
    // get next row
    gmm::size_type i = queue.get_next();
    gmm::size_type j = find_max_abs_coeff(A.row(i));
    double aij = A(i,j);
    if(std::abs(aij) <= _eps)
    {
//      std::cerr << "drop " << aij << "in row " << i << "and column " << j << std::endl;
      // constraint is linearly dependent
      row_status[i] = 0;
      if(std::abs(A(i,n)) > _eps)
        std::cerr << "Warning: found dependent constraint with nonzero rhs " << A(i,n) << std::endl;
    }
    else
    {
//      std::cerr << "keep " << aij << "in row " << i << "and column " << j << std::endl;

      // constraint is linearly independent
      row_status[i] = 1;
      keep.push_back(i);

      // update undecided constraints
      // copy col
      SVectorGMM col = Ac.col(j);

      // copy row
      SVectorGMM row = A.row(i);

      // iterate over column
      gmm::linalg_traits<SVectorGMM>::const_iterator c_it   = gmm::vect_const_begin(col);
      gmm::linalg_traits<SVectorGMM>::const_iterator c_end  = gmm::vect_const_end(col);

      for(; c_it != c_end; ++c_it)
        if( row_status[c_it.index()] == -1) // only process unvisited rows
        {
          // row idx
          gmm::size_type k = c_it.index();

          double s = -(*c_it)/aij;
          add_row_simultaneously( k, s, row, A, Ac, _eps);
          // make sure the eliminated entry is 0 on all other rows
          A( k, j) = 0;
          Ac(k, j) = 0;

          gmm::size_type cur_nnz = gmm::nnz( gmm::mat_row(A,k));
          if( A(k,n) != 0.0)
            --cur_nnz;

          queue.update(k, cur_nnz);
        }
    }
  }

  DEB_line(2, "removed " << _constraints.size()-keep.size() << 
    " dependent linear constraints out of " << _constraints.size());

  // 4. store result
  std::vector<NConstraintInterface*> new_constraints;
  for(unsigned int i=0; i<keep.size(); ++i)
    new_constraints.push_back(_constraints[keep[i]]);

  // return linearly independent ones
  _constraints.swap(new_constraints);
}


//-----------------------------------------------------------------------------


gmm::size_type
ConstraintTools::
find_max_abs_coeff(SVectorGMM& _v)
{
  size_t n = _v.size();
  gmm::size_type imax(0);
  double       vmax(0.0);

  gmm::linalg_traits<SVectorGMM>::const_iterator c_it   = gmm::vect_const_begin(_v);
  gmm::linalg_traits<SVectorGMM>::const_iterator c_end  = gmm::vect_const_end(_v);

  for(; c_it != c_end; ++c_it)
    if(c_it.index() != n-1)
      if(std::abs(*c_it) > vmax)
      {
        imax = c_it.index();
        vmax = *c_it;
      }

  return imax;
}


//-----------------------------------------------------------------------------


void
ConstraintTools::
add_row_simultaneously( gmm::size_type _row_i,
                        double      _coeff,
                        SVectorGMM& _row,
                        RMatrixGMM& _rmat,
                        CMatrixGMM& _cmat,
                        const double _eps )
{
  typedef gmm::linalg_traits<SVectorGMM>::const_iterator RIter;
  RIter r_it  = gmm::vect_const_begin(_row);
  RIter r_end = gmm::vect_const_end(_row);

  for(; r_it!=r_end; ++r_it)
  {
    _rmat(_row_i, r_it.index()) += _coeff*(*r_it);
    _cmat(_row_i, r_it.index()) += _coeff*(*r_it);
    if( std::abs(_rmat(_row_i, r_it.index())) < _eps )
    {
      _rmat(_row_i, r_it.index()) = 0.0;
      _cmat(_row_i, r_it.index()) = 0.0;
    }
  }
}

//=============================================================================
} // namespace COMISO
//=============================================================================
#endif // COMISO_EIGEN3_AVAILABLE
//=============================================================================

