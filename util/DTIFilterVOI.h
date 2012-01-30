/** Copyright (c) 2005, The Board of Trustees of Leland Stanford Junior 
University. All rights reserved. **/

#ifndef FILTER_VOI_H
#define FILTER_VOI_H

#include "DTIFilter.h"

class DTIPathway;
class vtkPolyData;
class vtkCellArray;

#ifdef USE_RAPID
class RAPID_model;
typedef RAPID_model* collisionModel;
#else
#include <ozcollide/aabbtree_poly.h>
typedef ozcollide::AABBTreePoly* collisionModel;
#endif

static double ZERO_ROTATION[3][3] = {{1.0, 0.0, 0.0},
			      {0.0, 1.0, 0.0},
			      {0.0, 0.0, 1.0}};

static double ZERO_TRANSLATION[] = {0.0, 0.0, 0.0};

class DTIFilterVOI : public DTIFilter {
 public:
  DTIFilterVOI();
  virtual ~DTIFilterVOI();
  //  void addTriangle(const double a[3], const double b[3], const double c[3]);
  //  void endTriangles();
  void setRotationMatrix (const double R[3][3]);
  void setTranslation (const double R[3]);
  virtual void setScale (const double s[3]) = 0;
  virtual bool matches (DTIPathway *pathway);
  bool intersects (DTIPathway *pathway, collisionModel filterModel);

 protected:
  collisionModel generateModelFromPolydata(vtkPolyData *pd, vtkCellArray *cells);
  virtual collisionModel computeModel() = 0;
  collisionModel _cached_model;
  // int _counter;
  double _rotation_matrix[3][3];
  double _translation[3];
};

#endif
