#ifndef new_DFEATURE_VECTOR_H
#define new_DFEATURE_VECTOR_H

#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Core/Geometry/Vector11T.hh>
#include <OpenMesh/Core/IO/reader/OBJReader.hh>
#include <OpenMesh/Core/IO/writer/OBJWriter.hh>

#include <vector>
#include "Align.h"

#include <set>


class FeatureVector;

class RefMesh {
public:
	int vvs;
	DTriMesh *mesh;
	std::vector<AffineAlign*> align;
	std::vector<int> d;
	std::vector< std::vector< std::pair<int, int> > > rd;
	std::vector<double> w, c;
	std::vector< std::pair<int, std::pair<int, int> > > bfsq;
	std::vector<Eigen::Vector3d> edgeLength;
	std::vector<int> degree;
	bool usec;
	int fvNum;
	~RefMesh();

	RefMesh();
	RefMesh(DTriMesh &ms);

	void getFeature(DTriMesh &ms, FeatureVector &fv);

	double getWij(int e);

	static double normalScale;
	static int root;
};

class FeatureVector {
public:
	std::vector<Eigen::Matrix3d> s, r, dr, logdr, logr, t, logr_plus_s, t_new;
	std::vector<Rot> rots;

	FeatureVector() {}
	FeatureVector(std::vector<double> weight, std::vector<FeatureVector> &fvs);

	void resize(const FeatureVector &other);
	void IdentityRotation();

	//yangjie add
	std::set<int> rot_vertices;
	std::vector<bool> isrot;
	FeatureVector(DTriMesh & mesh);

	//resize
	void resize(DTriMesh &mesh);

};

#endif
