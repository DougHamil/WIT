/***********************************************************************
 * AUTHOR: David Akers <dakers>
 *   FILE: .//DTIPathwaySelection.cpp
 *   DATE: Fri Nov 11 13:25:38 2005
 *  DESCR: 
 ***********************************************************************/
#include "DTIPathwaySelection.h"
#include <util/typedefs.h>

/***********************************************************************
 *  Method: DTIPathwaySelection::DTIPathwaySelection
 *  Params: int numDTIPathways
 * Effects: 
 ***********************************************************************/
DTIPathwaySelection::DTIPathwaySelection(int numPathways, bool init)
{
  _num_pathways = numPathways;
  _selection = new bool[numPathways];
  for (int i = 0; i < numPathways; i++) {
    _selection[i] = init;
  }
  _dirty = true;
}


/***********************************************************************
 *  Method: DTIPathwaySelection::~DTIPathwaySelection
 *  Params: 
 * Effects: 
 ***********************************************************************/
DTIPathwaySelection::~DTIPathwaySelection()
{
  delete[] _selection;
}


/***********************************************************************
 *  Method: DTIPathwaySelection::intersectWith
 *  Params: const DTIPathwaySelection &other
 * Returns: void
 * Effects: 
 ***********************************************************************/
void
DTIPathwaySelection::intersectWith(const DTIPathwaySelection &other)
{
  assert (other._num_pathways == _num_pathways);

  for (int i = 0; i < _num_pathways; i++) {
    if (other[i] && _selection[i]) {
      _selection[i] = true;
    }
    else {
      _selection[i] = false;
    }
  }
  _dirty = true;
}


/***********************************************************************
 *  Method: DTIPathwaySelection::append
 *  Params: const DTIPathwaySelection &other
 * Returns: void
 * Effects: 
 ***********************************************************************/
void
DTIPathwaySelection::append(const DTIPathwaySelection &other)
{
  for (int i = 0; i < _num_pathways; i++) {
    if (other[i]) {
      _selection[i] = true;
    }
  }
  _dirty = true;
}


/***********************************************************************
 *  Method: DTIPathwaySelection::remove
 *  Params: const DTIPathwaySelection &other
 * Returns: void
 * Effects: 
 ***********************************************************************/
void
DTIPathwaySelection::remove(const DTIPathwaySelection &other)
{
  for (int i = 0; i < _num_pathways; i++) {
    if (other[i]) {
      _selection[i] = false;
    }
  }
  _dirty = true;
}


/***********************************************************************
 *  Method: DTIPathwaySelection::replace
 *  Params: const DTIPathwaySelection &other
 * Returns: void
 * Effects: 
 ***********************************************************************/
void
DTIPathwaySelection::replace(const DTIPathwaySelection &other)
{
  for (int i = 0; i < _num_pathways; i++) {
    _selection[i] = other[i];
  }
  _dirty = true;
}


/***********************************************************************
 *  Method: DTIPathwaySelection::copy
 *  Params: 
 * Returns: DTIPathwaySelection *
 * Effects: 
 ***********************************************************************/
DTIPathwaySelection *
DTIPathwaySelection::copy()
{
  DTIPathwaySelection *newSelection = new DTIPathwaySelection(_num_pathways);
  newSelection->replace (*this);
  newSelection->_dirty = _dirty;
  return newSelection;
}


/***********************************************************************
 *  Method: DTIPathwaySelection::getNumSelectedPathways
 *  Params: 
 * Returns: int
 * Effects: 
 ***********************************************************************/
int
DTIPathwaySelection::getNumSelectedPathways()
{
  // xxx slow - should cache this.
  int count = 0;
  for (int i = 0; i < _num_pathways; i++) {
    if (_selection[i]) {
      count++;
    }
  }
  return count;
}


