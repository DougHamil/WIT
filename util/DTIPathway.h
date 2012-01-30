/** Copyright (c) 2005, The Board of Trustees of Leland Stanford Junior 
University. All rights reserved. **/

#ifndef FIBER_TRACT_H
#define FIBER_TRACT_H

#include "typedefs.h"
#include "DTIPathwayInterface.h"
#include <vector>

#ifdef USE_RAPID
class RAPID_model;
typedef RAPID_model* collisionModel;
#else
#include <ozcollide/aabbtree_poly.h>
typedef ozcollide::AABBTreePoly* collisionModel;
#endif

class ScalarVolume;

class DTIPathway : public DTIPathwayInterface {
  friend class DTIFilterVOI;
  public:
  friend class DTIPathwayIO;
  DTIPathway(DTIPathwayAlgorithm algo);
  virtual ~DTIPathway();

  DTIPathwayAlgorithm getPathwayAlgorithm() { return _algo; }

  int getClusterIndex () { return _cluster_index; }
  void setClusterIndex (int index) { _cluster_index = index; }

  collisionModel getCollisionModel();

  int getID() {return _unique_id;}
  void setID(int id){_unique_id = id;}

  /*  double getValidity(){return _cached_validity;}
      void setValidity(double v){_cached_validity = v;}*/
  /*
  void setCachedLengthMM(double len);
  void setCachedMedianFA(double fa);
  void setCachedMinFA(double fa);
  void setCachedMaxAngle(double ang);
  void setCachedMeanCurvature(double curv);
  */
 private:

  collisionModel generateCollisionModel();

  int _cluster_index;
  int _unique_id;
 
  collisionModel _coll_model;
  //  std::vector <DTIVector *> _point_vector;
};

#endif
