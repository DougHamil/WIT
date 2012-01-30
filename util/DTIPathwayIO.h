/** Copyright (c) 2005, The Board of Trustees of Leland Stanford Junior 
University. All rights reserved. **/

#ifndef FIBER_TRACT_IO
#define FIBER_TRACT_IO

class DTIPathwayDatabase;
class DTIPathway;
class ScalarVolume;
class VOIManager;
class DTIPathDistanceMatrix;
class DTIPathwaySelection;

#include <fstream>
#include <vector>
#include <stdint.h>

void writeFloat (float f, std::ofstream &theStream);
void writeInt (int i, std::ofstream &theStream);
float readFloat (std::ifstream &theStream);
int readInt (std::ifstream &theStream);

class DTIPathwayIO {

 private:
  DTIPathwayIO() {}
  virtual ~DTIPathwayIO() {}

  static void savePathway (DTIPathway *pathway, std::ofstream &theStream, DTIPathwayDatabase *db, const double *mx, bool debug);
  static DTIPathway *loadPathway (std::istream &theStream, DTIPathwayDatabase *db, bool multiplyMatrix);

 public:

  static DTIPathwayDatabase *loadDatabase(std::istream &pdbStream, DTIPathwayDatabase *db, DTIPathwaySelection *sel,int versionNumber, unsigned int headerSize);
  static DTIPathwayDatabase *loadDatabaseNew(std::istream &pdbStream, DTIPathwayDatabase * oldPdb, bool bComputeRAPID, double ACPC[3], double vox_size[3]);
  static void loadDatabasePDB2Ver3(std::istream &theStream, DTIPathwayDatabase *db, bool multiplyMatrix);
  static void openDatabase (DTIPathwayDatabase *db, const char *filename);
  static uint64_t* appendDatabaseFile (DTIPathwayDatabase *pathways, const char* filename, DTIPathwaySelection *selection = NULL);
  static void appendFileOffsetsToDatabaseFile (int numPathways, const uint64_t *fileOffsets, const char *filename);
  static void saveDatabasePDB(DTIPathwayDatabase *db, std::ostream &theStream, double ACPC[3], DTIPathwaySelection *selection);
  static void saveDistanceMatrix (DTIPathDistanceMatrix *matrix, std::ofstream &theStream);
  static DTIPathDistanceMatrix *loadDistanceMatrix (std::ifstream &theStream);
  static DTIPathwaySelection *loadPathwaySelection (std::ifstream &theStream);
  static void savePathwaySelection (DTIPathwaySelection *sel, std::ofstream &theStream);
};


#endif
