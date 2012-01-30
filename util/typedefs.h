/** Copyright (c) 2005, The Board of Trustees of Leland Stanford Junior 
University. All rights reserved. **/

#ifndef DTI_H
#define DTI_H

#include <tnt/tnt_vec.h>
#include <tnt/tnt_array3d.h>
#include <tnt/tnt_cmat.h>


enum NumericalIntegrationMethod {
  INTEGRATION_EULER = 0,
  INTEGRATION_RUNGE_KUTTA_2 = 1,
  INTEGRATION_RUNGE_KUTTA_4 = 2,
  INTEGRATION_NUM_CHOICES = 3
};

enum StatisticID {
  STAT_ID_LENGTH = 0,
  STAT_ID_FA = 1,
  STAT_ID_CURVATURE = 2,
  NUM_STAT_IDS = 3
};

const std::string emptyStrOut = "NO";

typedef int PathwayProperty;
typedef int PointProperty;

/* enum PathwayProperty {
    PATHWAY_LENGTH = 0,
    PATHWAY_MEDIAN_FA = 1,
    PATHWAY_MEAN_CURVATURE = 2,
    PATHWAY_PRIOR = 3,
    PATHWAY_LIKELIHOOD = 4,
    PATHWAY_POSTERIOR = 5,
    PATHWAY_MIN_FA = 6,
    PATHWAY_MAX_ANGLE = 7,
    PATHWAY_ALGORITHM = 8
  };
*/
static const double PATHWAY_COLORS[][3] = {
  {128, 128, 128},
  {20, 90, 200}, /* blue */
  {152, 78, 163}, /* purple */
  {255, 255, 51}, /* yellow */
  {77, 175, 74}, /* green */
  {190, 40, 40}, /* red */
  {255, 127, 0}, /* orange */
  {120, 180, 180}, /* cyan */
  {120, 100, 50} /* gold */
};
typedef int DTILuminanceEncoding;

/*
enum DTILuminanceEncoding {
  DTI_LUMINANCE_JITTER = 0,
  DTI_LUMINANCE_CURVATURE = 1,
  DTI_LUMINANCE_PRIOR = 2,
  DTI_LUMINANCE_POSTERIOR = 3,
  DTI_LUMINANCE_LIKELIHOOD = 4,
  DTI_LUMINANCE_NUM_OPTIONS = 5
};
*/
enum DTIQueryOperation {

  DTI_FILTER_AND = 0,
  DTI_FILTER_OR = 1,
  DTI_FILTER_CUSTOM = 2
};

typedef int DTIPathwayGroupID;

enum DTIPathwayColor {

  DTI_COLOR_UNASSIGNED = 0,
  DTI_COLOR_GREY = 0,
  DTI_COLOR_BLUE = 1,
  DTI_COLOR_YELLOW = 2,
  DTI_COLOR_GREEN = 3,
  DTI_COLOR_PURPLE = 4,
  DTI_COLOR_ORANGE = 5,
  DTI_COLOR_RED = 6,
  NUM_PATHWAY_COLORS = 7

};

typedef int DTIPathwayAlgorithm;
static const int DTI_PATHWAY_ALGORITHM_ANY = -1;

typedef double NumberType;

typedef TNT::Array1D<double> DTIVector;

typedef TNT::Array1D<float> DTIGeometryVector;

typedef TNT::Array2D<double> DTITensor;

typedef TNT::Array3D<DTITensor> DTITensorArray;

typedef TNT::Array3D<double> DTIFAArray;


template <class T>
TNT::Array1D<T> operator* (const TNT::Array1D<T> &A, const T & B) {

  TNT::Array1D<T> C(A.dim1());
  for (int i = 0; i < A.dim1(); i++) {
    C[i] = A[i] * B;
  }
  return C;
}


template <class T>
TNT::Array1D<T> operator* (const T & B, const TNT::Array1D<T> &A) {

  TNT::Array1D<T> C(A.dim1());
  for (int i = 0; i < A.dim1(); i++) {
    C[i] = A[i] * B;
  }
  return C;
}

/*
template <class T>
T dproduct (const TNT::Array1D<T> &A, const TNT::Array1D<T> &B) {

  T sum = 0.0;
  for (int i = 0; i < A.dim1(); i++) {
    sum += A[i] * B[i];
        //std::cerr<< "sum: " << sum << std::endl;
  }
  return sum;
}*/

inline double 
dproduct (const TNT::Array1D<double> &A, const TNT::Array1D<double> &B) {

  double sum = 0.0;
  for (int i = 0; i < A.dim1(); i++) {
    sum += A[i] * B[i];
        //std::cerr<< "sum: " << sum << std::endl;
  }
  return sum;
}



template <class T>
TNT::Array1D<T> operator- (const TNT::Array1D<T> &A) {

  TNT::Array1D<T> B(A.dim1());
  for (int i = 0; i < A.dim1(); i++) {
    B[i] = -A[i];
  }
  return B;
}

template <class T>
T abs (const TNT::Array1D<T> &A) {

  T mag = 0;
  for (int i = 0; i < A.dim1(); i++) {
    mag += A[i]*A[i];
  }
  return sqrt (mag);
}



template <class T>
TNT::Array1D<T> norm (const TNT::Array1D<T> &A) {

  double mag = abs(A);
  TNT::Array1D<T> B(A.dim1());
  for (int i = 0; i < A.dim1(); i++) {
    B[i] = A[i]/mag;
  }
  return B;
}


#endif
