#include "DTIStats.h"
#include "DTIPathwayStatisticHeader.h"
#include "DTIPathwayDatabase.h"
#include "DTIPathway.h"
#include "DTIVolumeInterface.h"
#include "ScalarVolume.h"
#include <tnt/tnt_array1d_utils.h>
#include <iostream>
#include <math.h>
#include <fstream>
#include "DTIMath.h"
#include "DTIPathTracer.h"

using namespace std;

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef INVSQRT2PI
#define INVSQRT2PI 1/sqrt(2*M_PI)
#endif

/***********************************************************************
 *  Method: DTIStats::computeLength
 *  Params: DTIPathwayInterface *path
 * Returns: void
 * Effects: 
 ***********************************************************************/
double
DTIStats::computeLength(DTIPathwayInterface *path, PathwayProperty id)
{
  double len = 0.0;
  int counter = 0;
  //  path->setPointStatistic(0, id, 0);
  for (int i = 1; i < path->getNumPoints(); i++) {
    double prev[3];
    double cur[3];
    path->getPoint (i-1, prev);
    path->getPoint (i, cur);
    DTIVector p1(3, prev);
    DTIVector p2(3, cur);
    len += computeDistance (p1, p2);
    //    path->setPointStatistic (i, id, len);
  }
  path->setPathStatistic(id, len);
  return len;
}

/***********************************************************************
 *  Method: DTIStats::computeSampleOrder
 *  Params: DTIPathwayInterface *path, int order
 * Returns: void
 * Effects: 
 ***********************************************************************/
int
DTIStats::computeSampleOrder(DTIPathwayInterface *path, PathwayProperty id, int order)
{
  path->setPathStatistic(id, order);
  return order;
}


/***********************************************************************
 *  Method: DTIStats::computeCurvature
 *  Params: DTIPathwayInterface *path
 * Returns: double
 * Effects: 
 ***********************************************************************/
double
DTIStats::computeCurvature(DTIPathwayInterface *path, PathwayProperty id)
{
   double totalCurvature = 0.0;
   //  path->setPointStatistic (0, id, 0);

  for (int i = 0; i < path->getNumPoints()-2; i++) {
    double a[3], b[3], c[3];
    path->getPoint(i, a);
    path->getPoint(i+1,b);
    path->getPoint(i+2,c);
    
    DTIVector aVec(3,a);
    DTIVector bVec(3,b);
    DTIVector cVec(3,c);
    
    double curvature = computeLocalCurvature (aVec, bVec, cVec);

    //    path->setPointStatistic(i+1, id, curvature);

    totalCurvature += curvature;
  }
  //  path->setPointStatistic (path->getNumPoints()-1, id, 0);

  double avgCurvature;
  if (path->getNumPoints() > 2) {
    avgCurvature = totalCurvature / (double) (path->getNumPoints()-2);
  }
  else {
    avgCurvature = 0;
  }
  path->setPathStatistic (id, avgCurvature);
  return avgCurvature;
}



void computePathTangent (DTIVector &tangent, const DTIVector &p1, const DTIVector &p2, const DTIVector &p3) {
  tangent = norm((0.5)*norm(p2-p1) + (0.5)*norm((p3-p2)));
}

void computePathTangent (DTIVector &tangent, const DTIVector &p1, const DTIVector &p2) {
  tangent = norm(p2-p1);
}


/***********************************************************************
 *  Method: DTIStats::computeDistance
 *  Params: const DTIVector &p1, const DTIVector &p2
 * Returns: double
 * Effects: 
 ***********************************************************************/
double
DTIStats::computeDistance(const DTIVector &p1, const DTIVector &p2)
{
  double buf[3];
  DTIVector diff(3,buf);
  diff = p1-p2;
  double dot = dproduct(diff,diff);
  return sqrt(dot);
}


/***********************************************************************
 *  Method: DTIStats::computeLocalCurvature
 *  Params: const DTIVector &pt1, const DTIVector &pt2, const DTIVector &pt3
 * Returns: double
 * Effects: 
 ***********************************************************************/
double
DTIStats::computeLocalCurvature(const DTIVector &pt1, const DTIVector &pt2, const DTIVector &pt3)
{
  double a = computeDistance (pt1, pt2);
  double b = computeDistance (pt2, pt3);
  double c = computeDistance (pt1, pt3);
  double s = (a+b+c)/2.0;
  double denom = 4*sqrt(s*(s-a)*(s-b)*(s-c));
  double curvature = 0.0;
  if (denom != 0) {
    double radius = a*b*c/(4*sqrt(s*(s-a)*(s-b)*(s-c)));
    curvature = 1.0/radius;
  }
  else {
    curvature = 0.0;
  }
  return curvature;
}



/***********************************************************************
 *  Method: DTIStats::computeFA
 *  Params: DTIPathwayInterface *path, ScalarVolumeInterface *faVol
 * Returns: double
 * Effects: 
 ***********************************************************************/
double
DTIStats::computeFA(DTIPathwayInterface *path, PathwayProperty id, ScalarVolume *faVol)
{
  std::list <double> faList;
  //  double mmPerVox[3];
  //  faVol->getVoxelSize(mmPerVox[0], mmPerVox[1], mmPerVox[2]);
  double *inverseMx = faVol->getInverseTransformMatrix();
  
  for (int i = 0; i < path->getNumPoints(); i++) {
    double pt[4] = {0, 0, 0, 1};
    path->getPoint (i, pt);
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
    double fa = faVol->getFA(x,y,z,0);
    //    path->setPointStatistic (i, id, fa);
    faList.push_back(fa);
  }
  faList.sort();
  unsigned int count = 0;
  for (std::list<double>::iterator iter = faList.begin(); iter != faList.end(); iter++) {
    double fa = *iter;
    if (count == faList.size()/2) {
      if (fa > 1.0) fa = 0.0;
      // shouldn't have to do this - caused by negative eigenvalues from tensorcalc!
      path->setPathStatistic (id, fa);
      return fa;
    }
    count++;
  }
  assert (false);
}









