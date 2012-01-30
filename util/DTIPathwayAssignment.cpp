/***********************************************************************
 * AUTHOR: David Akers <dakers>
 *   FILE: .//DTIPathwayAssignment.cpp
 *   DATE: Sun Feb 26 18:57:15 2006
 *  DESCR: 
 ***********************************************************************/
#include "DTIPathwayAssignment.h"
#include "DTIPathwaySelection.h"
#include <iostream>
using namespace std;

/***********************************************************************
 *  Method: DTIPathwayAssignment::DTIPathwayAssignment
 *  Params: int numPathways
 * Effects: 
 ***********************************************************************/
DTIPathwayAssignment::DTIPathwayAssignment(int numPathways, DTIPathwayGroupID init)
{
  _num_pathways = numPathways;
  _assignment = new DTIPathwayGroupID[numPathways];
  for (int i = 0; i < numPathways; i++) {
    _assignment[i] = init;
  }
  _dirty = true;
}


/***********************************************************************
 *  Method: DTIPathwayAssignment::~DTIPathwayAssignment
 *  Params: 
 * Effects: 
 ***********************************************************************/
DTIPathwayAssignment::~DTIPathwayAssignment()
{
  delete[] _assignment;
}


/***********************************************************************
 *  Method: DTIPathwayAssignment::getNumAssignedPathways
 *  Params: DTIPathwayGroupID color
 * Returns: int
 * Effects: 
 ***********************************************************************/
int
DTIPathwayAssignment::getNumAssignedPathways(DTIPathwayGroupID color)
{
  // xxx slow - should cache this.
  int count = 0;
  for (int i = 0; i < _num_pathways; i++) {
    if (_assignment[i] == color) {
      count++;
    }
  }
  return count; 
}


/***********************************************************************
 *  Method: DTIPathwayAssignment::intersectWith
 *  Params: const DTIPathwaySelection &selection, DTIPathwayGroupID color
 * Returns: void
 * Effects: 
 ***********************************************************************/
void
DTIPathwayAssignment::intersectWith(const DTIPathwaySelection &other, DTIPathwayGroupID color)
{
  //assert (other.getNumPathways() == _num_pathways);

  for (int i = 0; i < _num_pathways; i++) {
    if (_assignment[i] == color) {
      if (other[i]) {
	_assignment[i] = color;
      }
      else {
	_assignment[i] = DTI_COLOR_UNASSIGNED;
      }
    }
  }
  _dirty = true;
}


/***********************************************************************
 *  Method: DTIPathwayAssignment::append
 *  Params: const DTIPathwaySelection &other, DTIPathwayGroupID color
 * Returns: void
 * Effects: 
 ***********************************************************************/
void
DTIPathwayAssignment::append(const DTIPathwaySelection &other, DTIPathwayGroupID color, bool lockedMode)
{
  for (int i = 0; i < _num_pathways; i++) {
    bool assignable = (!lockedMode ||
			(lockedMode &&
			 (_assignment[i] == DTI_COLOR_UNASSIGNED ||
			  _assignment[i] == color)));
    if (other[i] && assignable) {
      _dirty = true;
      _assignment[i] = color;
    }
  }
}


/***********************************************************************
 *  Method: DTIPathwayAssignment::remove
 *  Params: const DTIPathwaySelection &other, DTIPathwayGroupID color
 * Returns: void
 * Effects: 
 ***********************************************************************/
void
DTIPathwayAssignment::remove(const DTIPathwaySelection &other, DTIPathwayGroupID color)
{
  for (int i = 0; i < _num_pathways; i++) {
    if (other[i] && _assignment[i] == color) {
      _assignment[i] = DTI_COLOR_UNASSIGNED;
    }
  }
  _dirty = true;
}


/***********************************************************************
 *  Method: DTIPathwayAssignment::replace
 *  Params: const DTIPathwaySelection &other, DTIPathwayGroupID color
 * Returns: void
 * Effects: 
 ***********************************************************************/
void
DTIPathwayAssignment::replace(const DTIPathwaySelection &other, DTIPathwayGroupID color, bool lockedMode)
{
  for (int i = 0; i < _num_pathways; i++) {
    bool assignable = (!lockedMode ||
		       (lockedMode &&
			(_assignment[i] == DTI_COLOR_UNASSIGNED ||
			 _assignment[i] == color)));
    if (_assignment[i] == color) {
      _assignment[i] = DTI_COLOR_UNASSIGNED;
    }
    if (other[i] && assignable) {
      _assignment[i] = color;
    }
  }
  _dirty = true;
}


/***********************************************************************
 *  Method: DTIPathwayAssignment::copy
 *  Params: 
 * Returns: DTIPathwayAssignment *
 * Effects: 
 ***********************************************************************/
DTIPathwayAssignment *
DTIPathwayAssignment::copy()
{
  DTIPathwayAssignment *newAssignment = new DTIPathwayAssignment(_num_pathways, DTI_COLOR_UNASSIGNED);
  for (int i = 0; i < _num_pathways; i++) {
    newAssignment->setValue(i, _assignment[i]);
  }
  newAssignment->_dirty = _dirty;
  return newAssignment;
}


