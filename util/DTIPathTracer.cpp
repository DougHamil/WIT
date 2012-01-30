/** Copyright (c) 2003, The Board of Trustees of Leland Stanford Junior 
University. All rights reserved. **/

#include "DTIPathTracer.h"
#include <util/ScalarVolumeInterface.h>
#include <util/DTIVolumeInterface.h>
#include <util/DTIPathwayInterface.h>
#include <util/DTIPathwayFactoryInterface.h>
#include "DTISeedGenerator.h"
#include <iostream>
#include <jama/jama_eig.h>
#include <tnt/tnt_array1d_utils.h>
#include <tnt/tnt_array2d_utils.h>
#include <util/typedefs.h>

DTIPathTracer::DTIPathTracer (int algoID, DTISeedGenerator *seedGenerator, DTIPathwayFactoryInterface *pathwayFactory, DTIVolumeInterface *volume, double stepSizeMm) 

{

  _seed_generator = seedGenerator;
  _algo_id = algoID;
  _pathway_factory = pathwayFactory;
  _volume = volume;
  _step_size_mm = stepSizeMm;

  double voxSizeBuf[3];
  DTIVector voxSize (3, voxSizeBuf);
  _volume->getVoxelSize (voxSize[0], voxSize[1], voxSize[2]);
  
  _step_size_voxel = new DTIVector(3);
  (*_step_size_voxel)[0] = _step_size_mm / voxSize[0];
  (*_step_size_voxel)[1] = _step_size_mm / voxSize[1];
  (*_step_size_voxel)[2] = _step_size_mm / voxSize[2];
  
  initialize();

}


/*************************************************************************
 * Function Name: DTIPathTracer::getMajorEigenvector
 * Parameters: const DTITensor &tensor
 * Returns: DTIVector
 * Effects: 
 *************************************************************************/
DTIVector
DTIPathTracer::getMajorEigenvector(const DTITensor &tensor)
{
  // use jama to compute eigenvectors, eigenvalues of tensor.
  // pick the largest magnitude eigenvalue and return corresponding 
  // eigenvector.
  TNT::Array1D<double> eigenvalues;
  //  cerr << ".";
  JAMA::Eigenvalue<double> eig(tensor);
  eig.getRealEigenvalues (eigenvalues);
  //  cerr << "/";
  int index = 0;
  double largest = fabs (eigenvalues[0]);
  for (int i = 1; i < 3; i++) {
    if (fabs(eigenvalues[i]) > largest) {
      largest = fabs(eigenvalues[i]);
      index = i;
    }
  }
 
  
  TNT::Array2D<double> eigenvectors;
  eig.getV (eigenvectors);
  DTIVector result(3);
  result[0] = eigenvectors[0][index];
  result[1] = eigenvectors[1][index];
  result[2] = eigenvectors[2][index];

  return result;
}


/*************************************************************************
 * Function Name: DTIPathTracer::inBounds
 * Parameters: const DTIVector &pt, DTIVolume *volume
 * Returns: bool
 * Effects: 
 *************************************************************************/
bool
DTIPathTracer::inBounds(const DTIVector &pt, DTIVolumeInterface *volume)
{
  unsigned int x,y,z;
  volume->getDimension(x,y,z);
  if (pt[0] < 0 || pt[0] > x-1 ||
      pt[1] < 0 || pt[1] > y-1 ||
      pt[2] < 0 || pt[2] > z-1)
  return false;
  else 
    return true;
}


/*************************************************************************
 * Function Name: DTIPathTracer::initialize
 * Parameters:
 * Returns: 
 * Effects: 
 *************************************************************************/
void
DTIPathTracer::initialize()
{
  if (_seed_generator) 
    _seed_generator->initialize();
}
