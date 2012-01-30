#ifndef PATHWAY_ASSIGNMENT_H
#define PATHWAY_ASSIGNMENT_H

#include "typedefs.h"
class DTIPathwaySelection;

class DTIPathwayAssignment {

 public:
  DTIPathwayAssignment(int numPathways, DTIPathwayGroupID initColor);
  virtual ~DTIPathwayAssignment();

  int getNumAssignedPathways(DTIPathwayGroupID color);
  int getNumPathways () { return _num_pathways; }
  DTIPathwayGroupID operator[] (int i) const { return _assignment[i]; }
  void intersectWith (const DTIPathwaySelection &selection, DTIPathwayGroupID color);
  void append (const DTIPathwaySelection &other, DTIPathwayGroupID color, bool lockMode);
  void remove (const DTIPathwaySelection &other, DTIPathwayGroupID color);
  void replace (const DTIPathwaySelection &other, DTIPathwayGroupID color, bool lockMode);

  void setValue (int i, DTIPathwayGroupID value) { _assignment[i] = value; }

  void setDirty (bool flag) { _dirty = flag; }
  bool isDirty () { return _dirty; }

  DTIPathwayAssignment *copy();

 private:

  int _cached_num_selected;
  int _num_pathways;
  DTIPathwayGroupID *_assignment;
  bool _dirty;

};

#endif
