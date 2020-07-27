//=============================================================================
//
//                               OpenFlipper
//        Copyright (C) 2008 by Computer Graphics Group, RWTH Aachen
//                           www.openflipper.org
//
//-----------------------------------------------------------------------------
//
//                                License
//
//  OpenFlipper is free software: you can redistribute it and/or modify
//  it under the terms of the GNU Lesser General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  OpenFlipper is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public License
//  along with OpenFlipper.  If not, see <http://www.gnu.org/licenses/>.
//
//-----------------------------------------------------------------------------
//
//   $Revision: 3468 $
//   $Author: moebius $
//   $Date: 2008-10-17 14:58:52 +0200 (Fri, 17 Oct 2008) $
//
//=============================================================================

// (C) Copyright 2015 by Autodesk, Inc.
//
// The information contained herein is confidential, proprietary
// to Autodesk,  Inc.,  and considered a trade secret as defined
// in section 499C of the penal code of the State of California.
// Use of  this information  by  anyone  other  than  authorized
// employees of Autodesk, Inc.  is granted  only under a written
// non-disclosure agreement,  expressly  prescribing  the  scope
// and manner of such use.

//=============================================================================
//
//  CLASS StopWatch
//
//=============================================================================

#ifndef COMISO_STOPWATCH_HH_INCLUDED
#define COMISO_STOPWATCH_HH_INCLUDED
//=============================================================================
// This header is now a shortcut to Base::StopWatch
#include <Base/Utils/StopWatch.hh>

namespace COMISO {

  typedef Base::StopWatch StopWatch;

}

// We can expose Base::StopWatch as COMISO::StopWatch if needed

#endif//COMISO_STOPWATCH_HH_INCLUDED
//=============================================================================
