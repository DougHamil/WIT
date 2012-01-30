/** Copyright (c) 2005, The Board of Trustees of Leland Stanford Junior 
University. All rights reserved. **/

/*************************************************************************
 *    NAME: David Akers and Anthony Sherbondy
 *     
 *    FILE: DTIVolume.cpp
 *    DATE: Mon Feb 23  1:48:00 PST 2004
 *************************************************************************/
#include "DTIVolume.h"
#include <math.h>

/*************************************************************************
 * Function Name: DTIVolume::getDimension
 * Parameters: int &x, int &y, int &z
 * Returns: void
 * Effects: 
 *************************************************************************/
void
DTIVolume::getDimension(unsigned int &x, unsigned int &y, unsigned int &z) const
{
  x = _tensor_array->dim1();
  y = _tensor_array->dim2();
  z = _tensor_array->dim3();
}


/*************************************************************************
 * Function Name: DTIVolume::getVoxelSize
 * Parameters: int &x, int &y, int &z
 * Returns: void
 * Effects: 
 *************************************************************************/
void
DTIVolume::getVoxelSize(double &x, double &y, double &z) const
{
  x = _x_size;
  y = _y_size;
  z = _z_size;
}

/*************************************************************************
 * Function Name: DTIVolume::getVoxelSize
 * Parameters: int &x, int &y, int &z
 * Returns: void
 * Effects: 
 *************************************************************************/
void
DTIVolume::setVoxelSize(double x, double y, double z) 
{
  _x_size = x;
  _y_size = y;
  _z_size = z;
}

/*************************************************************************
 * Function Name: DTIVolume::getTensor
 * Parameters: int x, int y, int z
 * Returns: DTITensor
 * Effects: 
 *************************************************************************/

DTITensor
DTIVolume::getTensor(int x, int y, int z) const 
{
  return (*_tensor_array)[x][y][z];
}


/*************************************************************************
 * Function Name: DTIVolume::getTensor
 * Parameters: const DTIVector &position
 * Returns: DTITensor
 * Effects: 
 *************************************************************************/
DTITensor
DTIVolume::getTensorInterpolate(const DTIVector &position) const
{
  double xFrac, yFrac, zFrac;
  xFrac = position[0] - floor (position[0]);
  yFrac = position[1] - floor (position[1]);
  zFrac = position[2] - floor (position[2]);

  int x, y, z;
  x = (int) floor (position[0]);
  y = (int) floor (position[1]);
  z = (int) floor (position[2]);
  if (x < 0) x = 0;
  if (y < 0) y = 0;
  if (z < 0) z = 0;
  //std::cerr << "xdim: " << _tensor_array->dim1() << std::endl;
  if (x > _tensor_array->dim1()-3) {
    x = _tensor_array->dim1()-2;
  }
   if (y > _tensor_array->dim2()-3) {
    y = _tensor_array->dim2()-2;
  }
   if (z > _tensor_array->dim3()-3) {
    z = _tensor_array->dim3()-2;
  }
  
   //std::cerr << "x,y,z after: " << x << ", " << y << ", " << z << std::endl;
  

  DTITensor result(3,3);
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      result[i][j] = 
	((*_tensor_array)[x][y][z])[i][j]*(1-xFrac)*(1-yFrac)*(1-zFrac) +
	((*_tensor_array)[x+1][y][z])[i][j]*(xFrac)*(1-yFrac)*(1-zFrac) +
	((*_tensor_array)[x][y+1][z])[i][j]*(1-xFrac)*(yFrac)*(1-zFrac) +
	((*_tensor_array)[x][y][z+1])[i][j]*(1-xFrac)*(1-yFrac)*(zFrac) +
	((*_tensor_array)[x+1][y][z+1])[i][j]*(xFrac)*(1-yFrac)*(zFrac) +
	((*_tensor_array)[x][y+1][z+1])[i][j]*(1-xFrac)*(yFrac)*(zFrac) +
	((*_tensor_array)[x+1][y+1][z])[i][j]*(xFrac)*(yFrac)*(1-zFrac) +
	((*_tensor_array)[x+1][y+1][z+1])[i][j]*(xFrac)*(yFrac)*(zFrac);
    }
  }
  return result;
  //return DTITensor(3,3);
}



/*************************************************************************
 * Function Name: DTIVolume::inBounds
 * Parameters: const DTIVector &vec
 * Returns: bool
 * Effects: 
 *************************************************************************/
bool
DTIVolume::inBounds(const DTIVector &vec) const
{
  unsigned int x,y,z;
  getDimension(x,y,z);
  if (vec[0] < 0 || vec[0] > x-1 ||
      vec[1] < 0 || vec[1] > y-1 ||
      vec[2] < 0 || vec[2] > z-1)
    return false;
  else
    return true;

}


/*************************************************************************
 * Function Name: DTIVolume::DTIVolume
 * Parameters: 
 * Effects: 
 *************************************************************************/

DTIVolume::DTIVolume()
{
  _tensor_array = NULL;
}


/*************************************************************************
 * Function Name: DTIVolume::~DTIVolume
 * Parameters: 
 * Effects: 
 *************************************************************************/

DTIVolume::~DTIVolume()
{
  // xxx should also delete the data here - but there seems to be no easy way
  // to do this, because of the way TNT memory management works. GRR.
  
  delete _tensor_array;
}


/*************************************************************************
 * Function Name: DTIVolume::setTensor
 * Parameters: int x, int y, int z, const double data[6]
 * Returns: void
 * Effects: 
 *************************************************************************/
void
DTIVolume::setTensor(int x, int y, int z, const double data[6])
{
  // Untested
  unsigned int xDim, yDim, zDim;
  getDimension (xDim, yDim, zDim);
  assert (x < xDim && x >= 0 &&
	  y < yDim && y >= 0 &&
	  z < zDim && z >= 0);

  (*_tensor_array)[x][y][z][0][0] = data[0];
  (*_tensor_array)[x][y][z][1][1] = data[1];
  (*_tensor_array)[x][y][z][2][2] = data[2];
  (*_tensor_array)[x][y][z][0][1] = data[3];
  (*_tensor_array)[x][y][z][1][0] = data[3];
  (*_tensor_array)[x][y][z][0][2] = data[4];
  (*_tensor_array)[x][y][z][2][0] = data[4];
  (*_tensor_array)[x][y][z][1][2] = data[5];
  (*_tensor_array)[x][y][z][2][1] = data[5];
  
  
}


/*************************************************************************
 * Function Name: DTIVolume::DTIVolume
 * Parameters: int x, int y, int z
 * Effects: 
 *************************************************************************/

DTIVolume::DTIVolume(int xdim, int ydim, int zdim)
{
  // untested

  DTITensor *tensors = new DTITensor[xdim*ydim*zdim];
  _tensor_array = new DTITensorArray(xdim,ydim,zdim,tensors);

  for (int i = 0; i < xdim; i++) {
    for (int j = 0; j < ydim; j++) {
      for (int k = 0; k < zdim; k++) {
	double *tens = new double[9];
	for (int l = 0; l < 9; l++) {
	  tens[l] = 0.0;
	}
	(*_tensor_array)[i][j][k] = DTITensor (3, 3, tens);
      }
    }
  }

}







/***********************************************************************
 *  Method: DTIVolume::getTransformMatrix
 *  Params: 
 * Returns: double *
 * Effects: 
 ***********************************************************************/
double *
DTIVolume::getTransformMatrix()
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
 *  Method: DTIVolume::getInverseTransformMatrix
 *  Params: 
 * Returns: double *
 * Effects: 
 ***********************************************************************/
double *
DTIVolume::getInverseTransformMatrix()
{
  // first choice is sform matrix
  if (_sform_code > 0) {
    return getInverseSformMatrix();
  }
  else {
    return getInverseQformMatrix();
  }
}


