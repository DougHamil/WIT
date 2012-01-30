/** Copyright (c) 2005, The Board of Trustees of Leland Stanford Junior 
University. All rights reserved. **/

/*************************************************************************
 *    NAME: David Akers and Anthony Sherbondy
 *     
 *    FILE: DTIFilterCube.cpp
 *    DATE: Fri Mar  5 17:30:06 PST 2004
 *************************************************************************/
#include "DTIFilterCube.h"
#include "DTIPathway.h"
#include "vtkProperty.h"
#include "vtkCellArray.h"
#include "vtkPolyDataMapper.h"
#include "vtkActor.h"
#include "vtkCubeSource.h"
#include "vtkPolyData.h"
#include "vtkTriangleFilter.h"

/*************************************************************************
 * Function Name: DTIFilterCube::DTIFilterVOI
 * Parameters: 
 * Effects: 
 *************************************************************************/

DTIFilterCube::DTIFilterCube() : DTIFilterVOI()
{
  _scale[0] = 1; _scale[1] = 1; _scale[2] = 1;
}


/*************************************************************************
 * Function Name: DTIFilterCube::matches
 * Parameters: DTIPathway *pathway
 * Returns: bool
 * Effects: 
 *************************************************************************/
bool
DTIFilterCube::matches(DTIPathway *pathway)
{
	
  if (!_cached_model || isDirty()) {
		_cached_model = computeModel();
		setDirty(false);
  }
  return (intersects (pathway, _cached_model) || inside (pathway));
}


/*************************************************************************
 * Function Name: DTIFilterCube::inside
 * Parameters: DTIPathway *pathway
 * Returns: bool
 * Effects: 
 *************************************************************************/
bool
DTIFilterCube::inside(DTIPathway *pathway)
{
  
  static double firstPoint[3];
  static double lastPoint[3];
  pathway->getPoint(0, firstPoint);
  assert (pathway->getNumPoints() > 1);
  pathway->getPoint(pathway->getNumPoints()-1, lastPoint);
  static double minCoords[3];
  static double maxCoords[3];
  minCoords[0] = _translation[0] - _scale[0]/2.0;
  minCoords[1] = _translation[1] - _scale[1]/2.0;
  minCoords[2] = _translation[2] - _scale[2]/2.0;
  maxCoords[0] = _translation[0] + _scale[0]/2.0;
  maxCoords[1] = _translation[1] + _scale[1]/2.0;
  maxCoords[2] = _translation[2] + _scale[2]/2.0;

  return (firstPoint[0] >= minCoords[0] && firstPoint[0] <= maxCoords[0] &&
	  firstPoint[1] >= minCoords[1] && firstPoint[1] <= maxCoords[1] &&
	  firstPoint[2] >= minCoords[2] && firstPoint[2] <= maxCoords[2] &&
	  lastPoint[0] >= minCoords[0] && lastPoint[0] <= maxCoords[0] &&
	  lastPoint[1] >= minCoords[1] && lastPoint[1] <= maxCoords[1] &&
	  lastPoint[2] >= minCoords[2] && lastPoint[2] <= maxCoords[2]);

}


/*************************************************************************
 * Function Name: DTIFilterCube::setScale
 * Parameters: double x, double y, double z
 * Returns: void
 * Effects: 
 *************************************************************************/
void
DTIFilterCube::setScale(const double scale[3])
{
  _scale[0] = scale[0];
  _scale[1] = scale[1];
  _scale[2] = scale[2];
  setDirty(true);
}





/*************************************************************************
 * Function Name: DTIFilterCube::~DTIFilterCube
 * Parameters: 
 * Effects: 
 *************************************************************************/

DTIFilterCube::~DTIFilterCube()
{
}


/*************************************************************************
 * Function Name: DTIFilterCube::computeModel
 * Parameters: 
 * Returns: RAPID_model *
 * Effects: 
 *************************************************************************/
collisionModel
DTIFilterCube::computeModel()
{

  vtkCubeSource *cubeData;

  cubeData = vtkCubeSource::New();
  cubeData->SetCenter(ZERO_TRANSLATION);
  cubeData->SetXLength(_scale[0]);
  cubeData->SetYLength(_scale[1]);
  cubeData->SetZLength(_scale[2]);

  vtkTriangleFilter *triFilter;

  triFilter = vtkTriangleFilter::New();
  triFilter->SetInput(cubeData->GetOutput());

  triFilter->Update();

  return this->generateModelFromPolydata(triFilter->GetOutput(),triFilter->GetOutput()->GetPolys());
}


