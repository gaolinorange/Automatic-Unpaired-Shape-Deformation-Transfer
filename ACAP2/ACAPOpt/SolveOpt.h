#pragma once


#include <vector>
#include <NSolver/NProblemInterface.hh>
#include <Eigen/Sparse>
#include <mex.h>
#include "Align.h"


typedef Eigen::Triplet<double> T;

double GetCpuQPCTime(void);


class mstream : public std::streambuf {
public:
protected:
	virtual std::streamsize xsputn(const char *s, std::streamsize n) {
		mexPrintf("%.*s", n, s);
		return n;
	}
	virtual int overflow(int c = EOF) {
		if (c != EOF) {
			mexPrintf("%.1s", &c);
		}
		return 1;
	}
};

class SmallNProblem_axis : public COMISO::NProblemInterface
{
public:
	int num;
	//double * k;
	bool use_initvalue = false;
	Eigen::SparseMatrix<double> A, H_axis, AtplusA;
	Eigen::VectorXd temp;
	Eigen::MatrixXd one;
	std::vector<double> init_axis;
	double * result_axis = NULL;
	SmallNProblem_axis(int _num, double* _k, std::vector<T> & coo, bool use_or_not) {
		num = _num;
		//k = _k;
		result_axis = new double[num];
		one.setOnes(1, num);
		//result_axis.resize(num);
		temp.resize(num);
		use_initvalue = use_or_not;
		init_axis.resize(num, 1);
		if (use_initvalue)
			memcpy(init_axis.data(), _k, sizeof(double)*num);
		A.resize(num, num); // 等号左边的矩阵A
		A.setFromTriplets(coo.begin(), coo.end());
		AtplusA = Eigen::SparseMatrix<double>(A.transpose()) + A;
		H_axis = 4 * AtplusA;
	}
	SmallNProblem_axis(int _num, double* _k, Eigen::SparseMatrix<double> & coo, bool use_or_not) {
		num = _num;
		//k = _k;
		result_axis = new double[num];
		one.setOnes(1, num);
		//result_axis.resize(num);
		temp.resize(num);
		use_initvalue = use_or_not;
		init_axis.resize(num, 1);
		if (use_initvalue)
			memcpy(init_axis.data(), _k, sizeof(double)*num);
		A.resize(num, num); // 等号左边的矩阵A
		A = coo;
		AtplusA = Eigen::SparseMatrix<double>(A.transpose()) + A;
		H_axis = 4 * AtplusA;
	}

	virtual int    n_unknowns()
	{
		return num;
	}


	virtual void   initial_x(double* _x)
	{

		if (use_initvalue)
			for (int i = 0; i < num; i++)
				_x[i] = init_axis[i];
		else
			for (int i = 0; i < num; i++)
				_x[i] = 1.0;
	}


	virtual double eval_f(const double* _x)
	{
		//Eigen::MatrixXd k;//构造未知数矩阵
		//k.resize(num);
		for (int i = 0; i < num; i++)
			temp(i) = _x[i];
		//Eigen::VectorXd one = Eigen::VectorXd::Ones(1,num);//行向量1

		//for (int i = 0; i < num; i++)
		//one(0, i) = 1;
		Eigen::MatrixXd m3 = 4 * temp.transpose()*A*temp - 4 * one*A*temp;//L*k
		return m3(0, 0);
	}


	virtual void   eval_gradient(const double* _x, double*    _g)
	{
		//Eigen::VectorXd k;//未知数矩阵
		//k.resize(num);
		for (int i = 0; i < num; i++)
			temp(i) = _x[i];
		//Eigen::MatrixXd one;//行向量1
		//one.resize(1, num);
		//for (int i = 0; i < num; i++)
		//one(0, i) = 1;

		Eigen::MatrixXd G = 4 * temp.transpose() * AtplusA - 4 * one * A;//梯度表达式，注意G算出来是行向量
		for (int i = 0; i < num; i++)
			_g[i] = G(0, i);
	}


	virtual void   eval_hessian(const double* _x, SMatrixNP& _H)
	{
		_H.resize(n_unknowns(), n_unknowns());
		_H.setZero();
		_H = H_axis;
		//openmp
		//for (int i = 0; i < num; i++)
		//for (int j = 0; j < num; j++)
		//;
		//_H.coeffRef(i, j) = H_axis(i, j);//海森矩阵是常数，所以直接利用之前算出来的H即可

	}

	virtual void   store_result(const double* _x)
	{
		mstream mout;
		std::streambuf *outbuf = std::cout.rdbuf(&mout);//重定向cout  
		std::cout << "Axis Energy: " << eval_f(_x) << std::endl;
		//result = new double[num];
		memcpy(result_axis, _x, sizeof(double)*num);
		//for (int i = 0; i < num; i++)
		//result_axis[i] = _x[i];
		std::cout.rdbuf(outbuf);
	}

	virtual bool   constant_hessian() const { return true; }
};


class SmallNProblem_theta : public COMISO::NProblemInterface
{
public:
	int num;
	//double * k;
	bool use_initvalue = false;
	Eigen::SparseMatrix<double> L, H_theta, ltl, A_theta;
	Eigen::VectorXd theta, temp;
	Eigen::MatrixXd B_theta;
	std::vector<double> init_theta;
	double * result_theta = NULL;

	SmallNProblem_theta(int _num, double *_k, std::vector<T> & coo, double * thetadata, bool use_or_not) {
		num = _num;
		//k = _k;
		result_theta = new double[num];
		//result_theta.resize(num);
		temp.resize(num);
		init_theta.resize(num, 0);
		use_initvalue = use_or_not;
		if (use_initvalue)
			memcpy(init_theta.data(), _k, sizeof(double)*num);

		L.resize(coo.back().row() + 1, num); // 等号左边的矩阵A
		L.setFromTriplets(coo.begin(), coo.end());
		//L.selfadjointView<Lower>() = L.selfadjointView<Upper>();
		theta.resize(num);
		for (int i = 0; i < num; i++)
			theta(i) = thetadata[i];
		ltl = L.transpose()*L;
		A_theta = 4 * M_PI*M_PI*ltl;
		H_theta = 2 * A_theta;
		//B_theta = 4 * M_PI * (theta.transpose() * ltl);
	}
	SmallNProblem_theta(int _num, std::vector<double> &_k, Eigen::SparseMatrix<double> & coo, std::vector<double> & thetadata, bool use_or_not) {
		num = _num;
		//k = _k;
		result_theta = new double[num];
		//result_theta.resize(num);
		temp.resize(num);
		init_theta.resize(num, 0);
		use_initvalue = use_or_not;
		if (use_initvalue)
			init_theta.assign(_k.begin(), _k.end());
			//memcpy(init_theta.data(), _k, sizeof(double)*num);

		//L.resize(coo.back().row() + 1, num); // 等号左边的矩阵A
		//L.setFromTriplets(coo.begin(), coo.end());
		L = coo;
		//L.selfadjointView<Lower>() = L.selfadjointView<Upper>();
		theta.resize(num);
		for (int i = 0; i < num; i++)
			theta(i) = thetadata[i];
		ltl = L.transpose()*L;
		A_theta = 4 * M_PI*M_PI*ltl;
		H_theta = 2 * A_theta;
		//B_theta = 4 * M_PI * (theta.transpose() * ltl);
	}
	virtual int    n_unknowns()
	{
		return num;
	}

	virtual void   initial_x(double* _x)
	{
		if (use_initvalue)
			for (int i = 0; i < num; i++)
				_x[i] = init_theta[i];
		else
			for (int i = 0; i < num; i++)
				_x[i] = 0;
	}

	virtual double eval_f(const double* _x)
	{
		//Eigen::VectorXd k;
		//k.resize(num);
		for (int i = 0; i < num; i++)
			temp(i) = _x[i];
		//
		Eigen::MatrixXd m3 = L*(theta + temp * 2 * M_PI);

		return m3.squaredNorm();
	}

	virtual void   eval_gradient(const double * _x, double *   _g)
	{
		//Eigen::VectorXd k;
		//k.resize(num);
		//store_result(_x);
		for (int i = 0; i < num; i++)
			temp(i) = _x[i];
		//
		Eigen::MatrixXd G = temp.transpose() * 2 * A_theta + B_theta;
		for (int i = 0; i < num; i++)
			_g[i] = G(0, i);
		//_g[i] = G(i);
	}

	//hessian matrix evaluation at location _x
	virtual void   eval_hessian(const double* _x, SMatrixNP& _H)
	{
		_H.resize(n_unknowns(), n_unknowns());
		_H.setZero();
		_H = H_theta;
		//store_result(_x);
		//openmp
		//for (int i = 0; i < num; i++)
		//for (int j = 0; j < num; j++)
		//;//_H.coeffRef(i, j) = H(i, j);
	}

	// print result
	virtual void   store_result(const double* _x)
	{
		mstream mout;
		std::streambuf *outbuf = std::cout.rdbuf(&mout);//重定向cout  
		std::cout << "theta Energy: " << eval_f(_x) << std::endl;
		//result_theta = new double[num];
		memcpy(result_theta, _x, sizeof(double)*num);
		//for (int i = 0; i < num; i++)
		//result_theta[i] = _x[i];
		std::cout.rdbuf(outbuf);
	}

	// advanced properties
	virtual bool   constant_hessian() const { return true; }
};

std::vector<Rot> SolveOpt(std::vector<Eigen::Vector3d> & axis, std::vector<double> & theta, std::vector<std::pair<int, int>> & edge_agjacent, int * Ldata_row, int * Ldata_col, double * Ldata_value, double lim_time=120, bool use_init=false);