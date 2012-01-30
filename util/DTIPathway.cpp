/** Copyright (c) 2005, The Board of Trustees of Leland Stanford Junior 
University. All rights reserved. **/

/*************************************************************************
 *    NAME: David Akers and Anthony Sherbondy
 *     
 *    FILE: DTIPathway.cpp
 *    DATE: Mon Feb 23  6:00:16 PST 2004
 *************************************************************************/
#include "DTIPathway.h"
#include <exception>
#include <stdexcept>
#include <tnt/tnt_array1d_utils.h>
#include <math.h>
#include "ScalarVolume.h"
#include <list>

#ifdef USE_RAPID
#include <RAPID.H>
#else
#include <ozcollide/ozcollide.h>
#include <ozcollide/aabbtreepoly_builder.h>
#include <ozcollide/polygon.h>
using namespace ozcollide;
#endif

#ifndef M_PI
#define M_PI 3.141592
#endif

/*************************************************************************
 * Function Name: DTIPathway::DTIPathway
 * Parameters: 
 * Effects: 
 *************************************************************************/

DTIPathway::DTIPathway(DTIPathwayAlgorithm algo) : DTIPathwayInterface(algo)
{
  _seed_point_index = 0;
  _coll_model = NULL;
}


/*************************************************************************
 * Function Name: DTIPathway::~DTIPathway
 * Parameters: 
 * Effects: 
 *************************************************************************/

DTIPathway::~DTIPathway()
{
#ifdef USE_RAPID
  delete _coll_model;
#else
  _coll_model->destroy();
#endif
}


collisionModel
DTIPathway::generateCollisionModel()
{
  double EPS = 0.001;
#ifdef USE_RAPID
  RAPID_model *_rapid_model = new RAPID_model;
  _rapid_model->BeginModel();
  int counter = 0;
  DTIGeometryVector *previous = NULL;
  for (std::vector<DTIGeometryVector *>::iterator iter = _point_vector.begin();
        iter != _point_vector.end();
        iter++)
  {
    DTIGeometryVector *current = *iter;
    //std::cerr << "current: " << current << std::endl;
    if (counter != 0)
    {

      double p1[3] = {(*previous)[0]+EPS, (*previous)[1], (*previous)[2]};

      double p2[3] = {(*previous)[0]+EPS, (*previous)[1] + EPS, (*previous)[2]};
      double p3[3] = {(*current)[0], (*current)[1], (*current)[2]};
      //std::cerr << "pt1: " << p1[0] << ", " << p1[1] << ", " << p3[2] << std::endl;
      _rapid_model->AddTri (p1, p2, p3, counter);
    }
    previous = current;
    counter++;
  }
  _rapid_model->EndModel();
  return _rapid_model;
#else
  AABBTreePolyBuilder builder;
  Vector<Vec3f> vertList;
  Vector<Polygon> triList;

  int counter = 0;
  DTIGeometryVector *previous = NULL;
  for (std::vector<DTIGeometryVector *>::iterator iter = _point_vector.begin();
        iter != _point_vector.end();
        iter++)
  {
    DTIGeometryVector *current = *iter;
    //std::cerr << "current: " << current << std::endl;
    if (counter != 0)
    {

      double p1[3] = {(*previous)[0]+EPS, (*previous)[1], (*previous)[2]};

      double p2[3] = {(*previous)[0]+EPS, (*previous)[1] + EPS, (*previous)[2]};
      double p3[3] = {(*current)[0], (*current)[1], (*current)[2]};
      vertList.add(Vec3f(p1[0], p1[1], p1[2]));
      vertList.add(Vec3f(p2[0], p2[1], p2[2]));
      vertList.add(Vec3f(p3[0], p3[1], p3[2]));

      int indices[3];
      indices[0] = counter++;
      indices[1] = counter++;
      indices[2] = counter++;
      Polygon *poly = new Polygon();
      poly->setIndicesMemory(3,&indices[0]);
      triList.add(*poly);
    }
    previous = current;
  }

  return builder.buildFromPolys(triList.mem(),//polygons
                                               triList.size(),//polygon count
                                               vertList.mem(),//vertices
                                               vertList.size());//vertices count
#endif
}
/*************************************************************************
 * Function Name: DTIPathway::getRAPIDModel
 * Parameters: 
 * Returns: RAPID_model *
 * Effects: 
 *************************************************************************/
collisionModel
DTIPathway::getCollisionModel()
{

  if (_coll_model) {
    return _coll_model;
  }
  else {
    _coll_model = generateCollisionModel();
    return _coll_model;
  }
}


/*
double getFA (ScalarVolume *volume, double x, double y, double z) {
  int xInt = (int) floor(x+0.5);
  int yInt = (int) floor(y+0.5);
  int zInt = (int) floor(z+0.5);
  double fa = volume->getFA(xInt,yInt,zInt);
  return fa;
}
*/

