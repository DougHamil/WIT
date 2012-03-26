/** Copyright (c) 2005, The Board of Trustees of Leland Stanford Junior 
University. All rights reserved. **/

#ifndef FIBER_TRACT_H
#define FIBER_TRACT_H

#include "typedefs.h"
#include "DTIPathwayInterface.h"
#include <vector>

#ifdef USE_RAPID
#include <RAPID201/RAPID.h>
#else
#include <opcode/Opcode.h>
#endif

class DTIPathway : public DTIPathwayInterface {
  friend class DTIFilterROI;
  public:
  friend class DTIPathwayIO;
  DTIPathway(DTIPathwayAlgorithm algo);
  virtual ~DTIPathway();

  DTIPathwayAlgorithm getPathwayAlgorithm() { return _algo; }

  int getClusterIndex () { return _cluster_index; }
  void setClusterIndex (int index) { _cluster_index = index; }
  CollModel *getCollisionModel();

  int getID() {return _unique_id;}
  void setID(int id){_unique_id = id;}

 private:
  int _cluster_index;
  int _unique_id;
  CollModel *_coll_model;  
};

#endif
