/***********************************************************************
 * AUTHOR: David Akers <dakers>
 *   FILE: DTIUniformSeedGenerator.cpp
 *   DATE: Tue Jun  7 14:52:35 2005
 *  DESCR: 
 ***********************************************************************/

#include "DTIUniformSeedGenerator.h"
#include <util/DTIVolumeInterface.h>

#include <util/typedefs.h>
#include <util/DTIVolumeInterface.h>
/***********************************************************************
 *  Method: DTIUniformSeedGenerator::DTIUniformSeedGenerator
 *  Params: DTIVolumeInterface *volume, double sampleSpread
 * Effects: 
 ***********************************************************************/
DTIUniformSeedGenerator::DTIUniformSeedGenerator(DTIVolumeInterface *volume, double sampleSpread)
{
  double mmScale[3];
  volume->getVoxelSize (mmScale[0], mmScale[1], mmScale[2]);
  volume->getDimension (_x_dim, _y_dim, _z_dim);
  _x_step = sampleSpread / mmScale[0];
  _y_step = sampleSpread / mmScale[1];
  _z_step = sampleSpread / mmScale[2];
  
}


/***********************************************************************
 *  Method: DTIUniformSeedGenerator::initialize
 *  Params: 
 * Returns: void
 * Effects: 
 ***********************************************************************/
void
DTIUniformSeedGenerator::initialize()
{
  _x = 0;
  _y = 0;
  _z = 0;
}


/***********************************************************************
 *  Method: DTIUniformSeedGenerator::getNextSeed
 *  Params: DTIVector &pt
 * Returns: void
 * Effects: 
 ***********************************************************************/
void
DTIUniformSeedGenerator::getNextSeed(DTIVector &pt)
{
  _x += _x_step;
  if (_x > _x_dim) {
    _x = 0;
    _y += _y_step;
    if (_y > _y_dim) {
      _y = 0;
      _z += _z_step;
    }
  }
  pt[0] = _x;
  pt[1] = _y;
  pt[2] = _z;
}


/***********************************************************************
 *  Method: DTIUniformSeedGenerator::finished
 *  Params: 
 * Returns: bool
 * Effects: 
 ***********************************************************************/
bool
DTIUniformSeedGenerator::finished() const
{
  if (_z > _z_dim) {
    return true;
  }
  else {
    return false;
  }
}


