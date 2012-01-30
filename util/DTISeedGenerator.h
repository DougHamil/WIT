#ifndef SEED_GENERATOR_H
#define SEED_GENERATOR_H

#include <util/typedefs.h>

class DTISeedGenerator {

 public:
  DTISeedGenerator() {}
  virtual ~DTISeedGenerator() {}

  virtual void getNextSeed (DTIVector &pt) = 0;
  virtual void initialize () = 0;
  virtual bool finished() const = 0;
 
};

#endif
