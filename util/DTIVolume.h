/** Copyright (c) 2005, The Board of Trustees of Leland Stanford Junior 
University. All rights reserved. **/

#ifndef DTI_VOLUME_H
#define DTI_VOLUME_H

#include "typedefs.h"
#include "DTIVolumeInterface.h"

class DTIVolume : public DTIVolumeInterface {

  friend class DTIVolumeIO;

 private:
  DTIVolume();
 protected:
  virtual ~DTIVolume();

 public:

  // Constructor: Creates a tensor volume and allocates the data.
  
  DTIVolume (int x, int y, int z);

  // Get the dimensions of the volume.

  virtual void getDimension (unsigned int &x, unsigned int &y, unsigned int &z) const;

  // Get the size of each voxel in anatomical space (mm)

  virtual void getVoxelSize (double &x, double &y, double &z) const;

  virtual void setVoxelSize (double x, double y, double z);

  // Check to see if a point is inside the volume.

  bool inBounds (const DTIVector &vec) const;

  // Get a tensor, performing linear interpolation.

  DTITensor getTensorInterpolate(const DTIVector &position) const;

  // Get a tensor without interpolation.

  DTITensor getTensor (int x, int y, int z) const;

  // Set a tensor at a particular (x,y,z) location.
  // format is: [Dxx, Dyy, Dzz, Dxy, Dxz, Dyz]
  void setTensor (int x, int y, int z, const double data[6]);

  double *getTransformMatrix ();
  double *getInverseTransformMatrix();

  double *getSformMatrix () { return &(_stoxyz[0][0]); }
  double *getInverseSformMatrix() { return &(_stoijk[0][0]); }

  double *getQformMatrix () { return &(_qtoxyz[0][0]); }
  double *getInverseQformMatrix() { return &(_qtoijk[0][0]); }

  int getQformCode() { return _qform_code; }
  int getSformCode() { return _sform_code; }

 private:
  double _x_size, _y_size, _z_size;
  DTITensorArray *_tensor_array;

  double _qtoxyz[4][4];
  double _qtoijk[4][4];
  double _stoxyz[4][4];
  double _stoijk[4][4];
  
  int _qform_code;
  int _sform_code;
};

#endif
