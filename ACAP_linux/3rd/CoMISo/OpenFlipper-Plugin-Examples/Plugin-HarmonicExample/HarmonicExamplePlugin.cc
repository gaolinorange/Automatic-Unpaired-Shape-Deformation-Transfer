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
//  CLASS HarmonicExamplePlugin - IMPLEMENTATION
//
//=============================================================================

//== INCLUDES =================================================================

#include <Qt>
#include <QtGui>
#include <QSpacerItem>

#include "HarmonicExamplePlugin.hh"

#include <iostream>
#include <CoMISo/Utils/StopWatch.hh>

#include <OpenFlipper/BasePlugin/PluginFunctions.hh>

// #include <PhySim/QtWidgets/QwtFunctionPlot.hh>
// #include <PhySim/Meshes/PrincipalCurvatureT.hh>
// #include <PhySim/Meshes/PrincipalCurvatureJetT.hh>
// #include <PhySim/Math/Algorithms.hh>


#undef min
#undef max


//== IMPLEMENTATION ==========================================================


void HarmonicExamplePlugin::initializePlugin()
{
  tool_ = new HarmonicExampleToolbar();

  QSize size(300,300);
  tool_->resize(size);
  
  // connect button press event to function slotCompute()
  connect(tool_->compute_pb, SIGNAL( clicked() ), this, SLOT( slotCompute() ) );

  emit addToolbox( tr("HarmonicExample") , tool_ );
}


//-----------------------------------------------------------------------------


void HarmonicExamplePlugin::pluginsInitialized() 
{
//   emit addPickMode("Separator");
}


//-----------------------------------------------------------------------------


HarmonicExamplePlugin::HarmonicExample*
HarmonicExamplePlugin::
get_harmonicexample_object( BaseObjectData* _object )
{
  // initialize PerObjectData if not done yet
  if (!_object->hasObjectData(pod_name()))
  {
    // get mesh object
    TriMesh* mesh = dynamic_cast< TriMeshObject* >( _object )->mesh();

    // initialize per object data
    _object->setObjectData(pod_name(), new POD(*mesh));
  }
  
  // get feature lines object
  HarmonicExample* harmonicexample = dynamic_cast<HarmonicExample*>(& (dynamic_cast< POD* >(_object->objectData(pod_name() )))->harmonicexample());

  return harmonicexample;
}


//-----------------------------------------------------------------------------


void HarmonicExamplePlugin::slotCompute() 
{
  // iterate over all target triangle meshes
  for ( PluginFunctions::ObjectIterator o_it(PluginFunctions::TARGET_OBJECTS,DATA_TRIANGLE_MESH) ; o_it != PluginFunctions::objectsEnd(); ++o_it) 
  {
    // get pointer to mesh
    TriMesh* mesh = dynamic_cast< TriMeshObject* >( *o_it )->mesh();
    if ( mesh )
    {
      // get feature line object and call compute function
      get_harmonicexample_object( *o_it )->compute();
      
      // set correct draw mode
      TriMeshObject  *mesh_obj;
      mesh_obj = PluginFunctions::triMeshObject((*o_it));
      mesh_obj->meshNode()->drawMode(ACG::SceneGraph::DrawModes::SOLID_POINTS_COLORED);
      mesh_obj->materialNode()->disable_color_material();
    }
    emit updatedObject( o_it->id(), UPDATE_COLOR);
  }
}


//-----------------------------------------------------------------------------


Q_EXPORT_PLUGIN2( harmonicexampleplugin , HarmonicExamplePlugin );

