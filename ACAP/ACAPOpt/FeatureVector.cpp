#include "FeatureVector.h"
#include <unsupported/Eigen/MatrixFunctions>
#include <cmath>
#include <iostream>

#include <cassert>


#ifdef DUSE_OPENMP
#define OMP_open __pragma(omp parallel num_threads(omp_get_num_procs()*2)) \
{ \
	__pragma(omp for)
#define OMP_end \
}
#else
#define OMP_open ;
#define OMP_end ;
#endif
using namespace std;

double RefMesh::normalScale = 0.3;
const double EPS = 1e-10;

//double RefMesh::normalScale = 0.08;
int RefMesh::root = 0;

RefMesh::~RefMesh() {
	for (int i = 0; i < align.size(); i++)
		delete align[i];
}

double cotan(Eigen::Vector3d a, Eigen::Vector3d b) {
	double na = a.norm(), nb = b.norm();
	if (na < Eps || nb < Eps) return 0;
	double cos = a.dot(b) / (na*nb);
	if (cos == 1) return 1;
	return cos / sqrt(1 - cos*cos);
}

double tan2(Eigen::Vector3d a, Eigen::Vector3d b) {
	double na = a.norm(), nb = b.norm();
	if (na < Eps || nb < Eps) return 0;
	double cos = a.dot(b) / (na*nb);
	double theta = acos(cos) / 2;
	double ans = tan(theta);
	if (ans >= 0 && ans <= 100) return ans / nb;
	return 1 / nb;
}

double SinValue(const OpenMesh::Vec3d& a, const OpenMesh::Vec3d& b, const OpenMesh::Vec3d& c)
{
	double lab = (b - a).length();
	double lac = (c - a).length();
	return (OpenMesh::cross(b - a, c - a)).length() / (lab*lac);
}


double CosValue(const OpenMesh::Vec3d& a, const OpenMesh::Vec3d& b, const OpenMesh::Vec3d& c)
{
	double lab = (b - a).length();
	double lac = (c - a).length();
	double lab2 = (b - a).sqrnorm();
	double lac2 = (c - a).sqrnorm();
	double lbc2 = (b - c).sqrnorm();
	return (lab2 + lac2 - lbc2) / (2.0*lab*lac);
}

double sexp(double x) {
	if (x <= 0) return exp(x);
	return 1 + x;
}

RefMesh::RefMesh()
{
	usec = false, fvNum = 0;
	vvs = 0;
	mesh = NULL;
}

RefMesh::RefMesh(DTriMesh &ms) : mesh(&ms) {
	usec = false, fvNum = 0;
	//// Add vertex normals as default property (ref. previous tutorial)
	ms.request_vertex_normals();
	//// Add face normals as default property
	ms.request_face_normals();

	ms.update_normals();
	//ms.release_face_normals();
	align.resize(mesh->n_vertices());
	d.resize(mesh->n_vertices());
	rd.resize(mesh->n_vertices());
	this->degree.resize(mesh->n_vertices(), 0);
	vvs = 0;
	for (int i = 0; i < mesh->n_vertices(); i++) {
		DTriMesh::VertexHandle vi(i);
		DTriMesh::Point p = mesh->point(vi);
		std::vector<Eigen::Vector3d> vec;
		d[i] = vvs;
		double lens = 0;
		for (DTriMesh::VertexVertexIter vvi = mesh->vv_iter(vi); vvi.is_valid(); ++vvi) {
			rd[vvi->idx()].push_back(std::make_pair(i, vvs));
			Eigen::Vector3d q = OtoE(mesh->point(DTriMesh::VertexHandle(vvi->idx())) - p);
			vec.push_back(q);
			vvs++;
			lens += q.norm();
			degree[i]++;
		}
		vvs = d[i];

		OpenMesh::Vec3d currentPosition = mesh->point(vi);
		vector<OpenMesh::Vec3d> neighborPoints;
		for (DTriMesh::VertexVertexIter vv_it = mesh->vv_iter(vi); vv_it.is_valid(); ++vv_it)
		{
			neighborPoints.push_back(mesh->point(DTriMesh::VertexHandle(vv_it->idx())));
		}
		int numNeighbors = (int)neighborPoints.size();
		vector<double> weights(numNeighbors, 0);
		//if (i==2409)
		//{
		//	int a=1;
		//}
		//std::cout<<i;
		for (int neighborcounts = 0; neighborcounts < numNeighbors; neighborcounts++)
		{
			//precomput pi - pj of reference Mesh (yangjie add)
			OpenMesh::Vec3d edgejk_openmesh = p - neighborPoints[neighborcounts];
			Eigen::Vector3d edgejk(edgejk_openmesh[0], edgejk_openmesh[1], edgejk_openmesh[2]);
			this->edgeLength.push_back(edgejk);

			//double w1 = CosValue(neighborPoints[(neighborcounts + numNeighbors -1)%numNeighbors], currentPosition, neighborPoints[neighborcounts])/std::max(SinValue(neighborPoints[(neighborcounts + numNeighbors -1)%numNeighbors], currentPosition, neighborPoints[neighborcounts]), EPS);
			//double w2 = CosValue(neighborPoints[(neighborcounts+1)%numNeighbors], currentPosition, neighborPoints[neighborcounts])/std::max(SinValue(neighborPoints[(neighborcounts+1)%numNeighbors], currentPosition, neighborPoints[neighborcounts]), EPS);
			double w1 = cotan(OtoE(currentPosition - neighborPoints[(neighborcounts + numNeighbors - 1) % numNeighbors]), OtoE(neighborPoints[neighborcounts] - neighborPoints[(neighborcounts + numNeighbors - 1) % numNeighbors]));
			double w2 = cotan(OtoE(currentPosition - neighborPoints[(neighborcounts + 1) % numNeighbors]), OtoE(neighborPoints[neighborcounts] - neighborPoints[(neighborcounts + 1) % numNeighbors]));
			weights[neighborcounts] = 0.5*(w1 + w2);

			weights[neighborcounts] = sqrt(sexp(weights[neighborcounts]));

			if ((weights[neighborcounts] != weights[neighborcounts]) || weights[neighborcounts] > 100000)
			{
				//cout<<v_it.handle().idx()<<" "<<weights[i]<<endl;
				//system("pause");
				weights[neighborcounts] = 1;
			}

#ifdef uniformweight
			weights[neighborcounts] = 1;
#endif
			w.push_back(weights[neighborcounts]);
			vvs++;
		}
		//std::cout<<i;


		assert(vec.size() == (w.size() - d[i]));
		for (int ii = 0; ii < vec.size(); ii++)
		{
			vec[ii] = sqrt(w[ii + d[i]])*vec[ii];
			//assert(vec[ii] == vec[ii]);
		}

		// add normal
		if (mesh->normal(vi) != mesh->normal(vi)) {
			std::cout << mesh->normal(vi) << " " << i << " " << w[i];
		}
		//assert(mesh->normal(vi) == mesh->normal(vi));
		vec.push_back(OtoE(mesh->normal(vi) * (lens / vec.size() * RefMesh::normalScale)));
		align[i] = new AffineAlign(vec);
	}
	c.resize(w.size(), 0);
	std::vector<bool> visit(mesh->n_vertices(), false);
	int qBeg = 0;
	for (int i = root; i < visit.size(); i++) if (!visit[i]) {
		visit[i] = true;
		bfsq.push_back(make_pair(i, make_pair(-1, -1)));
		while (qBeg < bfsq.size()) {
			int i = bfsq[qBeg++].first;

			DTriMesh::VertexHandle vi(i);
			int vvs = d[i];

			for (DTriMesh::VertexVertexIter vvi = mesh->vv_iter(vi); vvi.is_valid(); ++vvi, vvs++) {
				int j = vvi->idx();
				if (!visit[j]) {
					visit[j] = true;
					bfsq.push_back(make_pair(j, make_pair(i, vvs)));
				}
			}
		}
	}
}

void RefMesh::getFeature(DTriMesh &ms, FeatureVector &fv) {

	static int callTime = -1;
	callTime++;
	ms.update_face_normals();
	ms.update_vertex_normals();

	fv.s.resize(align.size());
	fv.r.resize(align.size());
	fv.logr.resize(align.size());


	double allres = 0;

	for (int i = 0; i < ms.n_vertices(); i++) {
		DTriMesh::VertexHandle vi(i);
		DTriMesh::Point p = ms.point(vi);
		//DTriMesh::Point haha= ms.point(vi)-mesh->point(vi);
		std::vector<Eigen::Vector3d> vec;
		double lens = 0;
		for (DTriMesh::VertexVertexIter vvi = ms.vv_iter(vi); vvi.is_valid(); vvi++) {
			Eigen::Vector3d q = OtoE(ms.point(DTriMesh::VertexHandle(vvi->idx())) - p);
			vec.push_back(q);
			lens += q.norm();
		}

		for (int ii = 0; ii < vec.size(); ii++)
		{
			vec[ii] = vec[ii] * sqrt(w[ii + d[i]]);
		}

		// add normal
		vec.push_back(OtoE(ms.normal(vi) * (lens / vec.size() * RefMesh::normalScale)));
		if (vec.size() <= 1) {
			fv.s[i] = fv.r[i] = Eigen::Matrix3d::Identity();
			continue;
		}
		Eigen::Matrix3d mat = align[i]->calc(vec);
		polarDec(mat, fv.r[i], fv.s[i]);
		allres += (align[i]->residualwithoutnormal(mat, vec));

		fv.rots[i].r = fv.r[i];
		fv.logr[i] = log(fv.r[i]);
		fv.rots[i].logr = fv.logr[i];



	}
	//assert(allres == allres);
	cout << callTime << ": " << allres << std::endl;


	fv.dr.resize(vvs);
	fv.logdr.resize(vvs);

	if (usec)
		this->fvNum++;

	for (int i = 0; i < ms.n_vertices(); i++) {
		DTriMesh::VertexHandle vi(i);
		int vs = d[i];
		for (DTriMesh::VertexVertexIter vvi = ms.vv_iter(vi); vvi.is_valid(); vvi++) {
			fv.dr[vs] = fv.r[i].transpose() * fv.r[vvi->idx()];
			fv.logdr[vs] = log(fv.dr[vs]);
			if (usec)
				c[vs] = (c[vs] * (this->fvNum - 1) + fv.logdr[vs].norm() + (fv.s[i] - Eigen::Matrix3d::Identity()).norm()) / this->fvNum;
			vs++;
		}
	}

}


void FeatureVector::IdentityRotation()
{
	for (int i = 0; i < r.size(); i++)
	{
		r[i] = Eigen::Matrix3d::Identity();
	}
}



double RefMesh::getWij(int e) {
	//if (fvNum) return exp(-c[e]);// * w[e];
	return w[e];
}


void FeatureVector::resize(const FeatureVector &other) {
	s.resize(other.s.size());
	logdr.resize(other.logdr.size());
	logr.resize(other.logr.size());
}



double dot(const Eigen::Matrix3d &a, const Eigen::Matrix3d &b) {
	return
		a(0, 0) * b(0, 0) + a(0, 1) * b(0, 1) + a(0, 2) * b(0, 2) +
		a(1, 0) * b(1, 0) + a(1, 1) * b(1, 1) + a(1, 2) * b(1, 2) +
		a(2, 0) * b(2, 0) + a(2, 1) * b(2, 1) + a(2, 2) * b(2, 2);
}

//yangjie add

FeatureVector::FeatureVector(DTriMesh & mesh)
{
	this->r.resize(mesh.n_vertices(), Eigen::Matrix3d::Identity());
	this->s.resize(mesh.n_vertices(), Eigen::Matrix3d::Identity());
	this->t.resize(mesh.n_vertices(), Eigen::Matrix3d::Identity());
	this->t_new.resize(mesh.n_vertices(), Eigen::Matrix3d::Identity());
	this->logr.resize(mesh.n_vertices(), Eigen::Matrix3d::Zero());
	this->dr.resize(mesh.n_edges() * 2, Eigen::Matrix3d::Identity());
	this->logdr.resize(mesh.n_edges() * 2, Eigen::Matrix3d::Zero());
	this->rots.resize(mesh.n_vertices(), Rot::Rot());
	//this->rot_vertices.resize(mesh.n_vertices());
	this->isrot.resize(mesh.n_vertices());
	this->logr_plus_s.resize(mesh.n_vertices());
}

void FeatureVector::resize(DTriMesh & mesh)
{
	this->r.resize(mesh.n_vertices(), Eigen::Matrix3d::Identity());
	this->s.resize(mesh.n_vertices(), Eigen::Matrix3d::Identity());
	this->t.resize(mesh.n_vertices(), Eigen::Matrix3d::Identity());
	this->t_new.resize(mesh.n_vertices(), Eigen::Matrix3d::Identity());
	this->logr.resize(mesh.n_vertices(), Eigen::Matrix3d::Zero());
	this->dr.resize(mesh.n_edges() * 2, Eigen::Matrix3d::Identity());
	this->logdr.resize(mesh.n_edges() * 2, Eigen::Matrix3d::Zero());
	this->rots.resize(mesh.n_vertices(), Rot::Rot());
	//this->rot_vertices.resize(mesh.n_vertices());
	this->isrot.resize(mesh.n_vertices());
	this->logr_plus_s.resize(mesh.n_vertices());
}
