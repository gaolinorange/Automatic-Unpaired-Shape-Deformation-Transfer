/*===========================================================================*\
 *                                                                           *
 *                               CoMISo                                      *
 *      Copyright (C) 2008-2009 by Computer Graphics Group, RWTH Aachen      *
 *                           www.rwth-graphics.de                            *
 *                                                                           *
 *---------------------------------------------------------------------------* 
 *  This file is part of CoMISo.                                             *
 *                                                                           *
 *  CoMISo is free software: you can redistribute it and/or modify           *
 *  it under the terms of the GNU General Public License as published by     *
 *  the Free Software Foundation, either version 3 of the License, or        *
 *  (at your option) any later version.                                      *
 *                                                                           *
 *  CoMISo is distributed in the hope that it will be useful,                *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
 *  GNU General Public License for more details.                             *
 *                                                                           *
 *  You should have received a copy of the GNU General Public License        *
 *  along with CoMISo.  If not, see <http://www.gnu.org/licenses/>.          *
 *                                                                           *
\*===========================================================================*/ 

#include <CoMISo/Config/config.hh>
#include <CoMISo/Utils/StopWatch.hh>
#include <vector>
#include <CoMISo/NSolver/FiniteElementProblem.hh>
#include <CoMISo/NSolver/NPDerivativeChecker.hh>
#include <CoMISo/NSolver/IPOPTSolver.hh>
#include <CoMISo/NSolver/AcceleratedQuadraticProxy.hh>


// create log barrier term for triangle orientation
class TriangleOrientationLogBarrierElement
{
public:

  // define dimensions
  const static int NV = 6;
  const static int NC = 1;

  typedef Eigen::Matrix<size_t,NV,1> VecI;
  typedef Eigen::Matrix<double,NV,1> VecV;
  typedef Eigen::Matrix<double,NC,1> VecC;
  typedef Eigen::Triplet<double> Triplet;

  inline double eval_f       (const VecV& _x, const VecC& _c) const
  {
    return -_c[0]*std::log((-_x[0]+_x[2])*(-_x[1]+_x[5])-(-_x[0]+_x[4])*(-_x[1]+_x[3]));
  }

  inline void   eval_gradient(const VecV& _x, const VecC& _c, VecV& _g) const
  {
    // get c*determinant
    double d = _c[0]/((-_x[0]+_x[2])*(-_x[1]+_x[5])-(-_x[0]+_x[4])*(-_x[1]+_x[3]));

    _g[0] = -d*(-_x[5]+_x[3]);
    _g[1] = -d*(-_x[2]+_x[4]);
    _g[2] = -d*(-_x[1]+_x[5]);
    _g[3] = -d*( _x[0]-_x[4]);
    _g[4] = -d*( _x[1]-_x[3]);
    _g[5] = -d*(-_x[0]+_x[2]);
  }

  inline void   eval_hessian (const VecV& _x, const VecC& _c, std::vector<Triplet>& _triplets) const
  {
    double d = 1.0/((-_x[0]+_x[2])*(-_x[1]+_x[5])-(-_x[0]+_x[4])*(-_x[1]+_x[3]));

    VecV g;
    g[0] = d*(-_x[5]+_x[3]);
    g[1] = d*(-_x[2]+_x[4]);
    g[2] = d*(-_x[1]+_x[5]);
    g[3] = d*( _x[0]-_x[4]);
    g[4] = d*( _x[1]-_x[3]);
    g[5] = d*(-_x[0]+_x[2]);

    _triplets.clear();
    for(unsigned int i=0; i<NV; ++i)
      for(unsigned int j=0; j<NV; ++j)
      {
        double v = _c[0]*g[i]*g[j];
        _triplets.push_back(Triplet(i,j,v));
        _triplets.push_back(Triplet(j,i,v));
      }

    d *= _c[0];
    _triplets.push_back(Triplet(0,5, d));
    _triplets.push_back(Triplet(0,3,-d));
    _triplets.push_back(Triplet(5,0, d));
    _triplets.push_back(Triplet(3,0,-d));

    _triplets.push_back(Triplet(1,2, d));
    _triplets.push_back(Triplet(1,4,-d));
    _triplets.push_back(Triplet(2,1, d));
    _triplets.push_back(Triplet(4,1,-d));

    _triplets.push_back(Triplet(2,1, d));
    _triplets.push_back(Triplet(2,5,-d));
    _triplets.push_back(Triplet(1,2, d));
    _triplets.push_back(Triplet(5,2,-d));

    _triplets.push_back(Triplet(3,4, d));
    _triplets.push_back(Triplet(3,0,-d));
    _triplets.push_back(Triplet(4,3, d));
    _triplets.push_back(Triplet(0,3,-d));

    _triplets.push_back(Triplet(4,3, d));
    _triplets.push_back(Triplet(4,1,-d));
    _triplets.push_back(Triplet(3,4, d));
    _triplets.push_back(Triplet(1,4,-d));

    _triplets.push_back(Triplet(5,0, d));
    _triplets.push_back(Triplet(5,2,-d));
    _triplets.push_back(Triplet(0,5, d));
    _triplets.push_back(Triplet(2,5,-d));
  }

  inline double max_feasible_step(const VecV& _x, const VecV& _v, const VecC& _c)
  {
    double a = ((-_v[0]+_v[2])*(-_v[1]+_v[5])-(-_v[0]+_v[4])*(-_v[1]+_v[3]));
    double b = ((-_x[0]+_x[2])*(-_v[1]+_v[5])+(-_v[0]+_v[2])*(-_x[1]+_x[5])-(-_x[0]+_x[4])*(-_v[1]+_v[3])-(-_v[0]+_v[4])*(-_x[1]+_x[3]));
    double c = (-_x[0]+_x[2])*(-_x[1]+_x[5])-(-_x[0]+_x[4])*(-_x[1]+_x[3]);

    double a2 = a*a;
    double b2 = b*b;
    double c2 = c*c;

    double eps = 1e-9*(a2+b2+c2);

    // special case a=0?
    if(a2 <= eps)
    {
      if(b2 <= eps) // special case a=b=0
      {
        if(c2 <= eps) return 0.0; // special case a=b=c=0
        else          return DBL_MAX; // special case a=b=0 & c!=0
      }
      else // special case a=0 & b!=0
      {
        double t=-c/b;

        if( t>=0 ) return t;
        else       return DBL_MAX;
      }
    }
    else
    {
      // discriminant
      double d = b*b-4*a*c;

      if(d < 0 ) return DBL_MAX;
      else
      {
        d = std::sqrt(d);

        double t1 = (-b+d)/(2.0*a);
        double t2 = (-b-d)/(2.0*a);

        if(t1 < 0.0) t1 = DBL_MAX;
        if(t2 < 0.0) t2 = DBL_MAX;

        return std::min(t1,t2);
      }
    }
  }
};

// create a simple finite element (x-c)^2
class SimpleElement2
{
public:

  // define dimensions
  const static int NV = 1;
  const static int NC = 1;

  typedef Eigen::Matrix<size_t,NV,1> VecI;
  typedef Eigen::Matrix<double,NV,1> VecV;
  typedef Eigen::Matrix<double,NC,1> VecC;
  typedef Eigen::Triplet<double> Triplet;

  inline double eval_f(const VecV& _x, const VecC& _c) const
  {
    return std::pow(_x[0]-_c[0],2);
  }

  inline void   eval_gradient(const VecV& _x, const VecC& _c, VecV& _g) const
  {
    _g[0] = 2.0*(_x[0]-_c[0]);
  }

  inline void   eval_hessian (const VecV& _x, const VecC& _c, std::vector<Triplet>& _triplets) const
  {
    _triplets.clear();
    _triplets.push_back(Triplet(0,0,2));
  }

  inline double max_feasible_step(const VecV& _x, const VecV& _v, const VecC& _c)
  {
    return DBL_MAX;
  }
};


//------------------------------------------------------------------------------------------------------

// Example main
int main(void)
{
  {
    TriangleOrientationLogBarrierElement tolbe;

    TriangleOrientationLogBarrierElement::VecV x; x << 0.0, 0.0, 2.0, 0.0, 1.0, 1.0;
    TriangleOrientationLogBarrierElement::VecV v; v << 0.0, 0.0, 0.0, 0.0,-1.0,-1.0;
    TriangleOrientationLogBarrierElement::VecC c; c << 1.0;

    std::cerr << "max feasible step: " << tolbe.max_feasible_step(x,v,c) << std::endl;
  }

  std::cout << "---------- 1) Get an instance of a FiniteElementProblem..." << std::endl;

  // first create sets of different finite elements
  COMISO::FiniteElementSet<TriangleOrientationLogBarrierElement> fe_set;

  TriangleOrientationLogBarrierElement::VecI tidx;
  tidx << 0,1,2,3,4,5;
  TriangleOrientationLogBarrierElement::VecC c;
  c[0] = 1.0;
  fe_set.instances().add_element(tidx, c);

  std::cerr << "T1" << std::endl;

  // second set for boundary conditions
  COMISO::FiniteElementSet<SimpleElement2> fe_set2;
  SimpleElement2::VecI idx;
  SimpleElement2::VecC c2;
  idx[0] = 0; c2[0] =  0.0; fe_set2.instances().add_element(idx, c2);
  idx[0] = 1; c2[0] =  0.0; fe_set2.instances().add_element(idx, c2);
  idx[0] = 2; c2[0] =  2.0; fe_set2.instances().add_element(idx, c2);
  idx[0] = 3; c2[0] =  0.0; fe_set2.instances().add_element(idx, c2);
  idx[0] = 4; c2[0] =  1.0; fe_set2.instances().add_element(idx, c2);
  idx[0] = 5; c2[0] = -2.0; fe_set2.instances().add_element(idx, c2);

  std::cerr << "T2" << std::endl;

  // then create finite element problem and add sets
  COMISO::FiniteElementProblem fe_problem(6);
  fe_problem.add_set(&fe_set );
  fe_problem.add_set(&fe_set2);

  std::cerr << "T3" << std::endl;

  // set initial values
  fe_problem.x()[0] = 0.0;
  fe_problem.x()[1] = 0.0;
  fe_problem.x()[2] = 2.0;
  fe_problem.x()[3] = 0.0;
  fe_problem.x()[4] = 1.0;
  fe_problem.x()[5] = 1.0;

  std::cerr << "T4" << std::endl;

  std::cout << "---------- 2) (optional for debugging) Check derivatives of problem..." << std::endl;
  COMISO::NPDerivativeChecker npd;
  npd.check_all(&fe_problem);

  // check if IPOPT solver available in current configuration
  #if( COMISO_IPOPT_AVAILABLE)
    std::cout << "---------- 3) Get IPOPT solver... " << std::endl;
    COMISO::IPOPTSolver ipsol;

    std::cout << "---------- 4) Solve..." << std::endl;
    // there are no constraints -> provide an empty vector
    std::vector<COMISO::NConstraintInterface*> constraints;
    ipsol.solve(&fe_problem, constraints);
  #endif

  // print result
  for(unsigned int i=0; i<fe_problem.x().size(); ++i)
    std::cerr << "x[" << i << "] = " << fe_problem.x()[i] << std::endl;


  std::cout << "---------- 5) Get AQP Solver... " << std::endl;
  COMISO::AcceleratedQuadraticProxy aqp;

  std::cout << "---------- 6) Solve..." << std::endl;
  // there are no constraints -> provide an empty vector

  // then create finite element problems
  // first the nonlinear part
  COMISO::FiniteElementProblem fe_problem2(6);
  fe_problem2.add_set(&fe_set );
  // second the quadratic part
  COMISO::FiniteElementProblem fe_problem3(6);
  fe_problem3.add_set(&fe_set2);

  // set initial values
  fe_problem3.x()[0] = 0.0;
  fe_problem3.x()[1] = 0.0;
  fe_problem3.x()[2] = 2.0;
  fe_problem3.x()[3] = 0.0;
  fe_problem3.x()[4] = 1.0;
  fe_problem3.x()[5] = 1.0;

  COMISO::AcceleratedQuadraticProxy::SMatrixD A(0,6);
  A.setZero();
  COMISO::AcceleratedQuadraticProxy::VectorD b; b.resize(0);

  // alternatively with constraints
  //  COMISO::AcceleratedQuadraticProxy::SMatrixD A(2,6);
  //  A.setZero(); A.coeffRef(0,0) = 1.0; A.coeffRef(1,1) = 1.0;
  //  COMISO::AcceleratedQuadraticProxy::VectorD b(2);
  //  b[0] = b[1] = 0.0;

  // solve
  aqp.solve(&fe_problem3, &fe_problem2, A, b);

  // print result
  for(unsigned int i=0; i<fe_problem3.x().size(); ++i)
    std::cerr << "x[" << i << "] = " << fe_problem3.x()[i] << std::endl;


  return 0;
}

