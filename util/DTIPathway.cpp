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
Opcode::Model *DTIPathway::getCollisionModel()
{
  double EPS = 0.001;
  if (_coll_model) {
    return _coll_model;
  }
  else {
    _coll_model = new Opcode::Model;

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

	OPCC.mIMesh->SetNbTriangles(numTris);
	OPCC.mIMesh->SetNbVertices(numVerts);
	OPCC.mIMesh->SetPointers(tris, verts);
	
	_coll_model->Build(OPCC);

    return _coll_model;
  }
}


