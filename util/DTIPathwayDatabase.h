/** Copyright (c) 2005, The Board of Trustees of Leland Stanford Junior 
University. All rights reserved. **/

#ifndef FIBER_TRACT_DATABASE_H
#define FIBER_TRACT_DATABASE_H

class DTIPathway;
class DTIFilter;
class DTIFilterTree;
class VOIManager;
struct DTIPathwayStatisticHeader;
class DTIPathwayAlgorithmHeader;
class ComputeListener;
class DTIVolumeInterface;
class ScalarVolumeInterface;

#include "typedefs.h"

#include <list>
#include <vector>

enum StatConstraint {
  STAT_CONSTRAINT_NONE = 0,
  STAT_CONSTRAINT_LUMINANCE = 1,
  STAT_CONSTRAINT_STAT = 2
};

class DTIPathwayDatabase {
  friend class DTIPathwayIO;

 public:
  DTIPathwayDatabase();
  virtual ~DTIPathwayDatabase();

  DTIPathwayDatabase *copy();
  void addPathway (DTIPathway *pathway);
  void removePathway (DTIPathway *pathway);
  // Query stuff goes here (?)

  // void addPathwayPropertyFilter (DTIFilter *filter);
  // void removePathwayPropertyFilter (DTIFilter *filter);
  // void clearPathwayPropertyFilters();
  
  // DTIPathwayDatabase *getMatchingPathways();

  //  void setVOIFilter (const char *filtertext, VOIManager *manager);
  // const char * getVOIFilter ();

  //  bool doesPathwayMatch (DTIPathway *pathway);

  //  DTIPathwayDatabase *getFilterMatches (DTIFilter *filter);

  //void setFilterOp (DTIFilterOperation op);
  int getNumFibers();
  DTIPathway *getPathway(int index);
  void replacePathway (int index, DTIPathway *newPathway);

  int computeStatIndex (StatConstraint encoding, bool pointStatistic, int index);

  int getNumAlgorithms ();

  int getNumPathStatistics(StatConstraint encoding = STAT_CONSTRAINT_NONE);
  DTIPathwayStatisticHeader *getPathStatisticHeader (int id);
  std::vector<DTIPathwayStatisticHeader *> *getPathStatisticHeaders() { return &_pathway_statistic_headers; }

  int getNumPointStatistics(StatConstraint encoding = STAT_CONSTRAINT_NONE);

  void addStatisticHeader (DTIPathwayStatisticHeader *header);
  void clearStatisticHeaders();
  void setupStandardStatisticHeaders();

  double computeMinValue (PathwayProperty property);
  double computeMaxValue (PathwayProperty property);

  void computeAllStats(DTIVolumeInterface *tensors, ScalarVolumeInterface *faVol, ComputeListener *listener);

  DTIPathwayAlgorithmHeader *getPathAlgorithmHeader (int id);
  std::vector<DTIPathwayAlgorithmHeader *> *getPathAlgorithmHeaders() { return &_pathway_algorithm_headers; }
  
  void addAlgorithmHeader (DTIPathwayAlgorithmHeader *header);
  
  void setTransformMatrix (const double mx[16]);

  const double *getTransformMatrix() { return &(_transform_mx[0]);}

  void getBoundingBox (double xRange[2], double yRange[2], double zRange[2]);

  double getMinValue (PathwayProperty property);
  double getMaxValue (PathwayProperty property);

  void setVoxelSize(double vox_size[3]);
  void getVoxelSize(double vox_size[3]);

 private:

  const double *getMinValues();
  const double *getMaxValues();

  std::vector<DTIPathway *> _fiber_pathways;

  std::vector<DTIPathwayStatisticHeader *> _pathway_statistic_headers;

  std::vector<DTIPathwayAlgorithmHeader *> _pathway_algorithm_headers;

  double *_cached_min_values;
  double *_cached_max_values;

  double _transform_mx[16];

  bool _recompute_min_needed;
  bool _recompute_max_needed;

  double _mm_scale[3];
  unsigned int _scene_dim[3];
  double voxelSize[3];
};


#endif
