/***********************************************************************
 * AUTHOR: David Akers <dakers>
 *   FILE: .//DTIMath.cpp
 *   DATE: Fri Sep 23 13:06:57 2005
 *  DESCR: 
 ***********************************************************************/
#include "DTIMath.h"
#include <math.h>
#include "typedefs.h"
#include <tnt/tnt_array1d_utils.h>
#include <tnt/tnt_array2d_utils.h>
#include <tnt/tnt_vec.h>

static double *FAC_LOOKUP = NULL;

double factorial_compute (int n) 
{
  int ans = 1;
  for (int i = 2; i <= n; i++) {
    ans *= i;
  }
  return ans;
}

/***********************************************************************
 *  Method: DTIMath::randzeroone
 *  Params: 
 * Returns: double
 * Effects: 
 ***********************************************************************/
double
DTIMath::randzeroone()
{
  int randomnum = rand();
  if (randomnum == 0) {
    randomnum = 1;
  }
  return (double) randomnum/RAND_MAX;
}


/***********************************************************************
 *  Method: DTIMath::factorial
 *  Params: int n
 * Returns: double
 * Effects: 
 ***********************************************************************/
double
DTIMath::factorial(int n)
{
  if (!FAC_LOOKUP) {
    FAC_LOOKUP = new double[100];
    for (int i = 0; i < 100; i++) {
      FAC_LOOKUP[i] = (double) factorial_compute(i);
    }
  }
  assert (n < 100 && n >= 0);
  return (double) FAC_LOOKUP[n];
}


/***********************************************************************
 *  Method: DTIMath::nchoosek
 *  Params: int n, int k
 * Returns: double
 * Effects: 
 ***********************************************************************/
double
DTIMath::nchoosek(int n, int k)
{
  return factorial (n) / (factorial (n-k) * factorial (k));
}



/***********************************************************************
 *  Method: DTIMath::cross
 *  Params: const DTIVector &a, const DTIVector &b
 * Returns: DTIVector
 * Effects: 
 ***********************************************************************/
DTIVector
DTIMath::cross(const DTIVector &a, const DTIVector &b)
{
  DTIVector result(3);
  result[0] = a[1]*b[2]-a[2]*b[1];
  result[1] = a[2]*b[0]-a[0]*b[2];
  result[2] = a[0]*b[1]-a[1]*b[0];
  return result.copy();
}


/***********************************************************************
 *  Method: DTIMath::make_matrix
 *  Params: double x1, double x2, double x3, double y1, double y2, double y3, double z1, double z2, double z3
 * Returns: TNT::Array2D<double>
 * Effects: 
 ***********************************************************************/
TNT::Array2D<double>
DTIMath::make_matrix(double x1, double x2, double x3, double y1, double y2, double y3, double z1, double z2, double z3)
{
  TNT::Array2D<double> retval (3,3);
  retval[0][0] = x1;
  retval[0][1] = x2;
  retval[0][2] = x3;
  retval[1][0] = y1;
  retval[1][1] = y2;
  retval[1][2] = y3;
  retval[2][0] = z1;
  retval[2][1] = z2;
  retval[2][2] = z3;
  return retval;
}



