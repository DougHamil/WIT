#ifndef DTI_TEND_PATH_TRACER
#define DTI_TEND_PATH_TRACER

#include "DTIDeterministicPathTracer.h"

class DTITENDPathTracer : public DTIDeterministicPathTracer {

 public:
  DTITENDPathTracer(int algoID, DTISeedGenerator *seedGenerator, DTIPathwayFactoryInterface *pathwayFactory, DTIVolumeInterface *volume, ScalarVolumeInterface *faVolume, ScalarVolumeInterface *maskVolume, double faThresh, double angleThresh, double stepSizeMm);
  virtual ~DTITENDPathTracer() {} 

  virtual DTIVector computeNextDirection (const DTIVector &incoming,
					 const DTIVector &curPosition);

  virtual bool shouldTerminate (const DTIVector &incoming,
				const DTIVector &outgoing);

  static DTIVector computeTENDDirection (const DTIVector &incoming,
					 const DTITensor &tensor);
  
};

#endif
