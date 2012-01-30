/** Copyright (c) 2005, The Board of Trustees of Leland Stanford Junior 
University. All rights reserved. **/

#ifndef COMPLEX_FILTER_VOI_H
#define COMPLEX_FILTER_VOI_H

#include "DTIFilterVOI.h"

class DTIPathway;
class vtkPolyData;

#include "vtkProperty.h"

class DTIFilterComplex : public DTIFilterVOI {
 public:
  // Mesh will not change so let's set it here
  DTIFilterComplex(vtkPolyData *triangles);
  virtual ~DTIFilterComplex();
  virtual bool matches (DTIPathway *pathway);
  virtual collisionModel computeModel();
  bool inside (DTIPathway *pathway);
  virtual void setScale (const double scale[3]);
  
 private:
  void expathwayPoint (vtkIdType id, double pt[3]);
  double _scale[3];
  //  double _delta[3];
  vtkPolyData *tris;
};

#endif
