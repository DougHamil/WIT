#include "DTITENDPathTracer.h"
#include <jama/jama_eig.h>
#include <tnt/tnt_array1d_utils.h>
#include <tnt/tnt_array2d_utils.h>
#include "DTIVolumeInterface.h"

DTITENDPathTracer::DTITENDPathTracer(int algoID, DTISeedGenerator *seedGenerator, DTIPathwayFactoryInterface *pathwayFactory, DTIVolumeInterface *volume, ScalarVolumeInterface *faVolume, ScalarVolumeInterface *maskVolume, double faThresh, double angleThresh, double stepSizeMm) : DTIDeterministicPathTracer (algoID, seedGenerator, pathwayFactory, volume, faVolume, maskVolume, faThresh, angleThresh, stepSizeMm)

{
}

DTIVector 
DTITENDPathTracer::computeNextDirection (const DTIVector &incoming,
				       const DTIVector &currentPosition)
{
  return computeTENDDirection (incoming, _volume->getTensorInterpolate (currentPosition));
}

DTIVector 
DTITENDPathTracer::computeTENDDirection (const DTIVector &incoming,
				       const DTITensor &tensor)
{
  static const double EPSILON_MAG = 0.001;
  // Need to convert dir to an Array2D
  double buf[3];
  TNT::Array2D<double> dirArray2D(3,1,buf); 
  dirArray2D[0][0] = incoming[0];
  dirArray2D[1][0] = incoming[1];
  dirArray2D[2][0] = incoming[2];
  double bufresult[3];
  TNT::Array2D<double> tendDir2D(3,1,bufresult);
  tendDir2D = TNT::matmult(tensor, dirArray2D);
  
  DTIVector tendDir(3);
  tendDir[0] = tendDir2D[0][0];
  tendDir[1] = tendDir2D[0][1];
  tendDir[2] = tendDir2D[0][2];
  
  double mag = abs(tendDir);
  
  if (mag > EPSILON_MAG) {
    tendDir[0] /= mag;
    tendDir[1] /= mag;
    tendDir[2] /= mag;
  }
  return tendDir;
}

bool 
DTITENDPathTracer::shouldTerminate (const DTIVector &incoming,
				   const DTIVector &outgoing)
{
  if (dproduct(incoming,outgoing) < 0.0) {
    return true;
    // don't allow tend to reverse directions
    // can this only happen with negative eigenvalues?
  }
  else {
    return false; 
  }
}
