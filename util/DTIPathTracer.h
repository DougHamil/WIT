/** Copyright (c) 2003, The Board of Trustees of Leland Stanford Junior 
University. All rights reserved. **/

#ifndef PATH_TRACER_H
#define PATH_TRACER_H

class DTIVolumeInterface;
class DTIPathwayInterface;
class ScalarVolumeInterface;
class DTIPathwayFactoryInterface;
class DTISeedGenerator;

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include <util/typedefs.h>

class DTIPathTracer {
 protected:

  DTIPathTracer(int algoID, DTISeedGenerator *generator, DTIPathwayFactoryInterface *pathwayFactory, DTIVolumeInterface *volume, double stepSizeMm);

  virtual ~DTIPathTracer() {}

 public:

  virtual void initialize();

  virtual void setSeedGenerator (DTISeedGenerator *generator) { _seed_generator = generator; }

  virtual DTIPathwayInterface *computeNextPath() = 0;

  static DTIVector getMajorEigenvector (const DTITensor &tensor);

 protected:

  static bool inBounds (const DTIVector &pt, DTIVolumeInterface *volume);

  int _algo_id;
  DTISeedGenerator *_seed_generator;
  double _step_size_mm;
  DTIVector *_step_size_voxel;
  DTIPathwayFactoryInterface *_pathway_factory;
  DTIVolumeInterface *_volume;

};

#endif
