#include <gtest/gtest.h>
#include <Unittests/unittests_common.hh>
#include <OpenMesh/Core/Utils/PropertyManager.hh>

#include <iostream>

namespace {

class OpenMeshPropertyManager : public OpenMeshBase {

    protected:

        // This function is called before each test is run
        virtual void SetUp() {
        }

        // This function is called after all tests are through
        virtual void TearDown() {

            // Do some final stuff with the member data here...
        }

    // Member already defined in OpenMeshBase
    //Mesh mesh_;  
};

/*
 * ====================================================================
 * General Tests
 * ====================================================================
 */

/*
 * Collapsing a tetrahedron
 */
TEST_F(OpenMeshPropertyManager, set_range_bool) {

  mesh_.clear();

  // Add some vertices
  Mesh::VertexHandle vhandle[4];

  vhandle[0] = mesh_.add_vertex(Mesh::Point(0, 0, 0));
  vhandle[1] = mesh_.add_vertex(Mesh::Point(0, 1, 0));
  vhandle[2] = mesh_.add_vertex(Mesh::Point(1, 1, 0));
  vhandle[3] = mesh_.add_vertex(Mesh::Point(0, 0, 1));

  // Add two faces
  std::vector<Mesh::VertexHandle> face_vhandles;

  face_vhandles.push_back(vhandle[0]);
  face_vhandles.push_back(vhandle[1]);
  face_vhandles.push_back(vhandle[2]);
  mesh_.add_face(face_vhandles);

  face_vhandles.clear();

  face_vhandles.push_back(vhandle[0]);
  face_vhandles.push_back(vhandle[2]);
  face_vhandles.push_back(vhandle[3]);
  mesh_.add_face(face_vhandles);

  {
      OpenMesh::PropertyManager<
          OpenMesh::VPropHandleT<bool>, Mesh> pm_v_bool(mesh_, "pm_v_bool");
      pm_v_bool.set_range(mesh_.vertices_begin(), mesh_.vertices_end(), false);
      for (int i = 0; i < 4; ++i)
          ASSERT_FALSE(pm_v_bool[vhandle[i]]);
      pm_v_bool.set_range(mesh_.vertices_begin(), mesh_.vertices_end(), true);
      for (int i = 0; i < 4; ++i)
          ASSERT_TRUE(pm_v_bool[vhandle[i]]);

      OpenMesh::PropertyManager<
          OpenMesh::EPropHandleT<bool>, Mesh> pm_e_bool(mesh_, "pm_e_bool");
      pm_e_bool.set_range(mesh_.edges_begin(), mesh_.edges_end(), false);
      for (Mesh::EdgeIter e_it = mesh_.edges_begin(), f_end = mesh_.edges_end();
              e_it != f_end; ++e_it)
          ASSERT_FALSE(pm_e_bool[*e_it]);
      pm_e_bool.set_range(mesh_.edges_begin(), mesh_.edges_end(), true);
      for (Mesh::EdgeIter e_it = mesh_.edges_begin(), f_end = mesh_.edges_end();
              e_it != f_end; ++e_it)
          ASSERT_TRUE(pm_e_bool[*e_it]);

      OpenMesh::PropertyManager<
          OpenMesh::FPropHandleT<bool>, Mesh> pm_f_bool(mesh_, "pm_f_bool");
      pm_f_bool.set_range(mesh_.faces_begin(), mesh_.faces_end(), false);
      for (Mesh::FaceIter f_it = mesh_.faces_begin(), f_end = mesh_.faces_end();
              f_it != f_end; ++f_it)
          ASSERT_FALSE(pm_f_bool[*f_it]);
      pm_f_bool.set_range(mesh_.faces_begin(), mesh_.faces_end(), true);
      for (Mesh::FaceIter f_it = mesh_.faces_begin(), f_end = mesh_.faces_end();
              f_it != f_end; ++f_it)
          ASSERT_TRUE(pm_f_bool[*f_it]);
  }

#if (defined(_MSC_VER) && (_MSC_VER >= 1800)) || __cplusplus > 199711L || defined(__GXX_EXPERIMENTAL_CXX0X__)
  /*
   * Same thing again, this time with C++11 ranges.
   */
  {
      OpenMesh::PropertyManager<
          OpenMesh::VPropHandleT<bool>, Mesh> pm_v_bool(mesh_, "pm_v_bool2");
      pm_v_bool.set_range(mesh_.vertices(), false);
      for (int i = 0; i < 4; ++i)
          ASSERT_FALSE(pm_v_bool[vhandle[i]]);
      pm_v_bool.set_range(mesh_.vertices(), true);
      for (int i = 0; i < 4; ++i)
          ASSERT_TRUE(pm_v_bool[vhandle[i]]);

      OpenMesh::PropertyManager<
          OpenMesh::EPropHandleT<bool>, Mesh> pm_e_bool(mesh_, "pm_e_bool2");
      pm_e_bool.set_range(mesh_.edges(), false);
      for (Mesh::EdgeIter e_it = mesh_.edges_begin(), f_end = mesh_.edges_end();
              e_it != f_end; ++e_it)
          ASSERT_FALSE(pm_e_bool[*e_it]);
      pm_e_bool.set_range(mesh_.edges(), true);
      for (Mesh::EdgeIter e_it = mesh_.edges_begin(), f_end = mesh_.edges_end();
              e_it != f_end; ++e_it)
          ASSERT_TRUE(pm_e_bool[*e_it]);

      OpenMesh::PropertyManager<
          OpenMesh::FPropHandleT<bool>, Mesh> pm_f_bool(mesh_, "pm_f_bool2");
      pm_f_bool.set_range(mesh_.faces(), false);
      for (Mesh::FaceIter f_it = mesh_.faces_begin(), f_end = mesh_.faces_end();
              f_it != f_end; ++f_it)
          ASSERT_FALSE(pm_f_bool[*f_it]);
      pm_f_bool.set_range(mesh_.faces(), true);
      for (Mesh::FaceIter f_it = mesh_.faces_begin(), f_end = mesh_.faces_end();
              f_it != f_end; ++f_it)
          ASSERT_TRUE(pm_f_bool[*f_it]);
  }
#endif
}

/*
 * ====================================================================
 * C++11 Specific Tests
 * ====================================================================
 */
#if (defined(_MSC_VER) && (_MSC_VER >= 1800)) || __cplusplus > 199711L || defined(__GXX_EXPERIMENTAL_CXX0X__)

template<typename PropHandle, typename Mesh>
bool has_property(const Mesh& _mesh, const std::string& _name) {
    auto dummy_handle = PropHandle{};
    return _mesh.get_property_handle(dummy_handle, _name);
}

/*
 * Temporary property
 */
TEST_F(OpenMeshPropertyManager, cpp11_temp_property) {
    using handle_type = OpenMesh::VPropHandleT<int>;
    const auto prop_name = "pm_v_test_property";
    ASSERT_FALSE(has_property<handle_type>(mesh_, prop_name));

    {
        auto vprop = OpenMesh::makePropertyManagerFromNew<handle_type>(mesh_, prop_name);
        static_cast<void>(vprop); // Unused variable
        ASSERT_TRUE(has_property<handle_type>(mesh_, prop_name));
    }

    ASSERT_FALSE(has_property<handle_type>(mesh_, prop_name));
}

/*
 * Two temporary properties on a mesh using the same name and type. The second
 * (inner) one shadows the first (outer) one instead of aliasing.
 */
TEST_F(OpenMeshPropertyManager, cpp11_temp_property_shadowing) {
    auto vh = mesh_.add_vertex({0,0,0}); // Dummy vertex to attach properties to

    using handle_type = OpenMesh::VPropHandleT<int>;
    const auto prop_name = "pm_v_test_property";

    auto outer_prop = OpenMesh::makePropertyManagerFromNew<handle_type>(mesh_, prop_name);
    outer_prop[vh] = 100;
    ASSERT_EQ(100, outer_prop[vh]);

    {
        // inner_prop uses same type and name as outer_prop
        auto inner_prop = OpenMesh::makePropertyManagerFromNew<handle_type>(mesh_, prop_name);
        inner_prop[vh] = 200;
        ASSERT_EQ(200, inner_prop[vh]);
        // End of scope: inner_prop is removed from mesh_
    }

    // Ensure outer_prop still exists and its data has not been overwritten by inner_prop
    ASSERT_TRUE(has_property<handle_type>(mesh_, prop_name));
    ASSERT_EQ(100, outer_prop[vh]);
}

/*
 * In sequence:
 * - add a persistent property to a mesh
 * - retrieve an existing property of a mesh and modify it
 * - obtain a non-owning property handle
 * - attempt to obtain a non-owning handle to a non-existing property (throws)
 */
TEST_F(OpenMeshPropertyManager, cpp11_persistent_and_non_owning_properties) {
    auto vh = mesh_.add_vertex({0,0,0}); // Dummy vertex to attach properties to

    using handle_type = OpenMesh::VPropHandleT<int>;
    const auto prop_name = "pm_v_test_property";

    ASSERT_FALSE(has_property<handle_type>(mesh_, prop_name));

    {
        auto prop = OpenMesh::makePropertyManagerFromExistingOrNew<handle_type>(mesh_, prop_name);
        prop[vh] = 100;
        // End of scope, property persists
    }

    ASSERT_TRUE(has_property<handle_type>(mesh_, prop_name));

    {
        // Since a property of the same name and type already exists, this refers to the existing property.
        auto prop = OpenMesh::makePropertyManagerFromExistingOrNew<handle_type>(mesh_, prop_name);
        ASSERT_EQ(100, prop[vh]);
        prop[vh] = 200;
        // End of scope, property persists
    }

    ASSERT_TRUE(has_property<handle_type>(mesh_, prop_name));

    {
        // Acquire non-owning handle to the property, knowing it exists
        auto prop = OpenMesh::makePropertyManagerFromExisting<handle_type>(mesh_, prop_name);
        ASSERT_EQ(200, prop[vh]);
    }

    ASSERT_TRUE(has_property<handle_type>(mesh_, prop_name));

    {
        // Attempt to acquire non-owning handle for a non-existing property
        ASSERT_THROW(OpenMesh::makePropertyManagerFromExisting<handle_type>(mesh_, "wrong_property_name"), std::runtime_error);
    }

    ASSERT_TRUE(has_property<handle_type>(mesh_, prop_name));
}

#endif

}
