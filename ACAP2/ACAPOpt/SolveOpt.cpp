// SolveDll.cpp : 定义 DLL 应用程序的导出函数。
//

#include <vector>

#include <NSolver/GUROBISolver.hh>

#include <NSolver/COMISOSolver.hh>
#include <NSolver/LinearConstraint.hh>
#include <NSolver/BoundConstraint.hh>
#include <NSolver/VariableType.hh>
#include <cmath>
#include <windows.h>
#include <omp.h>
#include "SolveOpt.h"
#include <iostream>

using namespace std;

#define OMP_open __pragma(omp parallel) \
{ \
	__pragma(omp for)
#define OMP_end \
}

long long g_timeOffset = 0;
long long  g_frequency = 0;
double GetCpuQPCTime(void)
{
	if (g_frequency == 0)
	{
		long long frequency;
		QueryPerformanceFrequency((LARGE_INTEGER*)&g_frequency);
	}

	long long  value;
	QueryPerformanceCounter((LARGE_INTEGER*)&value);
	return (double)(value - g_timeOffset) / g_frequency;
}

std::vector<Rot> SolveOpt(std::vector<Eigen::Vector3d> & axis, std::vector<double> & thetadata, std::vector<std::pair<int, int>> & edge_agjacent, int * Ldata_row, int * Ldata_col, double * Ldata_value, double lim_time, bool use_init)
//std::vector<Rot> SolveOpt(std::vector<Rot> & axis, std::vector<double> & thetadata, std::vector<std::pair<int, int>> & edge_agjacent, int * Ldata_row, int * Ldata_col, double * Ldata_value, double lim_time, bool use_init)
{

	int cpunum = omp_get_num_procs();
	//cpunum=1;
	omp_set_num_threads(cpunum * 2);
	double * theta_init_, *axis_init_;
	theta_init_ = nullptr;
	axis_init_ = nullptr;
	std::vector<double> theta_init_v(axis.size(),0);
	bool use_axis_init = false;
	bool use_theta_init = false;
	double _finalgap_axis = 0;
	double _finalgap_angle = 0;
	double gap_axis = 0.2;//0.7
	double gap_theta1 = 0.05;
	double gap_theta2 = 0.9999;
	int thresholdw = 0.00001;
	int thresholdt = 0.02;
	mstream moutm;
	streambuf *outbufm = cout.rdbuf(&moutm);//重定向cout 

	//lim_time[0] = 120;
	if (use_init)
	{
		use_axis_init = true;
		use_theta_init = true;
		//theta_init_ = thetadata;
		theta_init_v.assign(thetadata.begin(), thetadata.end());
		axis_init_ = (double *)malloc(axis.size() * sizeof(double));
		memset(axis_init_, 1.0, axis.size() * sizeof(double));
	}

	//std::vector<T> coefficients_axis;
	std::vector<T> coefficients_theta;

	double t3 = 0;

	int row_L = edge_agjacent.size() * 2;


	int row_theta = axis.size();

	//coefficients_axis.resize(row_A, T(0, 0, 0));
	coefficients_theta.resize(row_L, T(0, 0, 0));
	double t1 = GetCpuQPCTime();
	//OMP_open
	//	for (int i = 0; i < row_A; i++) { //遍历行
	//		coefficients_axis[i] = T(Adata[i] - 1, Adata[i + row_A] - 1, Adata[i + row_A * 2]);
	//	}
	//OMP_end
	OMP_open
		for (int i = 0; i < row_L; i++) { //遍历行
			coefficients_theta[i] = T(double(Ldata_row[i]), double(Ldata_col[i]), Ldata_value[i]);
		}
	OMP_end

	Eigen::SparseMatrix<double> matrix_L(edge_agjacent.size(), axis.size());
	matrix_L.setFromTriplets(coefficients_theta.begin(), coefficients_theta.end());
	Eigen::SparseMatrix<double> weightmatrix(edge_agjacent.size(), edge_agjacent.size());
	std::vector<T> weightmatrix_coff(edge_agjacent.size());
	for (int i=0;i<edge_agjacent.size();i++)
	{
		int id_i = edge_agjacent[i].first;
		int id_j = edge_agjacent[i].second;
		double dott = axis[id_i].dot(axis[id_j]);
		int weightl = 0.0;
		if (dott > thresholdw)
			weightl = 1.0;
		else if (dott < -thresholdw)
			weightl = -1.0;
		else
			weightl = 0.0;
		double theta_i = abs(thetadata[id_i] - 2 * M_PI*round(thetadata[id_i] / (M_PI * 2)));
		double theta_j = abs(thetadata[id_j] - 2 * M_PI*round(thetadata[id_j] / (M_PI * 2)));
		if (theta_i < thresholdt || theta_j < thresholdt)
			weightl = 0.0;
		weightmatrix_coff[i] = T(double(i), double(i), weightl);
		//matrix_Lw.row(i) = matrix_L.row(i)*weightl;
	}
	weightmatrix.setFromTriplets(weightmatrix_coff.begin(), weightmatrix_coff.end());
	Eigen::SparseMatrix<double> matrix_Lw = weightmatrix*matrix_L;
	Eigen::SparseMatrix<double> matrix_A = matrix_L.transpose()*matrix_Lw;


	SmallNProblem_axis snp_axis(row_theta, axis_init_, matrix_A, use_axis_init);
	SmallNProblem_theta snp_theta(row_theta, theta_init_v, matrix_L, thetadata, use_theta_init);
	double t2 = GetCpuQPCTime();

	cout << "Construct Successfully!!\nThe time is " << t2 - t1 << " !!\nSolving..." << endl;

	t1 = GetCpuQPCTime();

	//solve axis
	std::vector<COMISO::PairIndexVtype> discrete_variables_axis;
	discrete_variables_axis.resize(row_theta);
	for (int i = 0; i < row_theta; i++)
		discrete_variables_axis[i] = COMISO::PairIndexVtype(i, COMISO::Binary);
	std::vector<COMISO::NConstraintInterface*> constraints_axis;
	COMISO::BoundConstraint lc_axis(0, 1, row_theta, COMISO::LinearConstraint::NC_EQUAL);
	constraints_axis.push_back(&lc_axis);
	COMISO::GUROBISolver gsol_axis;
	gsol_axis.solve_two_phase(&snp_axis, constraints_axis, discrete_variables_axis, lim_time, gap_axis, lim_time, gap_axis, _finalgap_axis);


	for (int i = 0; i < row_theta; i++)
	{
		//out_axis[i] = snp_axis.result_axis[i] * 2 - 1;
		if (snp_axis.result_axis[i] == 0)
			snp_theta.theta(i) = 2 * M_PI - snp_theta.theta(i);
		//cout << snp_axis.result_axis[i] << endl;
	}
	t2 = GetCpuQPCTime();
	//solve theta
	//SmallNProblem_theta snp_theta(row_theta, theta_init_, coefficients_theta, thetadata, use_theta_init);	
	snp_theta.B_theta = 4 * M_PI * (snp_theta.theta.transpose() * snp_theta.ltl);
	std::vector<COMISO::PairIndexVtype> discrete_variables_theta;
	discrete_variables_theta.resize(row_theta);
	for (int i = 0; i < row_theta; i++)
		discrete_variables_theta[i] = COMISO::PairIndexVtype(i, COMISO::Integer);
	std::vector<COMISO::NConstraintInterface*> constraints_theta;
	constraints_theta.resize(row_theta * 2 - 1);
	COMISO::BoundConstraint lc(0, 0, row_theta, COMISO::LinearConstraint::NC_EQUAL);

	constraints_theta[0] = &lc;
	int count = 1;
	for (int i = 1; i < row_theta; i++)
	{
		constraints_theta[count++] = new COMISO::BoundConstraint(i, -10, row_theta, COMISO::LinearConstraint::NC_GREATER_EQUAL);
		constraints_theta[count++] = new COMISO::BoundConstraint(i, 10, row_theta, COMISO::LinearConstraint::NC_LESS_EQUAL);
	}
	//COMISO::GUROBISolver gsol_theta;
	//gsol_axis.solve(&snp_theta, constraints_theta, discrete_variables_theta, lim_time[0]);
	gsol_axis.solve_two_phase(&snp_theta, constraints_theta, discrete_variables_theta, lim_time, gap_theta1, lim_time, gap_theta2, _finalgap_angle);  //return final gap

	t3 = GetCpuQPCTime();

	cout << "Solving finish!!\nThe axis-solving time is " << t2 - t1 << "\nThe angle-solving time is " << t3 - t2 << "\nSaving result!!" << endl;
	cout << "Axis Gap is " << _finalgap_axis << "!\nTheta Gap is " << _finalgap_angle << "!" << endl;
	//output data

	std::vector<Rot> axis_angleout(axis.size());
	OMP_open
		for (int i = 0; i < row_theta; i++)
		{
			axis_angleout[i].axis = axis[i] * (snp_axis.result_axis[i] * 2 - 1);
			//out_theta[i] = snp_theta.result_theta[i];
			if (snp_theta.result_theta[0] == 1)
				axis_angleout[i].theta = snp_theta.result_theta[i] * 2 * M_PI + snp_theta.theta(i) - 2 * M_PI;
			else
				axis_angleout[i].theta = snp_theta.result_theta[i] * 2 * M_PI + snp_theta.theta(i);
			axis_angleout[i].circlek = 0;
		}
	OMP_end

	std::cout.rdbuf(outbufm);
	return axis_angleout;
}


