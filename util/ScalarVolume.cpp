/** Copyright (c) 2005, The Board of Trustees of Leland Stanford Junior 
University. All rights reserved. **/

/*************************************************************************
 *    NAME: David Akers and Anthony Sherbondy
 *     
 *    FILE: ScalarVolume.cpp
 *    DATE: Wed Mar  3  6:27:07 PST 2004
 *************************************************************************/
#include "ScalarVolume.h"
#include "DTIVolume.h"
#include <jama/jama_eig.h>
#include "typedefs.h"
#include <tnt/tnt_array1d_utils.h>
#include <math.h>

/*************************************************************************
 * Function Name: ScalarVolume::ScalarVolume
 * Parameters: 
 * Effects: 
 *************************************************************************/

ScalarVolume::ScalarVolume()
{
  _x_size = _y_size = _z_size = 0;
  _data = NULL;
  setCalcMinVal (0);
  setCalcMaxVal (0);
}


/*************************************************************************
 * Function Name: ScalarVolume::~ScalarVolume
 * Parameters: 
 * Effects: 
 *************************************************************************/

ScalarVolume::~ScalarVolume()
{
  delete []_data;
  //  delete _fa_array;
}


/*************************************************************************
 * Function Name: DTIVolume::inBounds
 * Parameters: const DTIVector &vec
 * Returns: bool
 * Effects: 
 *************************************************************************/
bool
ScalarVolume::inBounds(const DTIVector &vec) const
{
  unsigned int x,y,z,c;
  getDimension(x,y,z,c);
  if (vec[0] < 0 || vec[0] > x-1 ||
      vec[1] < 0 || vec[1] > y-1 ||
      vec[2] < 0 || vec[2] > z-1)
    return false;
  else
    return true;

}

/*************************************************************************
 * Function Name: ScalarVolume::compute
 * Parameters: DTIVolume *vol
 * Returns: ScalarVolume *
 * Effects: 
 *************************************************************************/
ScalarVolume *
ScalarVolume::compute(DTIVolume *vol)
{
  unsigned int xDim,yDim,zDim;
  vol->getDimension (xDim,yDim,zDim);
  double xSize, ySize, zSize;
  vol->getVoxelSize (xSize,ySize,zSize);
  double *data = new double[xDim*yDim*zDim];

  ScalarVolume *scalarVolume = new ScalarVolume();
  scalarVolume->_x_size = xSize;
  scalarVolume->_y_size = ySize;
  scalarVolume->_z_size = zSize;

  scalarVolume->_dim[0] = xDim;
  scalarVolume->_dim[1] = yDim;
  scalarVolume->_dim[2] = zDim;
  scalarVolume->_dim[3] = 1;

  scalarVolume->_qform_code = vol->getQformCode();
  scalarVolume->_sform_code = vol->getSformCode();

  double *transformMx = vol->getTransformMatrix();
  double *thisMx = scalarVolume->getTransformMatrix();
  for (int i = 0; i < 16; i++) {
    (scalarVolume->getQformMatrix())[i] = (vol->getQformMatrix())[i];
     (scalarVolume->getInverseQformMatrix())[i] = (vol->getInverseQformMatrix())[i];
     (scalarVolume->getSformMatrix())[i] = (vol->getSformMatrix())[i];
     (scalarVolume->getInverseSformMatrix())[i] = (vol->getInverseSformMatrix())[i];
  }

  //std::cerr << "scalar volume created with dims: " << scalarVolume->_dim[0] << ", " << scalarVolume->_dim[1] << ", " << scalarVolume->_dim[2] << ", " << scalarVolume->_dim[3] << endl;

  scalarVolume->_data = data;
  for (int x = 0; x < xDim; x++) {
    for (int y = 0; y < yDim; y++) {
      for (int z = 0; z < zDim; z++) {
	TNT::Array1D<double> eigenvalues;
	DTITensor tensor = vol->getTensor(x,y,z);
	JAMA::Eigenvalue<double> eig(tensor);
	eig.getRealEigenvalues (eigenvalues);
	
	double epsilon = 1e-10;
	double meanDiffusivity = (eigenvalues[0] + eigenvalues[1] + eigenvalues[2])/3.0; 
	TNT::Array1D<double> meanDiff1D (3, meanDiffusivity);
	TNT::Array1D<double> differences = eigenvalues - meanDiff1D;
	double stdevDiffusivity = sqrt(dproduct (differences, differences));
	
	double normDiffusivity = sqrt(dproduct (eigenvalues, eigenvalues));
	
	if (normDiffusivity > epsilon) {
	  scalarVolume->setFA (x,y,z,0,sqrt(3.0/2.0) * stdevDiffusivity / normDiffusivity);
	}
	else {
	  scalarVolume->setFA (x,y,z,0,0.0);
	}
      }
    }
  }
  return scalarVolume;
}


/*************************************************************************
 * Function Name: ScalarVolume::getDimension
 * Parameters: int &x, int &y, int &z
 * Returns: void
 * Effects: 
 *************************************************************************/
void
ScalarVolume::getDimension(unsigned int &x, unsigned int &y, unsigned int &z, unsigned int &c) const
{
  x = _dim[0];
  y = _dim[1];
  z = _dim[2];
  c = _dim[3];
}


/*************************************************************************
 * Function Name: ScalarVolume::getFA
 * Parameters: int x, int y, int z
 * Returns: double
 * Effects: 
 *************************************************************************/
double
ScalarVolume::getFA(int x, int y, int z, int c) const
{
  //  return (*_fa_array)[x][y][z];
  return _data[z*_dim[0]*_dim[1]*_dim[3] + y*_dim[0]*_dim[3] + x*_dim[3] + c];
}


/*************************************************************************
 * Function Name: ScalarVolume::ScalarVolume
 * Parameters: int x, int y, int z
 * Effects: 
 *************************************************************************/

ScalarVolume::ScalarVolume(int x, int y, int z, int c, double xSize, double ySize, double zSize)
{
  _data = new double [x*y*z*c];
  memset (_data, 0, sizeof (double) * x*y*z*c);
  _x_size = xSize; _y_size = ySize; _z_size = zSize;
  _dim[0] = x;
  _dim[1] = y;
  _dim[2] = z;
  _dim[3] = c;
  setCalcMinVal (0);
  setCalcMaxVal (0);
}







/*************************************************************************
 * Function Name: ScalarVolume::setFA
 * Parameters: int x, int y, int z, double value
 * Returns: void
 * Effects: 
 *************************************************************************/
void
ScalarVolume::setFA(int x, int y, int z, int c, double value)
{
  unsigned int xDim, yDim, zDim, cDim;
  getDimension (xDim, yDim, zDim, cDim);
  assert (x < xDim && x >= 0 &&
	  y < yDim && y >= 0 &&
	  z < zDim && z >= 0 &&
	  c < cDim && c >= 0);

  _data[z*_dim[0]*_dim[1]*_dim[3] + y*_dim[0]*_dim[3] + x*_dim[3] + c] = value;
}


 // Get the voxel dimensions (in mm)

/*************************************************************************
 * Function Name: ScalarVolume::getVoxelSize
 * Parameters: double x, double y, double z
 * Returns: void
 * Effects: 
 *************************************************************************/

void 
ScalarVolume::getVoxelSize (double &x, double &y, double &z) const 
{
  x = _x_size;
  y = _y_size;
  z = _z_size;
}








/***********************************************************************
 *  Method: ScalarVolume::resampleToAcPc
 *  Params: ScalarVolume *vol, const double mmPerVoxOut[3]
 * Returns: ScalarVolume *
 * Effects: 
 ***********************************************************************/
ScalarVolume *
ScalarVolume::resampleToAcPc(ScalarVolume *vol)
{
  double bb[][2] = {{-78, 78},
		    {-120, 80},
		    {-60, 85}};
  double mmPerVox[3] = {vol->_x_size, vol->_y_size, vol->_z_size};

  //  return resampleVolume (vol, vol->_inv_xform_to_acpc, bb, mmPerVox);
  return NULL;

}


/***********************************************************************
 *  Method: ScalarVolume::resampleVolume
 *  Params: ScalarVolume *img, const double xform[4][4], const double bb[6], const double mmPerVoxOut[3]
 * Returns: ScalarVolume *
 * Effects: 
 ***********************************************************************/
ScalarVolume *
ScalarVolume::resampleVolume(ScalarVolume *img, const double xform[4][4], const double bb[3][2], const double mmPerVoxOut[3])
{
  int xdim = (int) floor((bb[0][1]-bb[0][0]) / mmPerVoxOut[0]);
  int ydim = (int) floor((bb[1][1]-bb[1][0]) / mmPerVoxOut[1]);
  int zdim = (int) floor((bb[2][1]-bb[2][0]) / mmPerVoxOut[2]);
  
  int cdim = img->_dim[3];

  ScalarVolume *newVol = new ScalarVolume(xdim, ydim, zdim, cdim, mmPerVoxOut[0], mmPerVoxOut[1], mmPerVoxOut[2]);
  // generate a grid of values, transform them all by the xform, use transformed coords to look up values in the original image and copy them into the new one.
  return newVol;
}


/***********************************************************************
 *  Method: ScalarVolume::getTransformMatrix
 *  Params: 
 * Returns: double *
 * Effects: 
 ***********************************************************************/
double *
ScalarVolume::getTransformMatrix()
{
  // first choice is sform matrix
  if (_sform_code > 0) {
    return getSformMatrix();
  }
  else {
    return getQformMatrix();
  }
}


/***********************************************************************
 *  Method: ScalarVolume::getInverseTransformMatrix
 *  Params: 
 * Returns: double *
 * Effects: 
 ***********************************************************************/
double *
ScalarVolume::getInverseTransformMatrix()
{
  // first choice is sform matrix
  if (_sform_code > 0) {
    return getInverseSformMatrix();
  }
  else {
    return getInverseQformMatrix();
  }
}


/*************************************************************************
 * Function Name: ScalarVolume::ScalarVolume
 * Parameters: 
 * Effects: 
 *************************************************************************/

ScalarVolume::ScalarVolume(ScalarVolume *other)
{

  // don't copy these parts
  setCalcMinVal (0);
  setCalcMaxVal (0);

  _x_size = other->_x_size;
  _y_size = other->_y_size;
  _z_size = other->_z_size;

  unsigned int xDim,yDim,zDim,cDim;
  other->getDimension(xDim, yDim, zDim, cDim);
  _data = new double[xDim*yDim*zDim*cDim];

  _dim[0] = other->_dim[0];
  _dim[1] = other->_dim[1];
  _dim[2] = other->_dim[2];
  _dim[3] = other->_dim[3];

  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      _qtoxyz[i][j] = other->_qtoxyz[i][j];
      _qtoijk[i][j] = other->_qtoijk[i][j];
      _stoxyz[i][j] = other->_stoxyz[i][j];
      _stoijk[i][j] = other->_stoijk[i][j];
    }
  }
  _qform_code = other->_qform_code;
  _sform_code = other->_sform_code;
}

