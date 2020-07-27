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
//  CLASS HarmonicExampleT - IMPLEMENTATION
//
//=============================================================================

#define COMISO_HARMONICEXAMPLET_C

//== INCLUDES =================================================================

#include "HarmonicExampleT.hh"

//== NAMESPACES ===============================================================

namespace COMISO {

//== IMPLEMENTATION ==========================================================

template<class MeshT>
void
HarmonicExampleT<MeshT>::
compute()
{
  // setup simple laplace system
  init_laplace_matrix();
  COMISO_GMM::inspect_matrix( laplace_);

  // add the integer constraints for the selected vertices
  add_integer_constraints();

  // init constraints matrix (assume we have at most n_vertices constraints)
  gmm::resize( constraints_, mesh_.n_vertices(), mesh_.n_vertices()+1);

  // add boundary constraints
  gmm::clear(constraints_);
  int nconstraints = add_boundary_constraints();
  gmm::resize( constraints_, nconstraints, mesh_.n_vertices()+1);

  // solve 
  x_.clear();
  x_.resize( mesh_.n_vertices(), 0.0);
  rhs_.clear();
  rhs_.resize( mesh_.n_vertices(), 0.0);

  COMISO::ConstrainedSolver cs;
  cs.solve( constraints_, laplace_, x_, rhs_, ids_to_round_, 0.0, true, true);

  // color the vertices of the mesh
  color_mesh();
}


//-----------------------------------------------------------------------------


template<class MeshT>
void
HarmonicExampleT<MeshT>::
init_laplace_matrix()
{
  int n = mesh_.n_vertices();

  gmm::clear( laplace_);
  gmm::resize( laplace_, n, n);

  int row = 0;
  for( VIter v_it = mesh_.vertices_begin(); v_it != mesh_.vertices_end(); ++v_it)
  {
    row = add_laplace_row( v_it.handle(), row);
  }
}


//-----------------------------------------------------------------------------


template<class MeshT>
int 
HarmonicExampleT<MeshT>::
add_laplace_row( VH _vh, int _row)
{
  //// use edge-length "laplacian"
  //Point p( mesh_.point( _vh));
  //Point q(0,0,0);
  //double length = 0.0;
  
  // use simple topological laplacian
  for( VOHIter voh_it = mesh_.voh_iter( _vh); voh_it; ++voh_it)
  {
    VH  neigh_vh = mesh_.to_vertex_handle(voh_it.handle());
    //q = mesh_.point( neigh_vh);
    int neigh_id = neigh_vh.idx();
    laplace_( _row, neigh_id) = -1.0;
    //double edge_length = (p-q).norm();
    //length += edge_length;
    //laplace_( _row, neigh_id) = edge_length;
  }

  laplace_( _row, _vh.idx()) = 1.0*mesh_.valence( _vh);
  //laplace_( _row, _vh.idx()) = 1.0*length;
  return ++_row;
}


//-----------------------------------------------------------------------------


template<class MeshT>
void
HarmonicExampleT<MeshT>::
add_integer_constraints()
{
  ids_to_round_.clear();
  for( VIter v_it = mesh_.vertices_begin(); v_it != mesh_.vertices_end(); ++v_it)
  {
    if( mesh_.status( v_it.handle()).selected())
    {
      ids_to_round_.push_back( v_it.handle().idx());
    }
  }
}


//-----------------------------------------------------------------------------


template<class MeshT>
int 
HarmonicExampleT<MeshT>::
add_boundary_constraints()
{
  std::vector< int > boundary( mesh_.n_edges(), -1);
  int boundaries = 0;
  int constraints = 0;

  // vertex indices of current boundary and last boundary...
  int first_vidx(-1), last_bdry_vidx(-1);

  // find boundaries by for every (non marked) boundary edge,
  // traverse and mark the whole boundary
  for ( EIter e_it = mesh_.edges_begin(); e_it != mesh_.edges_end(); ++e_it )
  {
    if ( mesh_.is_boundary( e_it.handle() ) && boundary[e_it.handle().idx()] == -1)
    {
      boundaries++;

      // get boundary halfedge
      HEH heh_bound( mesh_.halfedge_handle( e_it, 0 ) );
      if ( !mesh_.is_boundary( heh_bound ) )
        heh_bound = mesh_.halfedge_handle( e_it, 1 );

      // fix underdetermindness of laplace system by constraining first boundary to be zero
      if ( first_vidx == -1)
      {
        first_vidx = mesh_.to_vertex_handle( heh_bound).idx();
        constraints_(constraints, first_vidx) = 1;
        constraints++;
      }
 
      // add constraints saying that all vertices on boundary have the same value
      // do this by traversing the boundary and setting all vertices equal to the 
      // first vertex of the boundary
      first_vidx = mesh_.to_vertex_handle( heh_bound).idx();
      int cur_vidx = -1;

      while ( boundary[mesh_.edge_handle( heh_bound ).idx()] == -1 )
      {
        // tag edge as boundary
        boundary[mesh_.edge_handle( heh_bound ).idx()] = boundaries;
        heh_bound = mesh_.next_halfedge_handle( heh_bound );
        // add equality condition for vertices differnt from the first_vidx
        if( cur_vidx != -1)
        {
          constraints_(constraints, first_vidx) = -1;
          constraints_(constraints, cur_vidx)   = 1;
          constraints++;
        }
        cur_vidx = mesh_.to_vertex_handle( heh_bound).idx();
      }

      // just for fun, also relate the boundaries to each other with a difference of 5
      if( last_bdry_vidx != -1)
      {
        constraints_(constraints, first_vidx) = -1;
        constraints_(constraints, last_bdry_vidx) = 1;
        constraints_(constraints, mesh_.n_vertices()) = 4;
        constraints++;
      }
      last_bdry_vidx = first_vidx;
    }
  }

  if( boundaries == 0) // add 1 and zero to marked vertices
  {
    for( VIter v_it = mesh_.vertices_begin(); v_it != mesh_.vertices_end(); ++v_it)
    {
      if( mesh_.status(v_it).selected())
      {
        std::cerr << " setting selected point to " << constraints << std::endl;
        constraints_(constraints, v_it.handle().idx()) = 1.0;
        constraints_(constraints, mesh_.n_vertices()) = constraints;
        constraints++;
      }
    }
  }
    
  return constraints;
}

//-----------------------------------------------------------------------------

template<class MeshT>
void
HarmonicExampleT<MeshT>::
color_mesh()
{
  //double vmin = 0.0;
  //double vmax = 1.0;
  double vmin = (double)*std::min_element( x_.begin(), x_.end());
  double vmax = (double)*std::max_element( x_.begin(), x_.end());

  std::cerr << "vmin " << vmin << " vmax " << vmax << std::endl;
  // create ColorCoder
  COMISO::ColorCoder cc( vmin, vmax, false);

  VIter v_it  = mesh_.vertices_begin();
  VIter v_end = mesh_.vertices_end();
  for(; v_it != v_end; ++v_it)
  {
    Scalar v_cur = x_[v_it.handle().idx()];
    
    // clamp
    v_cur = std::min(v_cur, vmax);
    v_cur = std::max(v_cur, vmin);

    //double int_part = floor(v_cur);
    //v_cur = v_cur - int_part;

    // set vertex color
    mesh_.set_color(v_it, cc.color_floata(v_cur));
  }
}


//=============================================================================
} // namespace COMISO
//=============================================================================
