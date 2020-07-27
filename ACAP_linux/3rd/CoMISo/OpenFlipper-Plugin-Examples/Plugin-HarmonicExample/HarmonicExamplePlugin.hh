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
//  CLASS HarmonicExamplePlugin
//
//=============================================================================


#ifndef HARMONICEXAMPLEPLUGIN_HH
#define HARMONICEXAMPLEPLUGIN_HH


//== INCLUDES =================================================================

#include <QObject>
#include <QMenuBar>
#include <QSpinBox>

#include <OpenFlipper/common/Types.hh>
#include <OpenFlipper/BasePlugin/BaseInterface.hh>
#include <OpenFlipper/BasePlugin/ToolboxInterface.hh>
#include <OpenFlipper/BasePlugin/KeyInterface.hh>
#include <OpenFlipper/BasePlugin/MouseInterface.hh>
#include <OpenFlipper/BasePlugin/PickingInterface.hh>
#include <OpenFlipper/BasePlugin/ScriptInterface.hh>

#include <ACG/QtWidgets/QtExaminerViewer.hh>

#include <ObjectTypes/TriangleMesh/TriangleMesh.hh>

#include "HarmonicExampleToolbar.hh"
#include "HarmonicExamplePerObjectDataT.hh"
#include "HarmonicExampleT.hh"


//== CLASS DEFINITION =========================================================


class HarmonicExamplePlugin : public QObject, BaseInterface, ToolboxInterface, KeyInterface, ScriptInterface, MouseInterface, PickingInterface
{
  Q_OBJECT
  Q_INTERFACES(BaseInterface)
  Q_INTERFACES(ToolboxInterface)
  Q_INTERFACES(KeyInterface)
  Q_INTERFACES(ScriptInterface)
  Q_INTERFACES(MouseInterface)
  Q_INTERFACES(PickingInterface)


  // typedef for easy access
  typedef COMISO::HarmonicExampleT<TriMesh> HarmonicExample;
  typedef HarmonicExamplePerObjectDataT<TriMesh>   POD;

signals:
  void updateView();
  void updatedObject(int , const UpdateType );
  void addToolbox( QString _name  , QWidget* _widget );  


private slots:

  // initialization functions
  void initializePlugin();
  void pluginsInitialized();


  // compute
  void slotCompute();


public :

  ~HarmonicExamplePlugin() {};


  QString name() { return (QString("HarmonicExample")); };
  QString description( ) { return (QString("Computes the HarmonicExample of the the active Mesh")); }; 

private :

  // return name of per object data
  const char * pod_name() { return "HARMONICEXAMPLE_PER_OBJECT_DATA";}

  // get HarmonicExample object for a given object
  HarmonicExample* get_harmonicexample_object( BaseObjectData* _object );
  


private :
  /// Widget for Toolbox
  HarmonicExampleToolbar* tool_;
};


//=============================================================================
#endif // HARMONICEXAMPLEPLUGIN_HH defined
//=============================================================================

