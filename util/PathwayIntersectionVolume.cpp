/***********************************************************************
 * AUTHOR: David Akers <dakers>
 *   FILE: .//PathwayIntersectionVolume.cpp
 *   DATE: Thu Jul  7 13:28:22 2005
 *  DESCR: 
 ***********************************************************************/
#include "PathwayIntersectionVolume.h"
#include "DTIPathway.h"
#include <math.h>
#include <iostream>
using namespace std;


/***********************************************************************
 *  Method: PathwayIntersectionVolume::munchPathway
 *  Params: DTIPathway *pathway
 * Returns: void
 * Effects: 
 ***********************************************************************/
void
PathwayIntersectionVolume::munchPathway(DTIPathway *pathway)
{
  for (int i = 0; i < pathway->getNumPoints(); i++) {
    static double oldPt[3];
    static double pt[3];
    oldPt[0] = pt[0];
    oldPt[1] = pt[1];
    oldPt[2] = pt[2];
    pathway->getPoint (i, pt);
    if (i > 0) {
      rasterizeSegment (oldPt, pt);
    }
  }
}

#if 0

/***********************************************************************
 *  Method: PathwayIntersectionVolume::munchPoint
 *  Params: const double pt[3]
 * Returns: void
 * Effects: 
 ***********************************************************************/
void
PathwayIntersectionVolume::munchPoint(const double ptInput[3])
{
  unsigned int xSize, ySize, zSize, cSize;
  getDimension(xSize, ySize, zSize, cSize);
  
  double *inverseMx = faVol->getInverseTransformMatrix();

  double pt[4] = {ptInput[0], ptInput[1], ptInput[2], 1};
  double ptVoxSpace[4];

  ptVoxSpace[0] = inverseMx[0]*pt[0] + inverseMx[1]*pt[1]
    + inverseMx[2]*pt[2] + inverseMx[3];
  ptVoxSpace[1] = inverseMx[4]*pt[0] + inverseMx[5]*pt[1]
    + inverseMx[6]*pt[2] + inverseMx[7];
  ptVoxSpace[2] = inverseMx[8]*pt[0] + inverseMx[9]*pt[1]
    + inverseMx[10]*pt[2] + inverseMx[11];

  int x = (int) floor(ptVoxSpace[0]+0.5);
  int y = (int) floor(ptVoxSpace[1]+0.5);
  int z = (int) floor(ptVoxSpace[2]+0.5);

  assert (xCoord < xSize);
  assert (yCoord < ySize);
  assert (zCoord < zSize);
  assert (xCoord >= 0);
  assert (yCoord >= 0);
  assert (zCoord >= 0);
  //  cerr << "PathwayIntersectionVolume::munchPoint(const double pt[3]) is disabled" << endl;
  double oldCount = getFA (xCoord, yCoord, zCoord);
  setFA (xCoord, yCoord, zCoord, oldCount+1);

  //  (*_fa_array)[xCoord][yCoord][zCoord] = (*_fa_array)[xCoord][yCoord][zCoord] + 1;
  //  if ((*_fa_array)[xCoord][yCoord][zCoord] > _max_intersect) {
  //    _max_intersect = (*_fa_array)[xCoord][yCoord][zCoord];
  //  }
}
#endif

/***********************************************************************
 *  Method: PathwayIntersectionVolume::normalize
 *  Params: 
 * Returns: void
 * Effects: 
 ***********************************************************************/
void
PathwayIntersectionVolume::normalize()
{
  double maxValue = getFA (0,0,0,0);
 
  unsigned int xSize, ySize, zSize, cDim;
  getDimension(xSize, ySize, zSize, cDim);
  for (int x = 0; x < xSize; x++) {
    for (int y = 0; y < ySize; y++) {
      for (int z = 0; z < zSize; z++) {
	double value = getFA (x,y,z,0);
	if (value > maxValue) {
	  maxValue = value;
	}
	//(*_fa_array)[x][y][z] /= _max_intersect;
      }
    }
  }
  //  setSlope (1/maxValue);
  setCalcMinVal (0);
  setCalcMaxVal (maxValue);
  
}


/***********************************************************************
 *  Method: PathwayIntersectionVolume::PathwayIntersectionVolume
 *  Params: const unsigned int intersectionVolumeDimensions[3], const unsigned int pathwayDimensions[3], const double pathwayVoxSize[3]
 * Effects: 
 ***********************************************************************/
PathwayIntersectionVolume::PathwayIntersectionVolume(ScalarVolume *currentBackground) : ScalarVolume (currentBackground)
{
  _max_intersect = 0;

}







void swapPoints (double a[3], double b[3]) {
  static double c[3];
  c[0] = b[0];
  c[1] = b[1];
  c[2] = b[2];
  b[0] = a[0];
  b[1] = a[1];
  b[2] = a[2];
  a[0] = c[0];
  a[1] = c[1];
  a[2] = c[2];
}

/***********************************************************************
 *  Method: PathwayIntersectionVolume::rasterizeSegment
 *  Params: const double a[3], const double b[3]
 * Returns: void
 * Effects: 
 ***********************************************************************/
void
PathwayIntersectionVolume::rasterizeSegment(const double a[3], const double b[3])
{
  double aCopy[3] = {a[0], a[1], a[2]};
  double bCopy[3] = {b[0], b[1], b[2]};

  //  cerr << "rasterizeSegment (" << aCopy[0] << ", " << aCopy[1] << ", " << aCopy[2] << "  " << bCopy[0] << ", " << bCopy[1] << ", " << bCopy[2] << ")" << endl;

  double dx = (aCopy[0]-bCopy[0]);
  double dy = (aCopy[1]-bCopy[1]);
  double dz = (aCopy[2]-bCopy[2]);

  bool iterateX = false;
  bool iterateY = false;
  bool iterateZ = false;

  if (fabs(dx) >= fabs(dy) && fabs(dx) >= fabs(dz)) {
    iterateX = true;
    if (aCopy[0] > bCopy[0]) {
      swapPoints (aCopy,bCopy);
      dy = -dy;
      dz = -dz;
    }
  }
  else if (fabs(dy) >= fabs(dx) && fabs(dy) >= fabs(dz)) {
    iterateY = true;
    if (aCopy[1] > bCopy[1]) {
      swapPoints (aCopy,bCopy);
      dx = -dx;
      dz = -dz;
    }
  }
  else if (fabs(dz) >= fabs(dx) && fabs(dz) >= fabs(dy)) {
    iterateZ = true;
    if (aCopy[2] > bCopy[2]) {
      swapPoints (aCopy,bCopy);
      dx = -dx;
      dy = -dy;
    }
  }

  double x = aCopy[0];
  double y = aCopy[1];
  double z = aCopy[2];

  bool sign;

  if (iterateX) {
    // iterate over x:
    double yStep = dy/dx;
    double zStep = dz/dx;
    while (x < bCopy[0]) {
      // calculate y, z:
      setVoxel (x,y,z);
      if (x > bCopy[0]-1.0) {
	x = bCopy[0];
      }
      else {
	x += 1.0;
      }
      y += yStep;
      z += zStep;
    }
  }
  else if (iterateY) {
    // iterate over y:
    double xStep = dx/dy;
    double zStep = dz/dy;
    while (y < bCopy[1]) {
      // calculate y, z:
      setVoxel (x,y,z);
      if (y > bCopy[1]-1) {
	y = bCopy[1];
      }
      else {
	y += 1.0;
      }
      x += xStep;
      z += zStep;
    }
  }
  else if (iterateZ) {
    // iterate over z:
    double xStep = dx/dz;
    double yStep = dy/dz;
    while (z < bCopy[2]) {
      // calculate x, y:
      setVoxel (x,y,z);
      if (z > bCopy[2]-1) {
	z = bCopy[2];
      }
      else {
	z += 1.0;
      }
      x += xStep;
      y += yStep;
    }
  }
}


/***********************************************************************
 *  Method: PathwayIntersectionVolume::setVoxel
 *  Params: double x, double y, double z
 * Returns: void
 * Effects: 
 ***********************************************************************/
void
PathwayIntersectionVolume::setVoxel(double x, double y, double z)
{
  //  cerr << "setVoxel (" << x << ", " << y << ", " << z << ")" << endl;

  unsigned int xDim, yDim, zDim, cDim;
  getDimension(xDim, yDim, zDim, cDim);

  static int lastX = -1000;
  static int lastY = -1000;
  static int lastZ = -1000;

  double *inverseMx = getInverseTransformMatrix();

  double pt[4] = {x, y, z, 1};
  double ptVoxSpace[4];

  ptVoxSpace[0] = inverseMx[0]*pt[0] + inverseMx[1]*pt[1]
    + inverseMx[2]*pt[2] + inverseMx[3];
  ptVoxSpace[1] = inverseMx[4]*pt[0] + inverseMx[5]*pt[1]
    + inverseMx[6]*pt[2] + inverseMx[7];
  ptVoxSpace[2] = inverseMx[8]*pt[0] + inverseMx[9]*pt[1]
    + inverseMx[10]*pt[2] + inverseMx[11];

  int xCoord = (int) floor(ptVoxSpace[0]+0.5);
  int yCoord = (int) floor(ptVoxSpace[1]+0.5);
  int zCoord = (int) floor(ptVoxSpace[2]+0.5);

  if (xCoord == lastX && yCoord == lastY && zCoord == lastZ) {
    return;
  }
  else {
    lastX = xCoord;
    lastY = yCoord;
    lastZ = zCoord;
  }
  //  cerr << "anat:" << endl;
  //  cerr << x << ", " << y << ", " << z << endl;
  //  cerr << "vox:" << endl;
  //  cerr << xCoord << ", " << yCoord << ", " << zCoord << endl;
  //  if (xCoord >= xSize || 
//        yCoord >= ySize ||
//        zCoord >= zSize ||
//        xCoord < 0 ||
//        yCoord < 0 ||
//        zCoord < 0) {
//      return;
//    }
  
  assert (xCoord < xDim);
  assert (yCoord < yDim);
  assert (zCoord < zDim);
  assert (xCoord >= 0);
  assert (yCoord >= 0);
  assert (zCoord >= 0);

  double oldCount = getFA (xCoord, yCoord, zCoord, 0);
  setFA (xCoord, yCoord, zCoord, 0, oldCount+1);
}


