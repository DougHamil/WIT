/** Copyright (c) 2005, The Board of Trustees of Leland Stanford Junior 
University. All rights reserved. **/

/*************************************************************************
 *    NAME: David Akers and Anthony Sherbondy
 *     
 *    FILE: DTIFilterVOI.cpp
 *    DATE: Sun Feb 29  7:54:22 PST 2004
 *************************************************************************/
#include "DTIFilterVOI.h"
#include <RAPID.H>
#include <stdexcept>
#include "DTIPathway.h"
#include <iostream>
#include <vtkPolyData.h>
#include <vtkCellArray.h>
#include <vtkCellData.h>

#ifdef USE_RAPID
#include <RAPID.H>
#else
#include <ozcollide/ozcollide.h>
#include <ozcollide/aabbtreepoly_builder.h>
#include <ozcollide/polygon.h>
using namespace ozcollide;
#endif

using namespace std;

collisionModel DTIFilterVOI::generateModelFromPolydata(vtkPolyData *tris, vtkCellArray *cells)
{
  collisionModel model;

#ifdef USE_RAPID
  model = new RAPID_model;
  model->BeginModel();

  vtkIdType            npts = 0;
  vtkIdType            *pts = 0;

  cells->InitTraversal();
  int counter=0;
  while(cells->GetNextCell( npts, pts ))
  {
    double a[3], b[3], c[3];
    if(npts != 3)
      std::cerr << "May be doing intersection incorrectly." << endl;
    tris->GetPoint(pts[0],a);
    tris->GetPoint(pts[1],b);
    tris->GetPoint(pts[2],c);
    model->AddTri (a,b,c,counter);
    counter++;
  }
  model->EndModel();
#else
  AABBTreePolyBuilder builder;
  Vector<Vec3f> vertList;
  Vector<Polygon> triList;

  vtkIdType            npts = 0;
  vtkIdType            *pts = 0;

  cells->InitTraversal();
  int counter=0;
  int indCounter = 0;
  while(cells->GetNextCell( npts, pts ))
  {

    double a[3], b[3], c[3];
    if(npts != 3)
      std::cout << "May be doing intersection incorrectly." << endl;
    tris->GetPoint(pts[0],a);
    tris->GetPoint(pts[1],b);
    tris->GetPoint(pts[2],c);
    vertList.add(Vec3f(a[0],a[1],a[2]));
    vertList.add(Vec3f(b[0],b[1],b[2]));
    vertList.add(Vec3f(c[0],c[1],c[2]));

    int indices[3];
    indices[0] = counter++;
    indices[1] = counter++;
    indices[2] = counter++;
    Polygon *poly = new Polygon();
    poly->setIndicesMemory(3,&indices[0]);
    triList.add(*poly);
  }

  model = builder.buildFromPolys(triList.mem(),//polygons
                                               triList.size(),//polygon count
                                               vertList.mem(),//vertices
                                               vertList.size());//vertices count
#endif

  return model;
}

/*************************************************************************
 * Function Name: DTIFilterVOI::DTIFilterVOI
 * Parameters: 
 * Effects: 
 *************************************************************************/

DTIFilterVOI::DTIFilterVOI()
{
  _cached_model = NULL;
  
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      if (i != j) {
	_rotation_matrix[i][j] = 0;
      }
      else {
	_rotation_matrix[i][j] = 1;
      }
    }
    _translation[i] = 0;
  }
}


/*************************************************************************
 * Function Name: DTIFilterVOI::~DTIFilterVOI
 * Parameters: 
 * Effects: 
 *************************************************************************/

DTIFilterVOI::~DTIFilterVOI()
{
#ifdef USE_RAPID
  delete _cached_model;
#else
  _cached_model->destroy();
#endif
}


/*************************************************************************
 * Function Name: DTIFilterVOI::addTriangle
 * Parameters: double x, double y, double z
 * Returns: void
 * Effects: 
 *************************************************************************/
/*
void
DTIFilterVOI::addTriangle(const double x[3], const double y[3], const double z[3])
{
  if (_model_defined) {
    throw new std::runtime_error("Cannot add geometry to model after filter test!");
  }
  else {
    _model->AddTri (x, y, z, _counter);
    _counter++;
  }
}*/


/*************************************************************************
 * Function Name: DTIFilterVOI::matches
 * Parameters: DTIPathway *pathway
 * Returns: bool
 * Effects: 
 *************************************************************************/
bool
DTIFilterVOI::matches(DTIPathway *pathway)
{
	if (!_cached_model || isDirty()) {
		_cached_model = computeModel();
	}
  return intersects (pathway, _cached_model);
}

bool
DTIFilterVOI::intersects(DTIPathway *pathway, collisionModel filterModel)
{
	//cerr << "translation: " << _translation[0] << ", " << _translation[1] << ", " << _translation[2] << endl;

  collisionModel collModel = pathway->getCollisionModel();
#ifdef USE_RAPID
  RAPID_Collide (_rotation_matrix, _translation, filterModel,
		 ZERO_ROTATION, ZERO_TRANSLATION, collModel, RAPID_FIRST_CONTACT);
  //cerr << "num contacts: " << RAPID_num_contacts << endl;
  if (RAPID_num_contacts > 0) {
    //    std::cerr << "match: " << RAPID_num_contacts << std::endl;
    return true;
  }
  else {
    return false;
  }
#else
  // TODO: ozcollide
  AABBTreePoly::BoxColResult result;

  filterModel->collideWithBox(collModel->getRootNode()->aabb, result);
  cerr <<" Num polys: " << result.polys_.size() << endl;
  return result.polys_.size() > 0 ? true : false;
#endif
  return false;
}

/*
bool 
DTIFilterVOI::inside(DTIPathway *pathway)
{
	static double firstPoint[3];
	static double lastPoint[3];
	pathway->getPoint(0, firstPoint);
	pathway->getPoint(pathway->getLength()-1, lastPoint);
}
	*/




/*************************************************************************
 * Function Name: DTIFilterVOI::setRotationMatrix
 * Parameters: const double R[3][3]
 * Returns: void
 * Effects: 
 *************************************************************************/
void
DTIFilterVOI::setRotationMatrix(const double R[3][3])
{
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      _rotation_matrix[i][j] = R[i][j];
    }
  }

}


/*************************************************************************
 * Function Name: DTIFilterVOI::setTranslation
 * Parameters: const double R[3]
 * Returns: void
 * Effects: 
 *************************************************************************/
void
DTIFilterVOI::setTranslation(const double R[3])
{
  for (int i = 0; i < 3; i++) {
    _translation[i] = R[i];
  }
}


