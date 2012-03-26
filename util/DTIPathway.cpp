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
#include <list>

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
  delete _coll_model;
}

extern int tottris;
CollModel *DTIPathway::getCollisionModel()
{
  double EPS = 0.001;
  if (_coll_model) {
    return _coll_model;
  }
  else {
	  _coll_model = new CollModel;
#ifdef USE_RAPID
    _coll_model->BeginModel();
    int counter = 0;
    DTIGeometryVector *previous = NULL;
    for (std::vector<DTIGeometryVector *>::iterator iter = _point_vector.begin();
	 iter != _point_vector.end();
	 iter++) {
      DTIGeometryVector *current = *iter;
      //std::cerr << "current: " << current << std::endl;
      if (counter != 0) {
		tottris+=3;
		double p1[3] = {(*previous)[0]+EPS, (*previous)[1], (*previous)[2]};
	
		double p2[3] = {(*previous)[0]+EPS, (*previous)[1] + EPS, (*previous)[2]};
		double p3[3] = {(*current)[0], (*current)[1], (*current)[2]};
		//std::cerr << "pt1: " << p1[0] << ", " << p1[1] << ", " << p3[2] << std::endl;
		_coll_model->AddTri (p1, p2, p3, counter);
      }
      previous = current;
      counter++;
    }
    _coll_model->EndModel();
#else
	int numVerts = _point_vector.size();
	int numTris = _point_vector.size() - 1;
	IceMaths::Point *verts = new IceMaths::Point[numVerts];
	IceMaths::IndexedTriangle *tris = new IceMaths::IndexedTriangle[numTris];
	int index = 0;
    for (std::vector<DTIGeometryVector *>::iterator iter = _point_vector.begin(); iter != _point_vector.end(); ++iter)
	{
		DTIGeometryVector *vec = *iter;

		verts[index] = IceMaths::Point((*vec)[0], (*vec)[1], (*vec)[2]);

		if(index != 0)
		{
			tris[index-1] = IceMaths::IndexedTriangle(index - 1, index - 1, index);
		}

		++index;
	}

	Opcode::OPCODECREATE OPCC;
	OPCC.mIMesh = new Opcode::MeshInterface();
	//OPCC.mQuantized = false;
	//OPCC.mKeepOriginal = false;
	OPCC.mNoLeaf =false;
	OPCC.mIMesh->SetNbTriangles(numTris);
	OPCC.mIMesh->SetNbVertices(numVerts);
	OPCC.mIMesh->SetPointers(tris, verts);

	_coll_model->Build(OPCC);

	//delete [] verts;
	//delete [] tris;
#endif

    return _coll_model;
  }
}


