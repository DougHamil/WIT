/** Copyright (c) 2005, The Board of Trustees of Leland Stanford Junior 
University. All rights reserved. **/

#ifndef FA_VOLUME_INTERFACE_H
#define FA_VOLUME_INTERFACE_H

class ScalarVolumeInterface {
 public:
  
  virtual double getFA (int x, int y, int z, int c) const = 0;
  virtual bool inBounds (const DTIVector &vec) const = 0;
  
 protected:
  ScalarVolumeInterface() {}
  virtual ~ScalarVolumeInterface() {}
};

#endif
