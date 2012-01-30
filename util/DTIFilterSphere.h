/** Copyright (c) 2005, The Board of Trustees of Leland Stanford Junior 
University. All rights reserved. **/

#ifndef SPHERE_FILTER_VOI_H
#define SPHERE_FILTER_VOI_H

#include "DTIFilterVOI.h"

class DTIPathway;
class vtkActor;
class vtkPolyData;

class DTIFilterSphere : public DTIFilterVOI {
 public:
  DTIFilterSphere(vtkActor *actor);
  virtual ~DTIFilterSphere();
  virtual bool matches (DTIPathway *pathway);
  virtual collisionModel computeModel();
  bool inside (DTIPathway *pathway);
  virtual void setScale (const double scale[3]);
  
  static vtkPolyData *GenerateEllipsoid(const double scale[3]);
 private:
  vtkActor *actor;
  double _scale[3];
};

#endif
