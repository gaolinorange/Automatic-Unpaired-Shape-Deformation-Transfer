// (C) Copyright 2014 by Autodesk, Inc.

#ifndef GMMTYPES_HH_INCLUDED
#define GMMTYPES_HH_INCLUDED

#include <gmm/gmm_matrix.h>

namespace COMISO_GMM
{  

// Matrix Types
typedef gmm::col_matrix< gmm::wsvector<double> > WSColMatrix;
typedef gmm::row_matrix< gmm::wsvector<double> > WSRowMatrix;
typedef gmm::col_matrix< gmm::rsvector<double> > RSColMatrix;
typedef gmm::row_matrix< gmm::rsvector<double> > RSRowMatrix;
typedef gmm::csc_matrix<double> CSCMatrix;

}//namespace COMISO_GMM

#endif//GMMTYPES_HH_INCLUDED
