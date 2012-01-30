#ifndef DTI_STT_PATH_TRACER
#define DTI_STT_PATH_TRACER

#include "DTIDeterministicPathTracer.h"

class DTISTTPathTracer : public DTIDeterministicPathTracer {

 public:
  DTISTTPathTracer(int algoID, DTISeedGenerator *seedGenerator, DTIPathwayFactoryInterface *pathwayFactory, DTIVolumeInterface *volume, ScalarVolumeInterface *faVolume, ScalarVolumeInterface *maskVolume, double faThresh, double angleThresh, double stepSizeMm, NumericalIntegrationMethod method);
  virtual ~DTISTTPathTracer() {} 


  virtual DTIVector computeNextDirection (const DTIVector &incoming,
					 const DTIVector &curPosition);

  virtual bool shouldTerminate (const DTIVector &incoming,
				const DTIVector &outgoing);

  static bool checkAngle (const DTIVector &a, const DTIVector &b, 
			  double angleThresh,
			  int &direction, double &angle);

 private:

  
  virtual DTIVector numericalIntegrate (const DTIVector &incoming,
					const DTIVector &curPosition,
					NumericalIntegrationMethod method);

  virtual DTIVector integrateRK2 (const DTIVector &incoming,
				  const DTIVector &curPosition);

  virtual DTIVector integrateRK4 (const DTIVector &incoming,
				  const DTIVector &curPosition);

  NumericalIntegrationMethod _integration_method;

};

#endif
