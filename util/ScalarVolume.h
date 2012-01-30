/** Copyright (c) 2005, The Board of Trustees of Leland Stanford Junior 
University. All rights reserved. **/

#ifndef FA_VOLUME_H
#define FA_VOLUME_H

#include <string.h>
#include "typedefs.h"
#include "ScalarVolumeInterface.h"

class DTIVolume;

class ScalarVolume : public ScalarVolumeInterface {

  friend class DTIVolumeIO;

 private:
  ScalarVolume();

 protected:
  virtual ~ScalarVolume();
  
 public:

  // Copy constructor.

  ScalarVolume (ScalarVolume *other);

  // Constructor. Initializes the volume and allocates memory.

  ScalarVolume (int xDim, int yDim, int zDim, int cDim, double xSizeMM, double ySizeMM, double zSizeMM); 

  // Computes the FA volume given a DTI volume:

  static ScalarVolume *compute (DTIVolume *vol);

  // Get the dimensions of the FA volume.

  void getDimension (unsigned int &x, unsigned int &y, unsigned int &z, unsigned int &c) const;

  // Get the voxel dimensions (in mm)

  void getVoxelSize (double &x, double &y, double &z) const;

  // Check to see if a point is inside the volume.

  bool inBounds (const DTIVector &vec) const;

  // Set the FA at a specific point in the volume:

  void setFA (int x, int y, int z, int c, double value);

  // Get the FA at a specific point in the volume:

  double getFA (int x, int y, int z, int c) const;
  
  double *getTransformMatrix ();
  double *getInverseTransformMatrix();

  double *getSformMatrix () { return &(_stoxyz[0][0]); }
  double *getInverseSformMatrix() { return &(_stoijk[0][0]); }

  double *getQformMatrix () { return &(_qtoxyz[0][0]); }
  double *getInverseQformMatrix() { return &(_qtoijk[0][0]); }

  int getQformCode() { return _qform_code; }
  int getSformCode() { return _sform_code; }

  double *getDataPointer () { return _data; }

  double getCalcMinVal() { return _calcminval; }
  double getCalcMaxVal() { return _calcmaxval; }

  void setCalcMinVal(double calcminval) { _calcminval = calcminval; }
  void setCalcMaxVal(double calcmaxval) { _calcmaxval = calcmaxval; } 

  static ScalarVolume *resampleToAcPc (ScalarVolume *vol);

 protected:
  //  DTIFAArray *_fa_array;
 private:
  
  static ScalarVolume *resampleVolume (ScalarVolume *img, const double xform[4][4], const double bb[3][2], const double mmPerVoxOut[3]);

  double _x_size;
  double _y_size;
  double _z_size;
  
  int _dim[4];
  double *_data;
  //  double _xform_to_acpc[4][4];
  //  double _inv_xform_to_acpc[4][4];
  double _qtoxyz[4][4];
  double _qtoijk[4][4];
  double _stoxyz[4][4];
  double _stoijk[4][4];
  
  int _qform_code;
  int _sform_code;

  double _calcminval;
  double _calcmaxval;
  
};

#endif
