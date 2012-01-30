#ifndef DTI_UNIFORM_SEED_GENERATOR
#define DTI_UNIFORM_SEED_GENERATOR

#include "DTISeedGenerator.h"
#include <util/typedefs.h>
class DTIVolumeInterface;

class DTIUniformSeedGenerator : public DTISeedGenerator {

 public:

  DTIUniformSeedGenerator(DTIVolumeInterface *volume, double sampleSpread);
  virtual ~DTIUniformSeedGenerator() {}

  void initialize();
  void getNextSeed (DTIVector &pt);
  virtual bool finished() const;
  double getZ() { return _z; }

 protected:
  double _x, _y, _z;
  double _x_step, _y_step, _z_step;
  unsigned int _x_dim, _y_dim, _z_dim;
  

};

#endif
