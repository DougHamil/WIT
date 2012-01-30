#include "DTISTTPathTracer.h"
#include <iostream>
#include <util/typedefs.h>
#include <jama/jama_eig.h>
#include <tnt/tnt_array1d_utils.h>
#include <tnt/tnt_array2d_utils.h>
#include "typedefs.h"
#include "DTIVolumeInterface.h"

DTISTTPathTracer::DTISTTPathTracer(int algoID, DTISeedGenerator *seedGenerator, DTIPathwayFactoryInterface *pathwayFactory, DTIVolumeInterface *volume, ScalarVolumeInterface *faVolume, ScalarVolumeInterface *maskVolume, double faThresh, double angleThresh, double stepSizeMm, NumericalIntegrationMethod method) : DTIDeterministicPathTracer (algoID, seedGenerator, pathwayFactory, volume, faVolume, maskVolume, faThresh, angleThresh, stepSizeMm)

{
  _integration_method = method;
}

DTIVector 
DTISTTPathTracer::computeNextDirection (const DTIVector &incoming,
				       const DTIVector &curPosition)
{
  DTIVector factDir(3);
  factDir = numericalIntegrate (incoming, curPosition, _integration_method);
  //  factDir = getMajorEigenvector (tensor);

  int direction;
  double angle;
  
  bool smallAngle = checkAngle (incoming, factDir, _angle_thresh, direction, angle);
  if (smallAngle && direction == -1) {
    factDir = -factDir.copy();
  }
  return factDir;
}

bool 
DTISTTPathTracer::shouldTerminate (const DTIVector &incoming,
				   const DTIVector &outgoing)
{
  int direction;
  double angle;
  bool smallAngle = checkAngle (incoming, outgoing, _angle_thresh, direction, angle);
  return !smallAngle;
}

/*************************************************************************
 * Function Name: DTISTTPathTracer::checkAngle
 * Parameters: const DTIVector &a, const DTIVector &b, double angleThresh, int &direction, double &angle
 * Returns: bool
 * Effects: 
 *************************************************************************/
bool
DTISTTPathTracer::checkAngle(const DTIVector &a, const DTIVector &b, double angleThresh, int &direction, double &angle)
{
  // Checks the angle between 2 vectors, returns if angle is less than thresh
  // The angle between two vectors is given by acos(aDOTb/{mag(a)*mag(b)})  

  double dprod = dproduct(a,b);
  if (dprod > 1.0) dprod = 1.0;
  if (dprod < -1.0) dprod = -1.0;

  bool angleCheck = false;
  double anglePos = 180*acos(dprod)/M_PI; // In degrees; both vectors are unit vectors
  double angleNeg = 180-anglePos;
 
  angle = min(abs(anglePos), abs(angleNeg));
  int i;
  if (abs(anglePos) < abs(angleNeg)) {
    i = 0;
  }
  else {
    i = 1;
  }
  direction = 0;
  if (angle <= angleThresh) {
    angleCheck = true; // Angle is within permissable range
    if (i==1) {
      direction = -1; // Use FACT vector in opposite direction
    }
  }
  else {
    angleCheck = false;
  }
  return angleCheck;
}

DTIVector
DTISTTPathTracer::numericalIntegrate(const DTIVector &incoming,
				     const DTIVector &curPosition,
				     NumericalIntegrationMethod method)
{
  switch (method) {
  case INTEGRATION_EULER:
    return getMajorEigenvector (_volume->getTensorInterpolate (curPosition)).copy();
    break;
  case INTEGRATION_RUNGE_KUTTA_2:
    return integrateRK2(incoming, curPosition);
    break;
  case INTEGRATION_RUNGE_KUTTA_4:
    return integrateRK4(incoming, curPosition);
    break;
  default:
    break;
  };
}

DTIVector 
DTISTTPathTracer::integrateRK2 (const DTIVector &start_slope,
				const DTIVector &start_position) 
{
  /*-- temporary array --*/
  DTIVector midpoint_guess(3);

  /* return array */
  double *data = new double[3];
  DTIVector midpoint_slope(3, data);

  // Use the slope at the start of the interval to guess the function
  // value at the midpoint.
  for (int i = 0; i < 3; i++ ) {
    midpoint_guess[i] = start_position[i] + start_slope[i] * (*_step_size_voxel)[i]/2.0;
  }

  // get the slope at the midpoint guess
  //  problem->evaluate( midpoint_guess, _time + _timestep/2.0, midpoint_slope );
  DTIVector slopeVec = getMajorEigenvector(_volume->getTensorInterpolate (midpoint_guess)).copy();

  for (int i = 0; i < 3; i++) {
    midpoint_slope[i] = slopeVec[i];
  }
  return midpoint_slope;
}

void alignVector (DTIVector &toAlign, const DTIVector &relativeVector)
{
  if (dproduct(toAlign, relativeVector) < 0) {
    toAlign = -toAlign.copy();
  }
}

DTIVector 
DTISTTPathTracer::integrateRK4 (const DTIVector &start_slope,
				const DTIVector &start_position) 
{
  unsigned int i;
  /*-- temporary arrays --*/
  DTIVector y(3);  // temp vector for holdnig function values
  DTIVector k1(3); // slope at start of interval
  DTIVector k2(3); // slope at middle of interval (1st estimate)
  DTIVector k3(3); // slope at middle of interval (2nd estimate)
  DTIVector k4(3); // slope at end of interval
  
  /*-- pass 1 --*/
  k1 = start_slope.copy();
  //    problem->evaluate( _state, _time, k1 );  /* k1 <- slope at the start of the interval */
  for ( i=0; i < 3; i++ ) {    /* use k1 for first guess of function midpoint */
    y[i] = start_position[i] + k1[i] * (*_step_size_voxel)[i]/2.0;
  }
  
  /*-- pass 2 --*/
  
  k2 = getMajorEigenvector (_volume->getTensorInterpolate (y)).copy();
  alignVector (k2, start_slope);

  //    problem->evaluate( y, _time + _timestep/2.0, k2 );  /* k2 <- slope at first midpoint guess */
  for ( i=0; i < 3; i++ ) {             /* use k2 for second guess of function midpoint */
    y[i] = start_position[i] + k2[i] * (*_step_size_voxel)[i]/2.0;
  }
  
  /*- pass 3 --*/
  //    problem->evaluate( y, _time + _timestep/2.0, k3 );  /* k3 <- slope at 2nd midpoint guess */
  k3 = getMajorEigenvector (_volume->getTensorInterpolate (y)).copy();
  alignVector (k3, start_slope);

  for ( i=0; i < 3; i++ ) {             /* use k3 to estimate function at end of interval */
    y[i] = start_position[i] + k3[i] * (*_step_size_voxel)[i];
  }
  
  /*-- pass 4 --*/
  k4 = getMajorEigenvector (_volume->getTensorInterpolate(y)).copy();
  alignVector (k4, start_slope);

  //    problem->evaluate( y, _time + _timestep, k4 );  /* k4 <- slope at end of interval */
  
  double *data = new double[3];
  DTIVector answer(3, data);
  
  /* Function evaluation done, time to update the state variables. */
  for ( i=0; i < 3; i++ ) {
    answer[i] = (k1[i] + 2.0*k2[i] + 2.0*k3[i] + k4[i])/6.0;
  }
  //        start_position[i] += (k1[i] + 2.0*k2[i] + 2.0*k3[i] +k4[i]) * (_timestep/6.0);
  return answer;
}
