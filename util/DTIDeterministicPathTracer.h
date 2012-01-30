/** Copyright (c) 2003, The Board of Trustees of Leland Stanford Junior 
University. All rights reserved. **/

#ifndef DETERM_PATH_TRACER_H
#define DETERM_PATH_TRACER_H

#include "DTIPathTracer.h"
#include <util/typedefs.h>

class DTIPathwayFactoryInterface;
class DTIVolumeInterface;
class ScalarVolumeInterface;
class DTISeedGenerator;
class DTIDeterministicPathTracer : public DTIPathTracer {

 public:

  DTIDeterministicPathTracer(int algoID, DTISeedGenerator *seedGenerator, DTIPathwayFactoryInterface *pathwayFactory, DTIVolumeInterface *volume, ScalarVolumeInterface *faVolume, ScalarVolumeInterface *maskVolume, double faThresh, double angleThresh, double stepSizeMm);

  virtual ~DTIDeterministicPathTracer() {}

  DTIPathwayInterface *computeNextPath ();

  virtual DTIVector computeNextDirection (const DTIVector &incoming,
					 const DTIVector &curPosition) = 0;

  virtual bool shouldTerminate (const DTIVector &incoming,
				const DTIVector &outgoing) = 0;
 protected:
   
  double _angle_thresh;
  double _fa_thresh;
  ScalarVolumeInterface *_fa_volume;
  ScalarVolumeInterface *_mask_volume;
};

#endif
