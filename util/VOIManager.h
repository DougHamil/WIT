/** Copyright (c) 2005, The Board of Trustees of Leland Stanford Junior 
University. All rights reserved. **/

#ifndef VOI_MANAGER
#define VOI_MANAGER

class DTIFilterVOI;

class VOIManager {

 protected:
  VOIManager() {}
  virtual ~VOIManager() {}

 public:
  virtual DTIFilterVOI *getVOIFilter (const char *filterName) = 0;

};

#endif
