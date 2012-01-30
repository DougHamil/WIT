/** Copyright (c) 2005, The Board of Trustees of Leland Stanford Junior 
University. All rights reserved. **/

/*************************************************************************
 *    NAME: David Akers and Anthony Sherbondy
 *     
 *    FILE: DTIFilterComplex.cpp
 *    DATE: Fri Mar  5 17:30:06 PST 2004
 *************************************************************************/
#include "DTIFilterComplex.h"
#include "DTIPathway.h"
#include "vtkProperty.h"
#include "vtkCellArray.h"
#include "vtkPolyDataMapper.h"
#include "vtkActor.h"
#include "vtkCubeSource.h"
#include "vtkPolyData.h"
#include "vtkTriangleFilter.h"
#include <RAPID.H>

/*************************************************************************
 * Function Name: DTIFilterComplex::DTIFilterVOI
 * Parameters: 
 * Effects: 
 *************************************************************************/

DTIFilterComplex::DTIFilterComplex(vtkPolyData *triangles) : DTIFilterVOI()
{
  tris = triangles;
  _scale[0] = 1; _scale[1] = 1; _scale[2] = 1;
  //  _delta[0] = delta[0];
  //  _delta[1] = delta[1];
  //  _delta[2] = delta[2];
}


/*************************************************************************
 * Function Name: DTIFilterComplex::matches
 * Parameters: DTIPathway *pathway
 * Returns: bool
 * Effects: 
 *************************************************************************/
bool
DTIFilterComplex::matches(DTIPathway *pathway)
{
	
  if (!_cached_model || isDirty()) {
		_cached_model = computeModel();
		setDirty(false);
  }
  return (intersects (pathway, _cached_model) || inside (pathway));
}


/*************************************************************************
 * Function Name: DTIFilterComplex::inside
 * Parameters: DTIPathway *pathway
 * Returns: bool
 * Effects: 
 *************************************************************************/
bool
DTIFilterComplex::inside(DTIPathway *pathway)
{
  
//   static double firstPoint[3];
//   static double lastPoint[3];
//   pathway->getPoint(0, firstPoint);
//   assert (pathway->getNumPoints() > 1);
//   pathway->getPoint(pathway->getNumPoints()-1, lastPoint);
//   static double minCoords[3];
//   static double maxCoords[3];
//   minCoords[0] = _translation[0] - _scale[0]/2.0;
//   minCoords[1] = _translation[1] - _scale[1]/2.0;
//   minCoords[2] = _translation[2] - _scale[2]/2.0;
//   maxCoords[0] = _translation[0] + _scale[0]/2.0;
//   maxCoords[1] = _translation[1] + _scale[1]/2.0;
//   maxCoords[2] = _translation[2] + _scale[2]/2.0;

//   return (firstPoint[0] >= minCoords[0] && firstPoint[0] <= maxCoords[0] &&
// 	  firstPoint[1] >= minCoords[1] && firstPoint[1] <= maxCoords[1] &&
// 	  firstPoint[2] >= minCoords[2] && firstPoint[2] <= maxCoords[2] &&
// 	  lastPoint[0] >= minCoords[0] && lastPoint[0] <= maxCoords[0] &&
// 	  lastPoint[1] >= minCoords[1] && lastPoint[1] <= maxCoords[1] &&
// 	  lastPoint[2] >= minCoords[2] && lastPoint[2] <= maxCoords[2]);

// Can't do this yet
  return false;
}


/*************************************************************************
 * Function Name: DTIFilterComplex::setScale
 * Parameters: double x, double y, double z
 * Returns: void
 * Effects: 
 *************************************************************************/
void
DTIFilterComplex::setScale(const double scale[3])
{
  _scale[0] = scale[0];
  _scale[1] = scale[1];
  _scale[2] = scale[2];
  setDirty(true);
}





/*************************************************************************
 * Function Name: DTIFilterComplex::~DTIFilterComplex
 * Parameters: 
 * Effects: 
 *************************************************************************/

DTIFilterComplex::~DTIFilterComplex()
{
}


void 
DTIFilterComplex::expathwayPoint(vtkIdType id, double pt[3]) {
  
  tris->GetPoint (id, pt);
  //  pt[0] /= _delta[0];
  //  pt[1] /= _delta[1];
  //  pt[2] /= _delta[2];
}

/*************************************************************************
 * Function Name: DTIFilterComplex::computeModel
 * Parameters: 
 * Returns: RAPID_model *
 * Effects: 
 *************************************************************************/
collisionModel
DTIFilterComplex::computeModel()
{
  return this->generateModelFromPolydata(tris, tris->GetPolys());
}


