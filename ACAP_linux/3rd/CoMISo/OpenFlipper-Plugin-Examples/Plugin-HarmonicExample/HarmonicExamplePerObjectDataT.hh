/*===========================================================================*\
 *                                                                           *
 *                          Plugin-HarmonicExample                           *
 *      Copyright (C) 2008-2009 by Computer Graphics Group, RWTH Aachen      *
 *                           www.rwth-graphics.de                            *
 *                                                                           *
 *---------------------------------------------------------------------------* 
 *  This file is part of Plugin-HarmonicExample                              *
 *                                                                           *
 *  Plugin-HarmonicExample is free software: you can redistribute it and/or  *
 *  modify it under the terms of the GNU General Public License as published *
 *  by the Free Software Foundation, either version 3 of the License, or     *
 *  (at your option) any later version.                                      *
 *                                                                           *
 *  Plugin-HarmonicExample is distributed in the hope that it will be useful,*
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
 *  GNU General Public License for more details.                             *
 *                                                                           *
 *  You should have received a copy of the GNU General Public License        *
 *  along with Plugin-HarmonicExample.                                       *
 *  If not, see <http://www.gnu.org/licenses/>.                              *
 *                                                                           *
\*===========================================================================*/ 

//=============================================================================
//
// CLASS HarmonicExamplePerObjectData
//
//=============================================================================


#ifndef HARMONICEXAMPLEPEROBJECTDATA_HH
#define HARMONICEXAMPLEPEROBJECTDATA_HH

//== INCLUDES =================================================================


#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>
#include <OpenFlipper/common/perObjectData.hh>
#include "HarmonicExampleT.hh"


//== CLASS DEFINITION =========================================================


template <class MeshT>
class HarmonicExamplePerObjectDataT : public PerObjectData
{
public:

  HarmonicExamplePerObjectDataT( MeshT& _mesh) : harmonicexample_(_mesh) 
  {}
  
  virtual
  ~HarmonicExamplePerObjectDataT() 
  {}
  
  COMISO::HarmonicExampleT<MeshT>& harmonicexample() { return harmonicexample_;}

private:
  // create an FeatureLine
  COMISO::HarmonicExampleT<MeshT> harmonicexample_;
};


//=============================================================================
#endif // HARMONICEXAMPLEPEROBJECTDATA_HH defined
//=============================================================================

