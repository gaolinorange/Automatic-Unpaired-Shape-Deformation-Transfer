#include "Align.h"

#include <iostream>

#include <Eigen/SVD>
#include <cmath>


int flag = 0;
using namespace Eigen;
using namespace std;



Eigen::Matrix3d exp(Eigen::Matrix3d x) {
	//return x.exp();
	double theta = sqrt(x(0, 1)*x(0, 1) + x(0, 2)*x(0, 2) + x(1, 2)*x(1, 2));
	if (abs(theta) == 0) return Eigen::Matrix3d::Identity();
	x /= theta;
	return Eigen::Matrix3d::Identity() +
		x * sin(theta) +
		x * x * (1 - cos(theta));
}



Eigen::Matrix3d log(Eigen::Matrix3d x) {
	//return x.log();
	double theta = (x.trace() - 1) / 2;
	theta = acos(max(-1.0, min(1.0, theta)));
	if (abs(theta) == 0) return Eigen::Matrix3d::Zero();
	return (theta / (2 * sin(theta))) * (x - x.transpose());
}

double polarDec(const Eigen::Matrix3d &a, Eigen::Matrix3d &r, Eigen::Matrix3d &s) {
	JacobiSVD<Eigen::MatrixXd> svd(a, ComputeThinU | ComputeThinV);
	r = svd.matrixU() * svd.matrixV().transpose();
	s = svd.matrixV() * svd.singularValues().asDiagonal() * svd.matrixV().transpose();

	if (r.determinant() < 0) {
		Vector3d sv = svd.singularValues();
		int minsv = 0;
		r = svd.matrixU();
		if (sv(1) < sv(minsv)) minsv = 1;
		if (sv(2) < sv(minsv)) minsv = 2;
		if (sv(minsv) < -Eps) {
			cerr << "polar dec Error, min singular values <= 0 :" << endl;
			cerr << a << endl;
		}
		//cout << "Min SV " << sv(minsv) << " " << minsv << endl;
		r.col(minsv) *= -1;
		sv(minsv) *= -1;
		//cout << "R :\n" << r << endl;
		r = r * svd.matrixV().transpose();
		s = svd.matrixV() * sv.asDiagonal() * svd.matrixV().transpose();
		return sv.sum();
	}
	return svd.singularValues().sum();
}



AffineAlign::AffineAlign(std::vector<Eigen::Vector3d> &v) : p(v), AtA(Matrix3d::Zero()) {
	//if (flag) fout << p.size() << endl;
	for (int i = 0; i < p.size(); i++) {
		//assert(p[i] == p[i]);
		AtA += p[i] * p[i].transpose();
		//if (flag) fout << p[i] << endl << endl;
	}
	if (flag) {
		//fout << "AtA" << endl;
		//fout << AtA << endl;
		//fout << AtA.determinant() << endl;
	}
	AtA = AtA.inverse().eval();
}

Matrix3d AffineAlign::calc(const std::vector<Vector3d> &v) {
	if (v.size() != p.size()) {
		cout << "!!Error v.size() != p.size()" << endl;
		//fout << "!!Error v.size() != p.size()" << endl;
	}
	Vector3d vx(Vector3d::Zero()), vy(Vector3d::Zero()), vz(Vector3d::Zero());
	for (int i = 0; i < p.size(); i++) {
		vx += p[i] * v[i](0);
		vy += p[i] * v[i](1);
		vz += p[i] * v[i](2);
	}
	//assert(vx == vx);
	vx = AtA * vx;
	vy = AtA * vy;
	vz = AtA * vz;
	Matrix3d res;
	res << vx, vy, vz;
	return res.transpose();
}

double AffineAlign::residualwithoutnormal(Eigen::Matrix3d m, std::vector<Eigen::Vector3d> v) {
	double rs = 0;
	for (int i = 0; i < v.size() - 1; i++)
		rs += (v[i] - m*p[i]).squaredNorm();
	return rs;
}


OpenMesh::Vec3d EtoO(const Eigen::Vector3d &v) {
	return OpenMesh::Vec3d(v(0), v(1), v(2));
}

Eigen::Vector3d OtoE(const OpenMesh::Vec3d &v) {
	return Eigen::Vector3d(v[0], v[1], v[2]);
}

void Rot::ToLogR()
{
	double the = circlek * 2 * M_PI + theta;
	//the = theta;
	logr = Eigen::Matrix3d::Zero();
	logr(0, 1) = -axis(2);
	logr(0, 2) = axis(1);
	logr(1, 2) = -axis(0);
	Eigen::Matrix3d logr1 = logr.transpose();
	logr = the*(logr - logr1);
}

double Rot::ToAngle()
{
	double the = circlek * 2 * M_PI + theta;
	return the;
}


Rot logrot(Eigen::Matrix3d jrotation)
{
	Rot jrot;
	double theta = (jrotation.trace() - 1) / 2;
	theta = acos(max(-1.0, min(1.0, theta)));
	if (abs(theta) <= 1e-6)
	{
		jrot.circlek = 0;
		jrot.theta = 0;
		jrot.axis = Eigen::Vector3d::Zero();
	}
	else
	{
		jrot.theta = theta;
		Eigen::Matrix3d tmp = (1 / (2 * sin(theta))) * (jrotation - jrotation.transpose());
		Eigen::Vector3d jaxis;
		jaxis(0) = tmp(2, 1);
		jaxis(1) = tmp(0, 2);
		jaxis(2) = tmp(1, 0);
		jrot.axis = jaxis / jaxis.norm();
	}
	jrot.ToLogR();
	return jrot;
}

void logrot(Rot & jrot ,Eigen::Matrix3d & jrotation)
{
	double theta = (jrotation.trace() - 1) / 2;
	theta = acos(max(-1.0, min(1.0, theta)));
	if (abs(theta) <= 1e-6)
	{
		jrot.circlek = 0;
		jrot.theta = 0;
		jrot.axis = Eigen::Vector3d::Zero();
	}
	else
	{
		jrot.theta = theta;
		Eigen::Matrix3d tmp = (1 / (2 * sin(theta))) * (jrotation - jrotation.transpose());
		Eigen::Vector3d jaxis;
		jaxis(0) = tmp(2, 1);
		jaxis(1) = tmp(0, 2);
		jaxis(2) = tmp(1, 0);
		jrot.axis = jaxis / jaxis.norm();
	}
	//jrot.ToLogR();	
}


Rot logrot(Eigen::Matrix3d jrotation, Rot irot)
{

	Rot jrot;
	double theta = (jrotation.trace() - 1) / 2;
	theta = acos(max(-1.0, min(1.0, theta)));
	if (abs(theta) <=1e-6)
	//if (abs(sin(theta))<0.0001||abs(theta) <= 0.15)// cylinder ר��
	{
		jrot = irot;
		jrot.theta = 0;
	}
	else
	{
		Eigen::Vector3d jaxis;
		Eigen::Matrix3d tmp = Eigen::Matrix3d::Zero();
		if (abs(theta - M_PI) <= 1e-6)
		{
			jaxis = irot.axis;
			//jrot.theta = M_PI;
			tmp(0, 1) = -jaxis(2);
			tmp(0, 2) = jaxis(1);
			tmp(1, 2) = -jaxis(0);
			Eigen::Matrix3d tmp2 = tmp.transpose();
			tmp = (tmp - tmp2);
		}
		else
		{
			tmp = (1 / (2 * sin(theta))) * (jrotation - jrotation.transpose());

			jaxis(0) = tmp(2, 1);
			jaxis(1) = tmp(0, 2);
			jaxis(2) = tmp(1, 0);
		}

		//debug
		Eigen::Matrix3d logr = Eigen::Matrix3d::Zero();
		logr(0, 1) = -jaxis(2);
		logr(0, 2) = jaxis(1);
		logr(1, 2) = -jaxis(0);
		Eigen::Matrix3d tmp1 = logr.transpose();
		logr = (logr - tmp1);
		double _norm1 = (tmp - logr).squaredNorm();
		if (_norm1 >= 0.0001)
		{
			//cout<<"error"<<endl;
		}
		tmp = theta*tmp;
		double _norm2 = (tmp - log(jrotation)).squaredNorm();
		if (_norm2 > 0.00001)
		{
			log(jrotation);
		}
		double _norm = jaxis.norm();
		if (_norm < 1)
		{
			//cout<<_norm<<endl;
		}
		double _sign = jaxis.dot(irot.axis);
		if (_sign < 0)
		{
			jaxis = -jaxis;
			theta = 2 * PI - theta;
		}
		jrot.axis = jaxis / jaxis.norm();//
		//jrot.axis = jaxis;
		jrot.theta = theta;
		jrot.circlek = irot.circlek;
	}


	if (abs(theta - irot.theta) > M_PI)
	{
		if (irot.theta < M_PI)
		{
			//[0,2*PI)
			jrot.circlek = jrot.circlek - 1;
		}
		else
		{
			if (jrot.circlek >= 0)
			{
				// cout<<"test"<<endl;
			}
			jrot.circlek = jrot.circlek + 1;
		}
	}
	jrot.ToLogR();
	Eigen::Matrix3d tmp = log(jrotation);
	double _norm1 = (jrot.logr - tmp).squaredNorm();
	if (_norm1 >= 0.0001)
	{
		//cout<<"error"<<endl;
	}

	return jrot;
}