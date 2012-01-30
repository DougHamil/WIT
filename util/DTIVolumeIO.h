/** Copyright (c) 2005, The Board of Trustees of Leland Stanford Junior 
University. All rights reserved. **/

#ifndef DTI_VOLUME_IO_H
#define DTI_VOLUME_IO_H

class DTIVolume;

#include "typedefs.h"
#include <fstream>
class ScalarVolume;

class DTIVolumeIO {

 private:
  DTIVolumeIO() {}
  virtual ~DTIVolumeIO() {}
 public:


  static void adjustMatrices (float xform[4][4], float invXform[4][4]);
  static void saveScalarVolumeMat (ScalarVolume *scalarVolume, const char* filename);
  static DTIVolume *loadVolume(const char *filename);
  static DTIVolume *loadVolumeNifti(const char *filename, const int tensorOrder[6]);
  static DTIVolume *loadFSLTensorVolume(const char *baseFilename);

  static bool isValidBackground (const char *filename);
  static void saveScalarVolume (ScalarVolume *scalarVolume, std::ofstream &myStream);
  static void saveScalarVolumeNifti (ScalarVolume *scalarVolume, const char* filename);
  static ScalarVolume *loadScalarVolume (std::ifstream &myStream);
  static ScalarVolume *loadScalarVolumeNifti(const char* filename);

};

#endif
