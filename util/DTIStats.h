#ifndef DTI_STATS_H
#define DTI_STATS_H

class DTIPathwayDatabase;
class DTIVolumeInterface;
class ScalarVolumeInterface;
class DTIPathwayInterface;
class ScalarVolume;

#include "typedefs.h"

class DTIStats {
 public:

  static double computeLength (DTIPathwayInterface *path, PathwayProperty id);
  static double computeCurvature (DTIPathwayInterface *path, PathwayProperty id);
  static double computeFA (DTIPathwayInterface *path, PathwayProperty id, ScalarVolume *faVol);
  static int    computeSampleOrder(DTIPathwayInterface *path, PathwayProperty id, int order);
  static double computeDistance (const DTIVector &p1, const DTIVector &p2);
 private:
  static double computeLocalCurvature (const DTIVector &pt1, const DTIVector &pt2, const DTIVector &pt3);
};


#endif
