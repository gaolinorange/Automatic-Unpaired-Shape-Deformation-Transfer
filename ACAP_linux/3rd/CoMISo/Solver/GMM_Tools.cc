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



//=============================================================================
//
//  CLASS GMM_Tools - IMPLEMENTATION
//
//=============================================================================

//== INCLUDES =================================================================

#include "GMM_ToolsT.cc"

#include <CoMISo/Config/GmmTypes.hh>
#include <CoMISo/Config/StdTypes.hh>

// explicit instantiation

namespace COMISO_GMM
{
using namespace COMISO_STD;

template void factored_to_quadratic(WSRowMatrix&, WSColMatrix&, DoubleVector&);
template void factored_to_quadratic(WSRowMatrix&, RSColMatrix&, DoubleVector&);
template void factored_to_quadratic(RSRowMatrix&, RSColMatrix&, DoubleVector&);

template void eliminate_csc_vars(const IntVector&, const DoubleVector&,
  CSCMatrix&, DoubleVector&, DoubleVector&);

template void eliminate_csc_vars2(const IntVector&, const DoubleVector&,
  CSCMatrix&, DoubleVector&, DoubleVector&);

template void eliminate_csc_vars2(const UIntVector&, const DoubleVector&,
  CSCMatrix&, DoubleVector&, DoubleVector&);

template double residuum_norm(CSCMatrix& , DoubleVector&, DoubleVector&);

template void fix_var_csc_symmetric(const unsigned int, const double,
  CSCMatrix&, DoubleVector&, DoubleVector&);

template void eliminate_vars_idx(const IntVector&, IntVector&, int, int);

}//namespace COMISO_GMM
