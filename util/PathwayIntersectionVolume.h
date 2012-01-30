#ifndef PATHWAY_INTERSECTION_VOLUME_H
#define PATHWAY_INTERSECTION_VOLUME_H

#include "ScalarVolume.h"
class DTIPathway;

class PathwayIntersectionVolume : public ScalarVolume {
 public:
  PathwayIntersectionVolume (ScalarVolume *currentBackground);
  void munchPathway (DTIPathway *pathway);
  void rasterizeSegment (const double a[3], const double b[3]);
  //  void munchPoint (const double pt[3]);
  void normalize();
 protected:
  void setVoxel (double x, double y, double z);
 private:
  double _max_intersect;

};

#endif
