/** Copyright (c) 2005, The Board of Trustees of Leland Stanford Junior 
University. All rights reserved. **/

#ifndef CUBE_FILTER_VOI_H
#define CUBE_FILTER_VOI_H

#include "DTIFilterVOI.h"

class DTIPathway;

class DTIFilterCube : public DTIFilterVOI {
 public:
  DTIFilterCube();
  virtual ~DTIFilterCube();
  virtual bool matches (DTIPathway *pathway);
  virtual collisionModel computeModel();

  bool inside (DTIPathway *pathway);
  virtual void setScale (const double scale[3]);
  
 private:
  double _scale[3];
};

#endif
