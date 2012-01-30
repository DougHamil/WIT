/** Copyright (c) 2005, The Board of Trustees of Leland Stanford Junior 
University. All rights reserved. **/

/*************************************************************************
 *    NAME: David Akers and Anthony Sherbondy
 *     
 *    FILE: DTIFilterSphere.cpp
 *    DATE: Fri Mar  5 17:30:06 PST 2004
 *************************************************************************/
#include "DTIFilterSphere.h"
#include "DTIPathway.h"
#include "vtkProperty.h"
#include "vtkCellArray.h"
#include "vtkPolyDataMapper.h"
#include "vtkActor.h"
#include "vtkSphereSource.h"
#include "vtkPolyData.h"
#include "vtkTriangleFilter.h"
#include <RAPID.H>
#include "vtkTransform.h"

double sqr (double x) { return x*x; }

/*************************************************************************
 * Function Name: DTIFilterSphere::DTIFilterVOI
 * Parameters: 
 * Effects: 
 *************************************************************************/

DTIFilterSphere::DTIFilterSphere(vtkActor *actor) : DTIFilterVOI()
{
  this->actor = actor;
  _scale[0] = 1; _scale[1] = 1; _scale[2] = 1;
}


/*************************************************************************
 * Function Name: DTIFilterSphere::matches
 * Parameters: DTIPathway *pathway
 * Returns: bool
 * Effects: 
 *************************************************************************/
bool
DTIFilterSphere::matches(DTIPathway *pathway)
{
	
  if (!_cached_model || isDirty()) {
    _cached_model = computeModel();
    setDirty(false);
  }
  return (inside (pathway) || intersects(pathway, _cached_model));
}


/*************************************************************************
 * Function Name: DTIFilterSphere::inside
 * Parameters: DTIPathway *pathway
 * Returns: bool
 * Effects: 
 *************************************************************************/
bool
DTIFilterSphere::inside(DTIPathway *pathway)
{
  //double radiusSqr = _scale[0]*_scale[0];
  // dakers: corrected for ellipsoid:
  static double firstPoint[3];
  static double lastPoint[3];
  pathway->getPoint(0, firstPoint);
  pathway->getPoint(pathway->getNumPoints()-1, lastPoint);

  double distance1Sqr = sqr(_translation[0]-firstPoint[0])/sqr(_scale[0]/2.0) +
    sqr (_translation[1]-firstPoint[1])/sqr(_scale[1]/2.0) +
    sqr (_translation[2]-firstPoint[2])/sqr(_scale[2]/2.0);

  double distance2Sqr = sqr(_translation[0]-lastPoint[0])/sqr(_scale[0]/2.0) +
    sqr (_translation[1]-lastPoint[1])/sqr(_scale[1]/2.0) +
    sqr (_translation[2]-lastPoint[2])/sqr(_scale[2]/2.0);
  
  return (distance1Sqr < 1 && distance2Sqr < 1);
  
  
}


/*************************************************************************
 * Function Name: DTIFilterSphere::setScale
 * Parameters: double x, double y, double z
 * Returns: void
 * Effects: 
 *************************************************************************/
void
DTIFilterSphere::setScale(const double scale[3])
{
  _scale[0] = scale[0];
  _scale[1] = scale[1];
  _scale[2] = scale[2];
  setDirty(true);
}





/*************************************************************************
 * Function Name: DTIFilterSphere::~DTIFilterSphere
 * Parameters: 
 * Effects: 
 *************************************************************************/

DTIFilterSphere::~DTIFilterSphere()
{
}


/*************************************************************************
 * Function Name: DTIFilterSphere::computeModel
 * Parameters: 
 * Returns: RAPID_model *
 * Effects: 
 *************************************************************************/
collisionModel
DTIFilterSphere::computeModel()
{
  // get polydata

  vtkPolyData *sphereData = GenerateEllipsoid (_scale);

  vtkTriangleFilter *triFilter;

  triFilter = vtkTriangleFilter::New();
  //  triFilter->SetInput(((vtkPolyDataMapper *) actor->GetMapper())->GetInput());
  triFilter->SetInput (sphereData);

  return this->generateModelFromPolydata(triFilter->GetOutput(), triFilter->GetOutput()->GetPolys());
}




vtkPolyData *DTIFilterSphere::GenerateEllipsoid (const double lengths[3]) {
  vtkSphereSource *sphereData = vtkSphereSource::New();
  sphereData->SetRadius(0.5);
  sphereData->SetPhiResolution (16);
  sphereData->SetThetaResolution(16);
  
  sphereData->Update();
  vtkPolyData *data = sphereData->GetOutput();
  
  vtkPoints *points = data->GetPoints();
  
  vtkTransform *scaleAndTranslate = vtkTransform::New();
  scaleAndTranslate->Scale (lengths[0],
			    lengths[1],
			    lengths[2]);
  
  for (int i = 0; i < points->GetNumberOfPoints(); i++) {
    double pt[4];
    double ptNew[4];
    points->GetPoint(i,pt);
    pt[3] = 1.0;
    scaleAndTranslate->MultiplyPoint (pt, ptNew);
    points->SetPoint(i,ptNew);
  }
  
  data->Modified();
  return data;
}

