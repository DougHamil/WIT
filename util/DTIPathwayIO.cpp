/** Copyright (c) 2005, The Board of Trustees of Leland Stanford Junior 
University. All rights reserved. **/

/*************************************************************************
 *    NAME: David Akers and Anthony Sherbondy
 *     
 *    FILE: DTIPathwayIO.cpp
 *    DATE: Thu Feb 26 21:14:26 PST 2004
 *************************************************************************/
#include "DTIPathwayIO.h"
#include "DTIPathwayDatabase.h"
#include "DTIPathway.h"
#include "ScalarVolume.h"
#include "DTIPathwayStatisticHeader.h"
#include "DTIPathDistanceMatrix.h"
#include "DTIPathwayAlgorithmHeader.h"
#include "vtkMatrix4x4.h"
#include "DTIPathwaySelection.h"
#include "io_utils.h"

#include <time.h>
#include <fstream>

#include <iostream>
using namespace std;

//#define HACK

#include <algorithm> //required for std::swap

#define ByteSwap5(x) ByteSwap((unsigned char *) &x,sizeof(x))

void ByteSwap(unsigned char * b, int n)
{
   register int i = 0;
   register int j = n-1;
   while (i<j)
   {
      std::swap(b[i], b[j]);
      i++, j--;
   }
}

bool IsBigEndian()
{
   short word = 0x4321;
   if((*(char *)& word) != 0x21 )
     return true;
   else 
     return false;
}

void writeDouble (double d, std::ofstream &theStream) {
  if (IsBigEndian()) {
    ByteSwap5 (d);
  }
  theStream.write ((char *) &d, sizeof(double));
}

void writeFloat (float f, std::ofstream &theStream) {
  if (IsBigEndian()) {
    ByteSwap5 (f);
  }
  theStream.write ((char *) &f, sizeof(float));
}

void writeInt (int i, std::ofstream &theStream) {
  if (IsBigEndian()) {
    ByteSwap5 (i);
  }
  theStream.write ((char *) &i, sizeof(int));
}

void writeInt64 (uint64_t l, std::ofstream &theStream) {
  if (IsBigEndian()) {
    ByteSwap5 (l);
  }
  theStream.write ((char *) &l, sizeof(uint64_t));
}


void writeInt32 (uint32_t l, std::ofstream &theStream) {
  if (IsBigEndian()) {
    ByteSwap5 (l);
  }
  theStream.write ((char *) &l, sizeof(uint32_t));
}

int readInt (std::ifstream &theStream) {
  int i;
  theStream.read ((char *) &i, sizeof(int));
  if (IsBigEndian()) {
    ByteSwap5 (i);
  }
  return i;
}

uint64_t readInt64 (std::ifstream &theStream) {
  uint64_t l;
  theStream.read ((char *) &l, sizeof(uint64_t));
  if (IsBigEndian()) {
    ByteSwap5 (l);
  }
  return l;
}

uint32_t readInt32 (std::ifstream &theStream) {
  uint32_t l;
  theStream.read ((char *) &l, sizeof(uint32_t));
  if (IsBigEndian()) {
    ByteSwap5 (l);
  }
  return l;
}

double readDouble (std::ifstream &theStream) {
  double d;
  theStream.read ((char *) &d, sizeof(double));
  if (IsBigEndian()) {
    ByteSwap5 (d);
  }
  return d;
}

float readFloat (std::ifstream &theStream) {
  float f;
  theStream.read ((char *) &f, sizeof(float));
  if (IsBigEndian()) {
    ByteSwap5 (f);
  }
  return f;
}


void writeStatHeader (DTIPathwayStatisticHeader *header, std::ostream &theStream) {
  theStream.write ((char *) header, sizeof(*header));
}

void readStatHeader (DTIPathwayStatisticHeader &header, std::istream &theStream) {
  theStream.read ((char *) &header, sizeof (header));
}

void writeAlgoHeader (DTIPathwayAlgorithmHeader *header, std::ostream &theStream) {
  theStream.write ((char *) header, sizeof(*header));
}
 
void readAlgoHeader (DTIPathwayAlgorithmHeader &header, std::istream &theStream) {
  theStream.read ((char *) &header, sizeof (header));
}


/*************************************************************************
 * Function Name: DTIPathwayIO::savePathway
 * Parameters: DTIPathway *pathway, fstream &theStream
 * Returns: void
 * Effects: 
 *************************************************************************/
void
DTIPathwayIO::savePathway(DTIPathway *pathway, std::ofstream &theStream, DTIPathwayDatabase *db, const double *inverseTransformMx, bool debug)
{
  unsigned int headerSize;
  headerSize = 3* sizeof (int) + db->getNumPathStatistics()*sizeof (double);
  writeInt (headerSize, theStream);

  int numPoints = pathway->_point_vector.size();
  writeInt (numPoints, theStream);
  int algo = (int) pathway->getPathwayAlgorithm();
  writeInt (algo, theStream);
  int seedPointIndex = pathway->getSeedPointIndex();
  writeInt (seedPointIndex, theStream);
  for (int i = 0; i < db->getNumPathStatistics(); i++) {
    writeDouble (pathway->getPathStatistic(i), theStream);
  }
  //  double length = pathway->computeLengthMM();
  //writeDouble (length, theStream);
  //  double medianFA = pathway->computeMedianFA(vol, delta);
  //  writeDouble (medianFA, theStream);
  //  double minFA = pathway->computeMinFA(vol, delta);
  //  writeDouble (minFA, theStream);
  //#ifdef HACK
  //  double meanCurv = (double) pathway->getClusterIndex();
  //#else
  //  double meanCurv = pathway->computeMeanCurvature();
  //#endif
  //  writeDouble(meanCurv, theStream);
  //  double maxAngle = pathway->computeMaxAngle();
  //  writeDouble (maxAngle, theStream);
  //  double prior = pathway->getCachedPrior();
  //  writeDouble (prior, theStream);
  //  double likelihood = pathway->getCachedLikelihood();
  //  writeDouble (likelihood, theStream);

  for (std::vector<DTIGeometryVector *>::iterator iter = pathway->_point_vector.begin(); iter != pathway->_point_vector.end(); iter++) {
    DTIGeometryVector *vec = *iter;
    double p[4] = {(*vec)[0], (*vec)[1], (*vec)[2], 1};
    double pOut[4];
	// TODO: Remove dependency on VTK here...
    vtkMatrix4x4::MultiplyPoint(inverseTransformMx, p, pOut);
    
    writeDouble(pOut[0], theStream);
    writeDouble(pOut[1], theStream);
    writeDouble(pOut[2], theStream);
  }
  
  for (int i = 0; i < db->getNumPathStatistics(); i++) {
    if (debug) {
      std::cerr << "num points of stats: " << pathway->_point_stat_array[i].size() << std::endl;
    }
    if (db->getPathStatisticHeader(i)->_is_computed_per_point) {
      for (std::vector<double>::iterator iter = pathway->_point_stat_array[i].begin(); iter != pathway->_point_stat_array[i].end(); iter++) {
	double stat = *iter;
	writeDouble (stat, theStream);
      }
    }
  }
    
}


/*************************************************************************
 * Function Name: DTIPathwayIO::loadPathway
 * Parameters: fstream &theStream
 * Returns: DTIPathway *
 * Effects: 
 *************************************************************************/
DTIPathway *
DTIPathwayIO::loadPathway(std::istream &theStream, DTIPathwayDatabase *db, bool multiplyMatrix)
{
  int numPathStatistics = db->getNumPathStatistics();

  unsigned int headerSize;
  headerSize = readScalar<int>(theStream);
  unsigned int pos = theStream.tellg();
  int numPoints = readScalar<int>(theStream);
  int algoInt = readScalar<int>(theStream);
  DTIPathway *pathway = new DTIPathway((DTIPathwayAlgorithm) algoInt);
  int seedPointIndex = readScalar<int>(theStream);
  pathway->setSeedPointIndex (seedPointIndex);

  int i = 0;
 
  double *pathStat = new double[numPathStatistics];

  for (i = 0; i < numPathStatistics; i++)
    pathStat[i] = readScalar<double>(theStream);

  theStream.seekg (pos + headerSize, std::ios::beg);
	
  const double *mx = db->getTransformMatrix();

  for (i = 0; i < numPoints; i++) {
    double p[4];
    p[0] = readScalar<double> (theStream);
    p[1] = readScalar<double> (theStream);
    p[2] = readScalar<double> (theStream);
    p[3] = 1.0;
    double pOut[4];
    if (multiplyMatrix) {
      vtkMatrix4x4::MultiplyPoint (mx, p, pOut);
    }
    else {
      pOut[0] = p[0];
      pOut[1] = p[1];
      pOut[2] = p[2];
    }
    DTIVector v(3, pOut);
    pathway->append (v);
  }

  pathway->initializePathStatistics(numPathStatistics, db->getPathStatisticHeaders(), true);

  for (i = 0; i < numPathStatistics; i++) {
    pathway->setPathStatistic(i, pathStat[i]);
    if (db->getPathStatisticHeader(i)->_is_computed_per_point) {
      for (int j = 0; j < numPoints; j++) {
        double stat;
        stat = readScalar<double>(theStream);
        pathway->setPointStatistic (j, i, stat);
      }
    }
  }
  delete[] pathStat;
  return pathway;
}


/*************************************************************************
 * Function Name: DTIPathwayIO::loadDatabase
 * Parameters: fstream &theStream
 * Returns: DTIPathwayDatabase *
 * Effects: 
 *************************************************************************/
DTIPathwayDatabase *
DTIPathwayIO::loadDatabase(std::istream &pdbStream,  DTIPathwayDatabase *db, DTIPathwaySelection *selection,int versionNumber, unsigned int headerSize)
{
  pdbStream.seekg (headerSize, std::ios::beg);

  int numPathways = readScalar<int>(pdbStream);

  std::cerr << "Number of pathways: " << numPathways << std::endl;

  bool hackSubsample = false;

#ifdef _MSC_VER

  if (numPathways > 40000) {
	  std::cerr << "HACK - subsampling pathways." << std::endl;
	  hackSubsample = true;
  }

#endif

  streampos firstPathwaySeek = pdbStream.tellg();
  
  //  pdbStream.seekg (numPathways*sizeof(unsigned long), std::ios::end);

  uint64_t *fileOffsets = new uint64_t [numPathways];

  bool read64Bit = false;

  pdbStream.seekg (0, std::ios::end);
  streampos length = pdbStream.tellg();
  pdbStream.seekg (length-(streampos)(numPathways*sizeof(uint64_t)));
  int i;
  if (versionNumber == 2) {
    read64Bit = true;
  }
  else {
    uint64_t firstOffset = readScalar<uint64_t>(pdbStream);
    std::cerr << "Comparing " << firstPathwaySeek << " to " << firstOffset << std::endl;
    if (firstPathwaySeek == (streampos) firstOffset) {
      read64Bit = true;
      pdbStream.seekg (length-(streampos)(numPathways*sizeof(uint64_t)));
    }
  } 
	
  if (read64Bit) {
    for (i = 0; i < numPathways; i++) {
      fileOffsets[i] = (uint64_t) readScalar<uint64_t>(pdbStream);
    }
  }
  else {
    std::cerr << "Reading old 32-bit format..." << std::endl;

    pdbStream.seekg (length-(streampos)(numPathways*sizeof(uint32_t)));
    for (i = 0; i < numPathways; i++) {
      fileOffsets[i] = (uint64_t) readScalar<uint32_t>(pdbStream);
    }
  }

  pdbStream.seekg (headerSize+sizeof(int), std::ios::beg);

  //if (numPathways > 15000) exit(0);
  
  //  db->setVoxelSize (mmScale);
  //  db->setSceneDimension (sceneDim);
  double duration=0;

  int numAccepted = 0;
  int count = 0;
  //  cerr << "version num: " << versionNumber << endl;
  for (i = 0; i < numPathways; i++)
  {
    if (selection && !(*selection)[i])
      continue;

    if( count%1000 == 0 )
      std::cerr << "pathway: " << count << std::endl;
    count++;
    if (count % 3 != 0 && hackSubsample)
      continue;

    pdbStream.seekg (fileOffsets[i], std::ios::beg);  
    DTIPathway *pathway = loadPathway (pdbStream, db, true);
    pathway->setID(numAccepted);
    numAccepted++;
    db->addPathway (pathway);
    clock_t start, finish;
    start = clock();
    pathway->getCollisionModel();
    finish = clock();
    duration += (double)(finish - start) / CLOCKS_PER_SEC;
  }
  std::cerr << "Finished loading pathways in " << duration<< " seconds." << std::endl;
  delete[] fileOffsets;
  return db;
}


/*************************************************************************
 * Function Name: DTIPathwayIO::openDatabase
 * Parameters: DTIPathwayDatabase *pathways, fstream &theStream
 * Returns: void
 * Effects: 
 *************************************************************************/
void
DTIPathwayIO::openDatabase(DTIPathwayDatabase *db, const char *filename)
{
  ofstream theStream (filename, std::ios::out | std::ios::binary);
  unsigned int headerSize;
  headerSize = 4 * sizeof (int) + 16 * sizeof(double) + db->getNumPathStatistics()*sizeof (DTIPathwayStatisticHeader) + db->getNumAlgorithms() * sizeof (DTIPathwayAlgorithmHeader);

//  cerr << "Header size: " << headerSize << endl;

  writeInt (headerSize, theStream);

  // write matrix transform:
  for (int i = 0; i < 16; i++) {
    writeDouble (db->_transform_mx[i], theStream);
  }

  writeInt (db->getNumPathStatistics(), theStream);
  
  for (int i = 0; i < db->getNumPathStatistics(); i++) {
    writeStatHeader (db->getPathStatisticHeader(i), theStream);
  }
  
  writeInt (db->getNumAlgorithms(), theStream);

  //  for (int i = 0; i < pathways->getNumAlgorithms(); i++) {
  if (db->getNumAlgorithms() == 0) {
    cerr << "ERROR: Missing algorithm header information in database." << endl;
    cerr << "Cannot open database for writing..." << endl;
    return;
  }
  for (int i = 0; i < db->getNumAlgorithms(); i++) {
    writeAlgoHeader (db->getPathAlgorithmHeader(i), theStream);
  }
  int versionNum = 2; // DTIQuery 1.2
  writeInt (versionNum, theStream);

  // zero pathways...
  writeInt (0, theStream);
  theStream.close();
}



/***********************************************************************
 *  Method: DTIPathwayIO::appendDatabaseFile
 *  Params: DTIPathwayDatabase *pathways, ScalarVolume *vol, const double ACPC[3], std::fstream &theStream
 * Returns: void
 * Effects: 
 ***********************************************************************/
uint64_t *
DTIPathwayIO::appendDatabaseFile(DTIPathwayDatabase *pathways, const char* filename, DTIPathwaySelection *selection)
{
  
//  cerr << "reading from " << filename << endl;
  std::ifstream inStream(filename, std::ios::in | std::ios::binary);

  if( !inStream ) {
    std::cerr << "Can't open input file \"" << filename << "\"" << std::endl;
    return NULL;
  }
  // Skip all header information
  
  int headerSize = readInt(inStream);
  inStream.seekg(headerSize,std::ios::beg);
  
  int oldNumPaths = readInt(inStream);
//  cerr << "oldNumPaths: " << oldNumPaths << endl;
  inStream.close();
  
  if(oldNumPaths < 0) {
    std::cerr << "ERROR: Cannot append to pathway database - header is corrupted." << std::endl;
    return NULL;
  } 
  
  inStream.close();
  
  std::ofstream outStream (filename, std::ios::in | std::ios::out | std::ios::binary);
  // If the current database has more pathways then append them to the file
  int numAppendedPathways;
  if (selection) {
    numAppendedPathways = selection->getNumSelectedPathways();
//    cerr << "numAppendPaths: " << numAppendedPathways << endl;
  }
  else {
    numAppendedPathways = pathways->getNumFibers();
//    cerr << "numAppendedPaths (fibers): " << numAppendedPathways << endl;
  }
  uint64_t *fileOffsets = new uint64_t[numAppendedPathways];

  int newNumPaths = numAppendedPathways + oldNumPaths;
//  cerr << "newNumPaths: " << newNumPaths << endl;
  outStream.seekp(headerSize, std::ios::beg);
  writeInt (newNumPaths, outStream);
  int count=0;

  // compute inverse of transform matrix:
  double inverseMx[16];
  vtkMatrix4x4::Invert(pathways->_transform_mx, inverseMx);

  for (int i = 0; i < pathways->getNumFibers(); i++) {
    if (selection) {
      if (!(*selection)[i]) {
	continue;
      }
    }
    fileOffsets[count] = outStream.tellp();
    //    cerr << fileOffsets[i] << endl;
    DTIPathway *pathway = pathways->getPathway(i);
    savePathway (pathway, outStream, pathways, inverseMx, false);
    count++;
  }
  outStream.close();
  return fileOffsets;
}


/***********************************************************************
 *  Method: DTIPathwayIO::saveDistanceMatrix
 *  Params: DTIPathDistanceMatrix *matrix, std::ofstream &theStream
 * Returns: void
 * Effects: 
 ***********************************************************************/
void
DTIPathwayIO::saveDistanceMatrix(DTIPathDistanceMatrix *matrix, std::ofstream &theStream)
{
  int n = matrix->GetNumPathways();

  writeInt (n, theStream);

  int count = 0;
  float totalF = 0;
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < i; j++) {
      float f = matrix->GetDistance(i,j);
      writeFloat(f, theStream);
      count++;
      totalF += f;
    }
  }
  std::cerr << "Wrote " << count << "distances. Average distance = " << totalF / (count) << std::endl;
}


/***********************************************************************
 *  Method: DTIPathwayIO::loadDistanceMatrix
 *  Params: std::ifstream &theStream
 * Returns: DTIPathDistanceMatrix *
 * Effects: 
 ***********************************************************************/
DTIPathDistanceMatrix *
DTIPathwayIO::loadDistanceMatrix(std::ifstream &theStream)
{
  int n = readInt (theStream);
  DTIPathDistanceMatrix *matrix = new DTIPathDistanceMatrix(n);
  int count = 0;
  float totalF = 0;
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < i; j++) {
      float f = readFloat (theStream);
      matrix->SetDistance (i,j,f);
      count++;
      totalF += matrix->GetDistance (i,j);
    }
  }
  std::cerr << "Distance matrix contains " << count << " distances. Average distance = " << totalF / (count) << std::endl;
  return matrix;
}


/***********************************************************************
 *  Method: DTIPathwayIO::loadPathwaySelection
 *  Params: std::ifstream &theStream
 * Returns: DTIPathwaySelection *
 * Effects: 
 ***********************************************************************/
DTIPathwaySelection *
DTIPathwayIO::loadPathwaySelection(std::ifstream &theStream)
{
  int n = readInt (theStream);
  bool *selection = new bool[n];

  int i;
  for (i = 0; i < n; i++) {
    selection[i] = (bool) readInt (theStream);
  }
  DTIPathwaySelection *newSelection = new DTIPathwaySelection (n);
  for (i = 0; i < n; i++) {
    newSelection->setValue (i, selection[i]);
  }
  return newSelection;
    
}
/***********************************************************************
 *  Method: DTIPathwayIO::savePathwaySelection
 *  Params: DTIPathwaySelection *sel, const unsigned int *fileOffsets, std::ofstream &theStream
 * Returns: void
 * Effects: 
 ***********************************************************************/
void
DTIPathwayIO::savePathwaySelection(DTIPathwaySelection *sel, std::ofstream &theStream)
{
  int n = sel->getNumPathways();

  writeInt (n, theStream);
  int i;
  for (i = 0; i < n; i++) {
    writeInt ((*sel)[i], theStream);
  }
}


/***********************************************************************
 *  Method: DTIPathwayIO::appendFileOffsetsToDatabaseFile
 *  Params: const uint64_t *fileOffsets, const char *filename
 * Returns: void
 * Effects: 
 ***********************************************************************/
void
DTIPathwayIO::appendFileOffsetsToDatabaseFile(int numOffsets, const uint64_t *fileOffsets, const char *filename)
{
  ofstream outStream;
  outStream.open (filename, std::ios::in | std::ios::out | std::ios::binary);
  outStream.seekp (0, std::ios::end);
  for (int i = 0; i < numOffsets; i++) {
    writeInt64 (fileOffsets[i], outStream);
  }
  outStream.close();
}




/**************************************************
 * The Following is taken from Quench to load PDB3
 **************************************************/
DTIPathwayDatabase *
DTIPathwayIO::loadDatabaseNew(std::istream &pdbStream, DTIPathwayDatabase * oldPdb, bool bComputeRAPID, double ACPC[3], double vox_size[3])
{
  bool bPrintOut = true;

  if(bPrintOut) {
    cout << "###################################################################" << endl;
    cout << "#####               Begin PDB Printout                        #####" << endl;
    cout << "###################################################################" << endl;
  }

  // This function assumes that we want to load pathways into ACPC space
  // if the ACPC offset is NULL and MM space otherwise
  bool bAppend = oldPdb ? true:false;
  DTIPathwayDatabase *db = bAppend ? oldPdb : new DTIPathwayDatabase();

  // TONY 2009.08.14
  // ADDING half voxel offset to ACPC because contrack assumes samples are at the half integers
  // and the xform in the nifti image files assume measurments are at the integers
  if(vox_size)
  {
    db->setVoxelSize(vox_size);
    for(int ii=0; ii<3; ii++) ACPC[ii]+=0.5*vox_size[ii];
  }
  unsigned int headerSize;
  headerSize = readScalar<int> (pdbStream);

  if(bPrintOut)
    cout << "header size: " << headerSize << "(int32)" << endl;

  // This matrix is always the transform from pathway space into ACPC space
  // XXX Currently we only support pathways either in ACPC space or mm space.
  //     Therefore, we expect either the identity here or anything else means
  //     we are in mm space.
  double matrixEntry;
  for (int i = 0; i < 4; i++)
    for (int j = 0; j < 4; j++)
    {
      matrixEntry = readScalar<double> (pdbStream);
      db->_transform_mx[i*4+j] = matrixEntry;
    }

  /*
  if(bPrintOut)
    print_matrix(db->_transform_mx, 4, 4);
    */

  // See if we want the database in MM space rather than ACPC
  //if( ACPC!=NULL && DTIMath::sqr(M[3])<0.001 && DTIMath::sqr(M[7])<0.001 && DTIMath::sqr(M[11])<0.001 ) {
  if( ACPC!=NULL )
  {
    // Xform specifies ACPC space lets get these into MM
    // space by translating by the ACPC offset
    double* M = db->_transform_mx;
    M[3]  += ACPC[0];
    M[7]  += ACPC[1];
    M[11] += ACPC[2];
  }
  // Write out the matrix xform
  if(bPrintOut)
  {
    if(ACPC==NULL)
      std::cerr << "Pathway Transform (to AcPc):" << endl;
    else
      std::cerr << "Pathway Transform (to mm):" << endl;
    //print_matrix(db->_transform_mx, 4, 4);
  }

  int numStats = readScalar<int> (pdbStream);
  if(bPrintOut)
    cout << "num stats: " << numStats << "(int32)" << endl;

  if(oldPdb && numStats != oldPdb->getNumPathStatistics())
  {
    cout<<"The number of statistics should match in the old and new fibers. Cancelling"<<endl;
    return oldPdb;
  }

  for (int i = 0; i < numStats; i++)
  {
    DTIPathwayStatisticHeader *header = new DTIPathwayStatisticHeader;
    readStatHeader (*header, pdbStream);
    if(!oldPdb)
      db->addStatisticHeader(header);
    if(bPrintOut)
    {
      cout << "Stat Header " << i << " {" << endl;
      //header->print();
      cout << "}" << endl;
    }
  }

  int numAlgos = readScalar<int> (pdbStream);
  if(bPrintOut)
    cout << "num algs: " << numAlgos << "(int32)" << endl;

  if (numAlgos< 1 || numAlgos > 10)
  {
    if(!oldPdb)
    {
      std::cerr << "Warning: Unusual number of algorithms. Assuming old path format; will create default algorithm headers." << std::endl;
      DTIPathwayAlgorithmHeader *simulatedHeader = new DTIPathwayAlgorithmHeader;
#ifdef _MSC_VER
	  strcpy_s(simulatedHeader->_algorithm_name, "STT");
#else
      strcpy (simulatedHeader->_algorithm_name, "STT");
#endif
      db->addAlgorithmHeader(simulatedHeader);

      simulatedHeader = new DTIPathwayAlgorithmHeader;
#ifdef _MSC_VER
	  strcpy_s(simulatedHeader->_algorithm_name, "TEND");
#else
      strcpy (simulatedHeader->_algorithm_name, "TEND");
#endif
      db->addAlgorithmHeader(simulatedHeader);

      simulatedHeader = new DTIPathwayAlgorithmHeader;
#ifdef _MSC_VER
	  strcpy_s(simulatedHeader->_algorithm_name, "STAT");
#else
      strcpy (simulatedHeader->_algorithm_name, "STAT");
#endif
      db->addAlgorithmHeader(simulatedHeader);
    }
  }
  else
  {
    for (int i = 0; i < numAlgos; i++)
    {
      DTIPathwayAlgorithmHeader *algoHeader = new DTIPathwayAlgorithmHeader ;
      readAlgoHeader (*algoHeader, pdbStream);
      if(!oldPdb)
        db->addAlgorithmHeader(algoHeader);
    }
  }

  int versionNumber = readScalar<int> (pdbStream);
  if(bPrintOut) cout << "version number: " << versionNumber << "(int32)" << endl;
  // End of header

  // call the appropriate version #
  switch(versionNumber)
  {
  case 1:
    loadDatabase(pdbStream, db, NULL, true, headerSize);
    break;
  case 3:
    loadDatabasePDB2Ver3(pdbStream, db, true);
    break;
  default:
    delete db;
    cerr << "DTIQuery only supports PDB Version 1 and 3" << endl;
    return NULL;
    break;
  }

  if(bPrintOut) {
    cout << "###################################################################" << endl;
    cout << "#####               End PDB Printout                          #####" << endl;
    cout << "###################################################################" << endl;
  }

  if(bPrintOut)
    std::cerr << "Finished loading pathways." << std::endl;

  return db;
}



void DTIPathwayIO::loadDatabasePDB2Ver3(std::istream &theStream, DTIPathwayDatabase *db, bool multiplyMatrix)
{
  bool bPrintOut      = true;
  int numPathways     = readScalar<int> (theStream);
  int numPathStatistics = db->getNumPathStatistics();
  int nCntPercent     = 10;

  // Read the number of points per fiber
  int *points_per_fiber = new int[numPathways];
  theStream.read((char*)points_per_fiber, sizeof(int)*numPathways);

  // Compute the total # of points across all fibers
  int total_pts=0;
  for(int i = 0; i < numPathways; i++)
    total_pts += points_per_fiber[i];

  // Read all the points
  double *fiber_points = new double[total_pts*3];
  theStream.read((char*)fiber_points, sizeof(double)*total_pts*3);

  if(bPrintOut)
    fprintf(stderr,"Loaded     ");

  for (int k = 0, points_read=0; k < numPathways; k++)
  {
    int iDonePercent = (float)k*100.0f/(numPathways-1);
    if( iDonePercent >= nCntPercent )
    {
      if(bPrintOut)
        fprintf(stderr,"\b\b\b\b%3d%%", iDonePercent);
      nCntPercent+=10;
    }

    // load a single pathway
    int numPoints = points_per_fiber[k];
    if(bPrintOut)
      cout << "num points: " << numPoints << "(int32)" << endl;

    DTIPathway *pathway = new DTIPathway((DTIPathwayAlgorithm) 0);
    int seedPointIndex = 1;
    if(bPrintOut)
      cout << "seed index: " << seedPointIndex << "(int32)" << endl;
    pathway->setSeedPointIndex(seedPointIndex);

    const double *mx = db->getTransformMatrix();

    pathway->_point_vector.reserve(numPoints);
    //pathway->_path_grow_weight.reserve(numPoints);
    for (int i = 0; i < numPoints; i++, points_read+=3)
    {
      double *p = fiber_points + points_read ;
      double pFloat[3];

      if(bPrintOut) cout << "point " << i << ": [" << p[0] << "(double)" << "," << p[1] << "(double)" << "," << p[2] << "(double)" << "]" << endl;
      if (multiplyMatrix)
      {
        for(int j = 0; j < 3; j++)
          pFloat[j] = p[0]*mx[0+j*4] + p[1]*mx[1+j*4] +p[2]*mx[2+j*4] + mx[3+j*4];
      }
      DTIVector v(3, pFloat);
      pathway->append (v);
    }

    pathway->initializePathStatistics(numPathStatistics, db->getPathStatisticHeaders(), true);
    pathway->setID(k);
    db->addPathway (pathway);

    // build the rapid model
    pathway->getCollisionModel();
  }
  delete [] fiber_points;

  //Read the per fiber stats
  for(int i = 0; i < numPathStatistics; i++)
  {
    double *per_fiber_stat = new double[numPathways];
    theStream.read((char*)per_fiber_stat, sizeof(double)*numPathways);

    for(int j = 0, points_read = 0; j < numPathways; j++)
      db->getPathway(j)->setPathStatistic(i, per_fiber_stat[j]);

    delete [] per_fiber_stat;
  }

  //Read per point stat
  for(int i = 0; i < numPathStatistics; i++)
  {
    double *per_point_stat = new double[total_pts];
    if (db->getPathStatisticHeader(i)->_is_computed_per_point)
      theStream.read((char*)per_point_stat, sizeof(double) * total_pts);

    for(int j = 0, points_read = 0; j < numPathways; j++)
      if (db->getPathStatisticHeader(i)->_is_computed_per_point)
        for (int k = 0; k < points_per_fiber[j]; k++, points_read++)
        {
          db->getPathway(j)->setPointStatistic(k, i, per_point_stat[points_read]);
          if(bPrintOut) cout << "Point Stats " << i << "," << j << ": " << per_point_stat[points_read] << "(double)" << endl;
        }

    delete [] per_point_stat;
  }
  delete []points_per_fiber;
  cerr<<endl;
}


void DTIPathwayIO::saveDatabasePDB(DTIPathwayDatabase *db, std::ostream &theStream, double ACPC[3], DTIPathwaySelection *selection)
{
  // Header size
  unsigned int headerSize;
  headerSize = 3 * sizeof(int)+ 16 * sizeof(double)
    + db->getNumPathStatistics()*sizeof(DTIPathwayStatisticHeader)
    + db->getNumAlgorithms() * sizeof(DTIPathwayAlgorithmHeader) + sizeof(int);
  writeScalar<int>  (headerSize, theStream);

  // TONY 2009.08.14
  // ADDING half voxel offset to ACPC because contrack assumes samples are at the half integers
  // and the xform in the nifti image files assume measurments are at the integers
  double vox_size[3];
  db->getVoxelSize(vox_size);
  for(int ii=0; ii<3; ii++)
    if(vox_size[ii]!=-1)
      ACPC[ii]+=0.5*vox_size[ii];

  // Xform
  for (int i = 0; i < 16; i++)
  {
    if( i%5==0 )
      writeScalar<double> (1, theStream);
    else if(i==3 || i==7 || i==11)
      writeScalar<double> (-ACPC[(int)floor(float(i)/3-1)], theStream);
    else
      writeScalar<double> (0, theStream);
  }

  // Statistics header
  writeScalar<int>  (db->getNumPathStatistics(), theStream);
  for (int i = 0; i < db->getNumPathStatistics(); i++)
    writeStatHeader (db->getPathStatisticHeader(i), theStream);

  // Algorithm info. header
  writeScalar<int>  (db->getNumAlgorithms(), theStream);
  if (db->getNumAlgorithms() == 0)
  {
    cerr << "ERROR: Missing algorithm header information in database." << endl;
    cerr << "Cannot open database for writing..."<< endl;
    return;
  }
  for (int i = 0; i < db->getNumAlgorithms(); i++)
    writeAlgoHeader (db->getPathAlgorithmHeader(i), theStream);

  // Version info
  writeScalar<int> (3, theStream);
  int total_pathways = db->getNumFibers();

  int numPathways;
  if (selection)
    numPathways = selection->getNumSelectedPathways();
  else
    numPathways = db->getNumFibers();
  assert(numPathways<=db->getNumFibers());
  writeScalar<int>  (numPathways, theStream);


  // Save points per fiber
  for(int i = 0; i < total_pathways; i++)
    if( selection==NULL || (*selection)[i] )
      writeScalar<int>( db->getPathway(i)->getNumPoints(), theStream);

  //Save the fiber points to buffer
  for(int i = 0; i < total_pathways; i++)
    if( selection==NULL || (*selection)[i] )
      for(int j = 0; j < db->getPathway(i)->getNumPoints(); j++)
      {
        DTIGeometryVector *vec = db->getPathway(i)->_point_vector[j];
        writeScalar<double>((*vec)[0], theStream);
        writeScalar<double>((*vec)[1], theStream);
        writeScalar<double>((*vec)[2], theStream);
      }

  //Write the per fiber stats
  for(int i = 0; i < db->getNumPathStatistics(); i++)
    for(int j = 0; j < total_pathways; j++)
      if( selection==NULL || (*selection)[j] )
        writeScalar<double>(db->getPathway(j)->getPathStatistic(i), theStream);

  // Write per point stats
  for(int i = 0; i < db->getNumPathStatistics(); i++)
    if (db->getPathStatisticHeader(i)->_is_computed_per_point)
      for(int j = 0; j < total_pathways; j++)
        if( selection==NULL || (*selection)[j] )
          for(int k = 0; k < db->getPathway(j)->getNumPoints(); k++)
            writeScalar<double>(db->getPathway(j)->getPointStatistic(k,i), theStream);
}
