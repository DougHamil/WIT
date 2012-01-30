#ifndef PATHWAY_SELECTION_H
#define PATHWAY_SELECTION_H

class DTIPathwaySelection {

 public:
  DTIPathwaySelection(int numPathways, bool init = false);
  virtual ~DTIPathwaySelection();

  int getNumSelectedPathways();
  int getNumPathways () { return _num_pathways; }
  bool operator[] (int i) const { return _selection[i]; }
  void intersectWith (const DTIPathwaySelection &other);
  void append (const DTIPathwaySelection &other);
  void remove (const DTIPathwaySelection &other);
  void replace (const DTIPathwaySelection &other);

  void setValue (int i, bool value) { _selection[i] = value; }

  void setDirty (bool flag) { _dirty = flag; }
  bool isDirty () { return _dirty; }
  DTIPathwaySelection *copy();

 private:

  int _cached_num_selected;
  int _num_pathways;
  bool *_selection;
  bool _dirty;

};

#endif
