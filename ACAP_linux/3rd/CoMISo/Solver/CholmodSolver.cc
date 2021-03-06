/*===========================================================================*\
 *                                                                           *
 *                               CoMISo                                      *
 *      Copyright (C) 2008-2009 by Computer Graphics Group, RWTH Aachen      *
 *                           www.rwth-graphics.de                            *
 *                                                                           *
 *---------------------------------------------------------------------------* 
 *  This file is part of CoMISo.                                             *
 *                                                                           *
 *  CoMISo is free software: you can redistribute it and/or modify           *
 *  it under the terms of the GNU General Public License as published by     *
 *  the Free Software Foundation, either version 3 of the License, or        *
 *  (at your option) any later version.                                      *
 *                                                                           *
 *  CoMISo is distributed in the hope that it will be useful,                *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
 *  GNU General Public License for more details.                             *
 *                                                                           *
 *  You should have received a copy of the GNU General Public License        *
 *  along with CoMISo.  If not, see <http://www.gnu.org/licenses/>.          *
 *                                                                           *
\*===========================================================================*/ 

//== COMPILE-TIME PACKAGE REQUIREMENTS ========================================
#include <CoMISo/Config/config.hh>
#if COMISO_SUITESPARSE_AVAILABLE
//=============================================================================

#include "CholmodSolver.hh"


namespace COMISO {

CholmodSolver::CholmodSolver()
{
    mp_cholmodCommon = new cholmod_common;
    cholmod_start( mp_cholmodCommon );

    mp_L = 0;

    show_timings_ = false;

    mp_cholmodCommon->nmethods = 1;
    // use AMD ordering
    mp_cholmodCommon->method[0].ordering = CHOLMOD_AMD ;

    // use METIS ordering
    //    mp_cholmodCommon->method[0].ordering = CHOLMOD_METIS ;

    // try all methods
    // mp_cholmodCommon->nmethods = 9;
}


  //-----------------------------------------------------------------------------
  

CholmodSolver::~CholmodSolver()
{
    if( mp_L )
    {
	cholmod_free_factor( &mp_L, mp_cholmodCommon );
    }

    cholmod_finish( mp_cholmodCommon );
    delete mp_cholmodCommon;
    mp_cholmodCommon = NULL;
}
  

//-----------------------------------------------------------------------------


bool CholmodSolver::calc_system( const std::vector<int>&    _colptr, 
				 const std::vector<int>&    _rowind, 
				 const std::vector<double>& _values)
{
    if(show_timings_) sw_.start();

    colptr_ = _colptr;
    rowind_ = _rowind;
    values_ = _values;

    size_t n   = colptr_.size()-1;

    cholmod_sparse matA;

    matA.nrow = n;
    matA.ncol = n;
    matA.nzmax = _values.size();

    matA.p = &colptr_[0];
    matA.i = &rowind_[0];
    matA.x = &values_[0];
    matA.nz = 0;
    matA.z = 0;
    
    //    matA.stype = -1;
    matA.stype = 1;
    matA.itype = CHOLMOD_INT;
    matA.xtype = CHOLMOD_REAL;
    matA.dtype = CHOLMOD_DOUBLE;
    matA.sorted = 1;
    matA.packed = 1;


    // clean up
    if( mp_L )
    {
	cholmod_free_factor( &mp_L, mp_cholmodCommon );
	mp_L = 0;
    }

    if(show_timings_)
    {
      std::cerr << " Cholmod Timing cleanup: " << sw_.stop()/1000.0 << "s\n";
      sw_.start();
    }

    if( !(mp_L = cholmod_analyze( &matA, mp_cholmodCommon )) )
    {
	std::cout << "cholmod_analyze failed" << std::endl;
	return false;
    }

    // // show selected ordering method
    // std::cerr << "best    ordering was: " << mp_cholmodCommon->selected << std::endl;
    // std::cerr << "current ordering was: " << mp_cholmodCommon->current  << std::endl;


    if(show_timings_)
    {
      std::cerr << " Cholmod Timing analyze: " << sw_.stop()/1000.0 << "s\n";
      sw_.start();
    }

    if( !cholmod_factorize( &matA, mp_L, mp_cholmodCommon ) )
    {
	std::cout << "cholmod_factorize failed" << std::endl;
	return false;
    }

    if(show_timings_)
    {
      std::cerr << " Cholmod Timing factorize: " << sw_.stop()/1000.0 << "s\n";
      sw_.start();
    }

    return true;
}


//-----------------------------------------------------------------------------


bool CholmodSolver::calc_system_prepare_pattern( const std::vector<int>&    _colptr,
                                                 const std::vector<int>&    _rowind,
                                                 const std::vector<double>& _values,
                                                 const std::vector<int>&    _colptr2,
                                                 const std::vector<int>&    _rowind2,
                                                 const std::vector<double>& _values2 )
{
    if(show_timings_) sw_.start();

    colptr_ = _colptr;
    rowind_ = _rowind;
    values_ = _values;

    size_t n = colptr_.size() - 1;

    // setup matrix matA
    cholmod_sparse matA;

    matA.nrow = n;
    matA.ncol = n;
    matA.nzmax = _values.size();

    matA.p = &colptr_[0];
    matA.i = &rowind_[0];
    matA.x = &values_[0];
    matA.nz = 0;
    matA.z = 0;

    //    matA.stype = -1;
    matA.stype = 1;
    matA.itype = CHOLMOD_INT;
    matA.xtype = CHOLMOD_REAL;
    matA.dtype = CHOLMOD_DOUBLE;
    matA.sorted = 1;
    matA.packed = 1;

    // setup matrix matA_pattern
    cholmod_sparse matA_pattern;

    matA_pattern.nrow = n;
    matA_pattern.ncol = n;
    matA_pattern.nzmax = _values2.size();

    matA_pattern.p = (int*)(&_colptr2[0]);
    matA_pattern.i = (int*)(&_rowind2[0]);
    matA_pattern.x = (double*)(&_values2[0]);
    matA_pattern.nz = 0;
    matA_pattern.z = 0;

    //    matA_pattern.stype = -1;
    matA_pattern.stype = 1;
    matA_pattern.itype = CHOLMOD_INT;
    matA_pattern.xtype = CHOLMOD_REAL;
    matA_pattern.dtype = CHOLMOD_DOUBLE;
    matA_pattern.sorted = 1;
    matA_pattern.packed = 1;


    // clean up
    if( mp_L )
    {
        cholmod_free_factor( &mp_L, mp_cholmodCommon );
        mp_L = 0;
    }


    // compute permutation based on full pattern
    std::vector<int> perm(matA_pattern.nrow);
//    cholmod_metis(&matA_pattern, 0, 0, 0, perm.data(), mp_cholmodCommon) ;
    cholmod_amd(&matA_pattern, 0, 0, perm.data(), mp_cholmodCommon) ;

    if(show_timings_)
    {
      std::cerr << " Cholmod AMD ordering: " << sw_.stop()/1000.0 << "s\n";
      sw_.start();
    }


    if(show_timings_)
    {
      std::cerr << " Cholmod Timing cleanup: " << sw_.stop()/1000.0 << "s\n";
      sw_.start();
    }

    // fix permutation to given one
    mp_cholmodCommon->nmethods = 1;
    mp_cholmodCommon->method[0].ordering = CHOLMOD_GIVEN;

    if( !(mp_L = cholmod_analyze_p( &matA, perm.data(), 0, 0, mp_cholmodCommon )) )
    {
        std::cout << "cholmod_analyze failed" << std::endl;
        return false;
    }

    if(show_timings_)
    {
      std::cerr << " Cholmod Timing analyze: " << sw_.stop()/1000.0 << "s\n";
      sw_.start();
    }

    if( !cholmod_factorize( &matA, mp_L, mp_cholmodCommon ) )
    {
        std::cout << "cholmod_factorize failed" << std::endl;
        return false;
    }

    if(show_timings_)
    {
      std::cerr << " Cholmod Timing factorize: " << sw_.stop()/1000.0 << "s\n";
      sw_.start();
    }

    return true;
}

  //-----------------------------------------------------------------------------

    
bool CholmodSolver::update_system( const std::vector<int>& _colptr, 
				   const std::vector<int>& _rowind, 
				   const std::vector<double>& _values )
{
    if( !mp_L )
	return false;

    colptr_ = _colptr;
    rowind_ = _rowind;
    values_ = _values;
    size_t n = colptr_.size() - 1;

    cholmod_sparse matA;

    matA.nrow = n;
    matA.ncol = n;
    matA.nzmax = _values.size();

    matA.p = &colptr_[0];
    matA.i = &rowind_[0];
    matA.x = &values_[0];
    matA.nz = 0;
    matA.z = 0;
    
    //    matA.stype = -1;
    matA.stype = 1;
    matA.itype = CHOLMOD_INT;
    matA.xtype = CHOLMOD_REAL;
    matA.dtype = CHOLMOD_DOUBLE;
    matA.sorted = 1;
    matA.packed = 1;


    if( !cholmod_factorize( &matA, mp_L, mp_cholmodCommon ) )
    {
	std::cout << "cholmod_factorize failed" << std::endl;
	return false;
    }

    return true;
}


//-----------------------------------------------------------------------------


bool CholmodSolver::update_downdate_factor( const std::vector<int>&    _colptr,
                                            const std::vector<int>&    _rowind,
                                            const std::vector<double>& _values,
                                            const bool                 _upd)
{
    if(show_timings_) sw_.start();

    colptr_ = _colptr;
    rowind_ = _rowind;
    values_ = _values;

    size_t n = colptr_.size() - 1;

    cholmod_sparse matA;

    matA.nrow = mp_L->n;
    matA.ncol = n;
    matA.nzmax = _values.size();

    matA.p = &colptr_[0];
    matA.i = &rowind_[0];
    matA.x = &values_[0];
    matA.nz = 0;
    matA.z = 0;

    matA.stype = 0;
    matA.itype = CHOLMOD_INT;
    matA.xtype = CHOLMOD_REAL;
    matA.dtype = CHOLMOD_DOUBLE;
    matA.sorted = 1;
    matA.packed = 1;

    // get permuted matrix
    cholmod_sparse* matAp = cholmod_submatrix ( &matA, (int*)mp_L->Perm, mp_L->n, 0, -1, true, true, mp_cholmodCommon);
    if(show_timings_)
    {
      std::cerr << " Cholmod conversion Timing: " << sw_.stop()/1000.0 << "s\n";
      sw_.start();
    }

    if( !cholmod_updown( _upd, matAp, mp_L, mp_cholmodCommon))
    {
      std::cerr << "Warning: Cholmod update/downdate failed!" << std::endl;
    }

    if(show_timings_)
    {
      std::cerr << " Cholmod update/downdate Timing: " << sw_.stop()/1000.0 << "s\n";
      sw_.start();
    }

    // clean up permuted matrix
    if(matAp)
      cholmod_free_sparse(&matAp, mp_cholmodCommon);

    if(show_timings_)
    {
      std::cerr << " Cholmod free sparse Timing: " << sw_.stop()/1000.0 << "s\n";
    }

    return true;
}


//-----------------------------------------------------------------------------
  

bool CholmodSolver::solve( double * _x, double * _b)
{
    const size_t n = mp_L->n;

    cholmod_dense *x, b;

    b.nrow = n;
    b.ncol = 1;
    b.nzmax = n;
    b.d = b.nrow;
    b.x = _b;
    b.z = 0;
    b.xtype = CHOLMOD_REAL;
    b.dtype = CHOLMOD_DOUBLE;

    if( !(x = cholmod_solve( CHOLMOD_A, mp_L, &b, mp_cholmodCommon )) )
    {
	std::cout << "cholmod_solve failed" << std::endl;
	return false;
    }
    
    for( unsigned int i = 0; i < n; ++i )
	_x[i] = ((double*)x->x)[i];

    cholmod_free_dense( &x, mp_cholmodCommon );

    return true;
}


//-----------------------------------------------------------------------------

int CholmodSolver::dimension()
{
  return std::max(int(0), (int)(colptr_.size()-1));
}

//-----------------------------------------------------------------------------

bool CholmodSolver::
solve ( std::vector<double>& _x0, std::vector<double>& _b)
{
  return solve( &(_x0[0]), &(_b[0]));
}

//-----------------------------------------------------------------------------

bool& CholmodSolver::
show_timings()
{
  return show_timings_;
}


}

//=============================================================================
#endif // COMISO_SUITESPARSE_AVAILABLE
//=============================================================================
