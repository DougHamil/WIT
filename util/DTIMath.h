#ifndef DTI_MATH_H
#define DTI_MATH_H

#include "typedefs.h"

class DTIMath {

 public:
  static double randn(); // random number from normal distribution (mean 0: std: 1)
  static double randzeroone (); // random number uniform (0,1)
  static void randn3(double r[3]); // random number from normal for 3D
  static double pdf_randn3(double r[3]); // random number from normal for 3D

  static double factorial (int n); 
  static double nchoosek (int n, int k);

  static double sqr (double x) { return x*x; }

  static DTIVector perturbVector (const DTIVector &orig, double theta, double phi);
  static DTIVector cross (const DTIVector &a, const DTIVector &b);

  static TNT::Array2D<double> make_matrix (double x1, double x2, double x3,
				    double y1, double y2, double y3,
				    double z1, double z2, double z3);

  static double getStdParker (double fa);
};

#endif
