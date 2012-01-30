/** Copyright (c) 2005, The Board of Trustees of Leland Stanford Junior 
University. All rights reserved. **/

#ifndef DTI_VOLUME_INTERFACE_H
#define DTI_VOLUME_INTERFACE_H

#include "typedefs.h"

class DTIVolumeInterface {
 public:
  
  virtual void getDimension (unsigned int &xDim, unsigned int &yDim, unsigned int &zDim) const = 0;
  virtual DTITensor getTensorInterpolate (const DTIVector &position) const = 0;
  virtual bool inBounds (const DTIVector &vec) const = 0;  
  virtual void getVoxelSize (double &xSize, double &ySize, double &zSize) const = 0;

 protected:
  DTIVolumeInterface() {}
  virtual ~DTIVolumeInterface() {}
};

#endif
