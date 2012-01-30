/** Copyright (c) 2003, The Board of Trustees of Leland Stanford Junior 
University. All rights reserved. **/

#include "DTIDeterministicPathTracer.h"
#include <util/DTIPathwayFactoryInterface.h>
#include <util/DTIVolumeInterface.h>
#include <util/ScalarVolumeInterface.h>
#include <util/DTIPathwayInterface.h>
#include <jama/jama_eig.h>
#include <tnt/tnt_array1d_utils.h>
#include <tnt/tnt_array2d_utils.h>
#include "DTISeedGenerator.h"

DTIDeterministicPathTracer::DTIDeterministicPathTracer(int algoID, DTISeedGenerator *seedGenerator, DTIPathwayFactoryInterface *pathwayFactory, DTIVolumeInterface *volume, ScalarVolumeInterface *faVolume, ScalarVolumeInterface *maskVolume, double faThresh, double angleThresh, double stepSizeMm) : DTIPathTracer (algoID, seedGenerator, pathwayFactory, volume, stepSizeMm) {

  _fa_volume = faVolume;
  _mask_volume = maskVolume;
  _fa_thresh = faThresh;
  _angle_thresh = angleThresh;
}

/*************************************************************************
 * Function Name: DTIDeterministicPathTracer::compute
 * Parameters: DTIVolume *volume, const DTIVector &seedPoint, const DTIVector &voxSize, double faThresh, double angleThresh, double stepSizeMm, int whichAlgorithm
 * Returns: DTIPathway *
 * Effects: Step size is in mm, vox size is in mm (each dimension) seed point
 *          is in voxel space.
 *************************************************************************/
DTIPathwayInterface *
DTIDeterministicPathTracer::computeNextPath() 

{
  static const double EPSILON_FA = 0.001;
  DTIVector seedPoint (3);
  _seed_generator->getNextSeed(seedPoint);

  DTIPathwayInterface *fiberPath = _pathway_factory->createPathway ((DTIPathwayAlgorithm)_algo_id);

  if (!inBounds(seedPoint, _volume)) {
    return fiberPath;
  }

  if(_mask_volume != NULL) {
    int x = (int) floor(seedPoint[0]+0.5);
    int y = (int) floor(seedPoint[1]+0.5);
    int z = (int) floor(seedPoint[2]+0.5);
    
    // Only processed labeled voxels or all voxels when we provide no mask
    int label = (int) floor(_mask_volume->getFA(x,y,z,0));

    if (label == 0) return fiberPath;
  }
  

  double voxSizeBuf[3];
  DTIVector voxSize (3, voxSizeBuf);

  _volume->getVoxelSize (voxSize[0], voxSize[1], voxSize[2]);

  int seedPointIndex = 0;

 
  
  fiberPath->append (seedPoint);

  for (int directionToTrace = 0; directionToTrace < 2; directionToTrace++) {
    
    int iter = 0;
    bool done = false;
    int maxIter = 1000;
    
    DTITensor seedPointTensor = _volume->getTensorInterpolate (seedPoint);
    DTIVector originalDir = getMajorEigenvector (seedPointTensor);
    
    double curPosBuf[3];
    DTIVector currentPosition(3,curPosBuf);
    
    double nextDirBuf[3];
    DTIVector nextDir(3,nextDirBuf);
    if (directionToTrace == 0) {
      nextDir = originalDir.copy();
    }
    else {
      nextDir = -originalDir.copy();
    }
    
    double dirBuf[3];
    DTIVector dir(3, dirBuf);
    
    double nextPosBuf[3];
    DTIVector nextPosition (3, nextPosBuf);
    nextPosition = seedPoint.copy();
    
    while (!done && iter < maxIter) {
      dir = nextDir.copy();
      currentPosition = nextPosition.copy();
      double fa;
      int mask0, mask1;
      if (!inBounds (currentPosition, _volume)) {
	done = true;
      }
      else {
	// Get the FA for this voxel
	fa = _fa_volume->getFA ((int) floor (currentPosition[0]+0.5),
			      (int) floor (currentPosition[1]+0.5),
			      (int) floor (currentPosition[2]+0.5), 0);
	if (fa <= _fa_thresh || fabs(dir[0]) + fabs(dir[1]) + fabs(dir[2]) < EPSILON_FA) {
	  done = true;
	  break;
	}
	
	nextPosition = currentPosition.copy() + _step_size_voxel->copy()*dir.copy();
	
	// If we have a mask image make sure we are not crossing label boundaries
	
	if( _mask_volume != NULL && inBounds (nextPosition, _volume)) {
	  mask0 = (int) floor( _mask_volume->getFA ((int) floor (currentPosition[0]+0.5),
						  (int) floor (currentPosition[1]+0.5),
						  (int) floor (currentPosition[2]+0.5),0));
	  mask1 = (int) floor( _mask_volume->getFA ((int) floor (nextPosition[0]+0.5),
						  (int) floor (nextPosition[1]+0.5),
						  (int) floor (nextPosition[2]+0.5),0));
	  
	  if(mask0 != mask1) {
	    done = true;
	    break;
	  }
	}
	
	//	DTITensor nextTensor = _volume->getTensorInterpolate (nextPosition);
	
	nextDir = computeNextDirection (dir, nextPosition); // given incoming direction, tensor, compute new direction.
	
	done = shouldTerminate (dir, nextDir);

	if (directionToTrace == 0 && inBounds (nextPosition, _volume)) {
      fiberPath->append (nextPosition);
	}
	else if (directionToTrace == 1 && inBounds (nextPosition, _volume)) {
	  fiberPath->prepend (nextPosition);
	  seedPointIndex++;
	}
	iter++;
      }
    }
  }
  fiberPath->setSeedPointIndex (seedPointIndex);
    
  return fiberPath;
  }
