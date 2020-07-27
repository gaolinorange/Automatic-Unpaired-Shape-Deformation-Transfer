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

#ifndef TOOLS_HH
#define TOOLS_HH

#include <math.h>
#include <float.h>

inline int int_round(const double _x) 
{ 
  return int(_x >= 0.0 ? _x + 0.5 : _x - 0.5);
}

inline bool same(const double _x, const double _y, const double _tol)
{
  return fabs(_x - _y) < _tol;
}
template <typename T> inline T sqr(const T& _a) { return _a * _a; }

//=============================================================================
#endif // TOOLS_HH defined
//=============================================================================

