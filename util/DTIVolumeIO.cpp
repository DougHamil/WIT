/** Copyright (c) 2005, The Board of Trustees of Leland Stanford Junior 
University. All rights reserved. **/

/*************************************************************************
 *    NAME: David Akers and Anthony Sherbondy
 *     
 *    FILE: DTIVolumeIO.cpp
 *    DATE: Mon Feb 23  6:07:30 PST 2004
 *************************************************************************/


#include "DTIVolumeIO.h"
#include "DTIVolume.h"
#include "ScalarVolume.h"
#include <jama/jama_eig.h>
#include <tnt/tnt_array1d_utils.h>
//using namespace std;
using namespace TNT;

// For NIFTI
#include <nifti/extended_fslio.h>

int readInteger (FILE *datafile) {
  int i;
  fread ((char *) &i, sizeof(int), 1, datafile);
  return i;
}

double readDouble (FILE *datafile) {
  double d;
  fread ((char *) &d, sizeof(double), 1, datafile);
  return d;
}


extern double readDouble (std::ifstream &stream);

extern void writeDouble (double d, std::ofstream &stream);
extern void writeInt (int i, std::ofstream &stream);
extern int readInt (std::ifstream &stream);


/*************************************************************************
 * Function Name: DTIVolumeIO::loadVolume
 * Parameters: const char *filename
 * Returns: DTIVolume *s
 * Effects: 
 *************************************************************************/
DTIVolume *
DTIVolumeIO::loadVolume(const char *filename)
{
  
  DTIVolume *vol = new DTIVolume();

  FILE *datafile = fopen (filename, "rb");

  vol->_x_size = readDouble (datafile);
  vol->_y_size = readDouble (datafile);
  vol->_z_size = readDouble (datafile);

  int xdim = readInteger (datafile);
  int ydim = readInteger (datafile);
  int zdim = readInteger (datafile);

  double *dataPtr = new double[xdim*ydim*zdim*6];
  
  fread (dataPtr, sizeof(double), xdim*ydim*zdim*6, datafile);

  // reorganize data so that tensor components are adjacent in memory.

   // Dave's original data loading code
//     double *dataFinal = new double[xdim*ydim*zdim*6];
//     for (int x = 0; x < xdim; x++) {
//   	  for (int y = 0; y < ydim; y++) {
//   		  for (int z = 0; z < zdim; z++) {
//   			  for (int tens = 0; tens < 6; tens++) {
//   				  int oldIndex = tens*zdim*ydim*xdim + z*ydim*xdim + y*xdim + x;
//   				  dataFinal[x*ydim*zdim*6 + y*zdim*6 + z*6 + tens] = 
//   					  dataPtr[oldIndex];
//   			  }
//   		  }
//   	  }
//     }

  // try to read in NIMBL data.
    double *dataFinal = new double[xdim*ydim*zdim*6];
    for (int x = 0; x < xdim; x++) {
  	  for (int y = 0; y < ydim; y++) {
  		  for (int z = 0; z < zdim; z++) {
  			  for (int tens = 0; tens < 6; tens++) {
			    // this is kind of weird -- the order
			    // is Z,tensorComponent,Y,X
  				  int oldIndex = z*6*ydim*xdim + tens*ydim*xdim + y*xdim + x;
  				  dataFinal[x*ydim*zdim*6 + y*zdim*6 + (z % zdim)*6 + tens] = 
  					  dataPtr[oldIndex];
  			  }
  		  }
  	  }
    }
  
  delete[] dataPtr;

  // build a linear array of DTITensors, and feed it to the 3D tensor array.
  
  double *curPtr = dataFinal;


  DTITensor *tensors = new DTITensor[xdim*ydim*zdim];
 
  vol->_tensor_array = new DTITensorArray(xdim,ydim,zdim,tensors);

  for (int i = 0; i < xdim; i++) {
    for (int j = 0; j < ydim; j++) {
      for (int k = 0; k < zdim; k++) {
	double *tens = new double[9];
	(*(vol->_tensor_array))[i][j][k] = DTITensor (3, 3, tens);
      
	(*(vol->_tensor_array))[i][j][k][0][0] = curPtr[0];
	(*(vol->_tensor_array))[i][j][k][1][1] = curPtr[1];
	(*(vol->_tensor_array))[i][j][k][2][2] = curPtr[2];
	(*(vol->_tensor_array))[i][j][k][0][1] = curPtr[3];
	(*(vol->_tensor_array))[i][j][k][1][0] = curPtr[3];
	(*(vol->_tensor_array))[i][j][k][0][2] = curPtr[4];
	(*(vol->_tensor_array))[i][j][k][2][0] = curPtr[4];
	(*(vol->_tensor_array))[i][j][k][1][2] = curPtr[5];
	(*(vol->_tensor_array))[i][j][k][2][1] = curPtr[5];
	curPtr += 6;
      }
    }
  }
  delete[] dataFinal;

  return vol;
}

/***********************************************************************
 *  Method: DTIVolumeIO::isValidBackground
 *  Params: const char *baseFilename
 * Returns: DTIVolume *
 * Effects: 
 ***********************************************************************/
bool 
DTIVolumeIO::isValidBackground(const char *filename) 
{
  // to be a valid background image, it must be one-dimensional...
  // (for now)
  FSLIO *fslio;
  void *buffer;
  // Have to do this because fsl function doesn't take const char*???
  char* fname = new char[1024];
  strcpy(fname,filename);
  
  // open nifti dataset 
  FslSetOverrideOutputType(FSL_TYPE_NIFTI_GZ);
  fslio = FslInit();
  buffer = FslReadAllVolumes(fslio,fname);
  if (buffer == NULL) {
    //fprintf(stderr, "\nError opening and reading %s.\n",fname);
    return false;
  }
  return fslio->niftiptr->ndim == 1;
  
}


/*************************************************************************
 * Function Name: DTIVolumeIO::loadVolumeNifti
 * Parameters: const char *filename
 * Returns: DTIVolume *s
 * Effects: 
 *************************************************************************/
DTIVolume *
DTIVolumeIO::loadVolumeNifti(const char *filename, const int tensorOrder[6])
{
  int tensorIndex[6] = {0, 0, 0, 0, 0, 0};
  for (int index = 0; index < 6; index++) {
    for (int orderIndex = 0; orderIndex < 6; orderIndex++) {
      if (tensorOrder[orderIndex] == index) {
	tensorIndex[index] = orderIndex;
      }
    }
  }
  //  cerr << "loading volume - " << filename << endl;
  DTIVolume *vol = new DTIVolume();

  // XXX: Not sure about data creation/deletion who has control?

  double *****ddata;
  FSLIO *fslio;
  void *buffer;
  // Have to do this because fsl function doesn't take const char*???
  char* fname = new char[1024];
  strcpy(fname,filename);
  
  // open nifti dataset 
  FslSetOverrideOutputType(FSL_TYPE_NIFTI_GZ);
  fslio = FslInit();
  buffer = FslReadAllVolumes(fslio,fname);
  if (buffer == NULL) {
    //fprintf(stderr, "\nError opening and reading %s.\n",fname);
    return NULL;
  }
  
  int xdim = fslio->niftiptr->nx;
  int ydim = fslio->niftiptr->ny;
  int zdim = fslio->niftiptr->nz;
  vol->_x_size = fslio->niftiptr->dx;
  vol->_y_size = fslio->niftiptr->dy;
  vol->_z_size = fslio->niftiptr->dz;

  //offset[0] = (fslio->niftiptr->sto_xyz).m[0][3] / (fslio->niftiptr->sto_xyz).m[0][0];
  //  offset[1] = (fslio->niftiptr->sto_xyz).m[1][3] / (fslio->niftiptr->sto_xyz).m[1][1];
  //  offset[2] = (fslio->niftiptr->sto_xyz).m[2][3] / (fslio->niftiptr->sto_xyz).m[2][2];
  
  // get data as doubles and scaled 
  ddata = FslGetBufferAsScaledDouble_5D(fslio);
  
  // build a linear array of DTITensors, and feed it to the 3D tensor array.  
  DTITensor *tensors = new DTITensor[xdim*ydim*zdim]; 
  vol->_tensor_array = new DTITensorArray(xdim,ydim,zdim,tensors);
  if (fslio->niftiptr->dim[0] == 4) {
    cerr << "Warning: Loading non-standard 4D dataset - assuming fourth dimension indexes the tensor component." << endl;
  }
  for (int i = 0; i < xdim; i++) {
    for (int j = 0; j < ydim; j++) {
      for (int k = 0; k < zdim; k++) {
	double *tens = new double[9];
	(*(vol->_tensor_array))[i][j][k] = DTITensor (3, 3, tens);
	if (fslio->niftiptr->dim[0] == 5) {
	  // standard 5D volumes, fifth dimension is the tensor component.
	  // (4th dimension is TIME!)
	  //      Dxx, Dxy, Dyy, Dxz, Dyz, Dzz (lower triangular - see NIFTI docs)
	  (*(vol->_tensor_array))[i][j][k][0][0] = ddata[tensorIndex[0]][0][k][j][i];
	  (*(vol->_tensor_array))[i][j][k][0][1] = ddata[tensorIndex[1]][0][k][j][i];
	  (*(vol->_tensor_array))[i][j][k][1][0] = ddata[tensorIndex[1]][0][k][j][i];
	  (*(vol->_tensor_array))[i][j][k][1][1] = ddata[tensorIndex[2]][0][k][j][i];
	  (*(vol->_tensor_array))[i][j][k][0][2] = ddata[tensorIndex[3]][0][k][j][i];
	  (*(vol->_tensor_array))[i][j][k][2][0] = ddata[tensorIndex[3]][0][k][j][i];
	  (*(vol->_tensor_array))[i][j][k][1][2] = ddata[tensorIndex[4]][0][k][j][i];
	  (*(vol->_tensor_array))[i][j][k][2][1] = ddata[tensorIndex[4]][0][k][j][i];
	  (*(vol->_tensor_array))[i][j][k][2][2] = ddata[tensorIndex[5]][0][k][j][i];
	}
	else if (fslio->niftiptr->dim[0] == 4) {
	  // support for [non-standard] 4D NIFTI volumes, where dimension 4
	  // indexes the tensor components:
	  (*(vol->_tensor_array))[i][j][k][0][0] = ddata[0][tensorIndex[0]][k][j][i];
	  (*(vol->_tensor_array))[i][j][k][0][1] = ddata[0][tensorIndex[1]][k][j][i];
	  (*(vol->_tensor_array))[i][j][k][1][0] = ddata[0][tensorIndex[1]][k][j][i];
	  (*(vol->_tensor_array))[i][j][k][1][1] = ddata[0][tensorIndex[2]][k][j][i];
	  (*(vol->_tensor_array))[i][j][k][0][2] = ddata[0][tensorIndex[3]][k][j][i];
	  (*(vol->_tensor_array))[i][j][k][2][0] = ddata[0][tensorIndex[3]][k][j][i];
	  (*(vol->_tensor_array))[i][j][k][1][2] = ddata[0][tensorIndex[4]][k][j][i];
	  (*(vol->_tensor_array))[i][j][k][2][1] = ddata[0][tensorIndex[4]][k][j][i];
	  (*(vol->_tensor_array))[i][j][k][2][2] = ddata[0][tensorIndex[5]][k][j][i];
	}
      }
    }
  }

  vol->_qform_code = (fslio->niftiptr->qform_code);
  vol->_sform_code = (fslio->niftiptr->sform_code);

#if 0
  // fix matrix to work on zero-indexed inputs instead of 1-indexed:
  adjustMatrices ((fslio->niftiptr->qto_xyz).m,
		  (fslio->niftiptr->qto_ijk).m);
#endif
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      vol->_qtoxyz[i][j] = (fslio->niftiptr->qto_xyz).m[i][j];
      vol->_qtoijk[i][j] = (fslio->niftiptr->qto_ijk).m[i][j];
      vol->_stoxyz[i][j] = (fslio->niftiptr->sto_xyz).m[i][j];
      vol->_stoijk[i][j] = (fslio->niftiptr->sto_ijk).m[i][j];
    }
  }

  //cleanup
  delete fname;

  return vol;
}


/*************************************************************************
 * Function Name: DTIVolumeIO::saveFA
 * Parameters: DTIFAArray *scalarVolume, std::ofstream &myStream
 * Returns: void
 * Effects: 
 *************************************************************************/
void
DTIVolumeIO::saveScalarVolume(ScalarVolume *scalarVolume, std::ofstream &myStream)
{
  unsigned int xdim,ydim,zdim,cdim;
  scalarVolume->getDimension(xdim,ydim,zdim,cdim);

  double xSize, ySize, zSize;
  scalarVolume->getVoxelSize(xSize,ySize,zSize);

  //DTIFAArray *faArray = scalarVolume->_fa_array;

  writeDouble (xSize, myStream);
  writeDouble (ySize, myStream);
  writeDouble (zSize, myStream);

  writeInt (xdim, myStream);
  writeInt (ydim, myStream);
  writeInt (zdim, myStream);

  long counter=0;
  double *data = new double[xdim*ydim*zdim];
 
  for (int x = 0; x < xdim; x++) {
    for (int y = 0; y < ydim; y++) {
      for (int z = 0; z < zdim; z++) {
	for (int c = 0; c < cdim; c++) {
	  data[counter] = scalarVolume->getFA(x,y,z,c);
	  counter++;
	}
      }
    }
  }
  myStream.write ((char *) data, sizeof (double) * xdim*ydim*zdim);
  myStream.close();
}

/*************************************************************************
 * Function Name: DTIVolumeIO::saveFA
 * Parameters: DTIFAArray *scalarVolume, std::ofstream &myStream
 * Returns: void
 * Effects: 
 *************************************************************************/
void
DTIVolumeIO::saveScalarVolumeNifti(ScalarVolume *scalarVolume, const char* filename)
{
  unsigned int xdim,ydim,zdim,cdim;
  scalarVolume->getDimension(xdim,ydim,zdim,cdim);

  double xSize, ySize, zSize;
  scalarVolume->getVoxelSize(xSize,ySize,zSize);

  double cmin, cmax;
  cmin = scalarVolume->getCalcMinVal();
  cmax = scalarVolume->getCalcMaxVal();

  long counter=0;
  float *data = new float[xdim*ydim*zdim];
  float minVal = scalarVolume->getFA (0,0,0,0);
  float maxVal = minVal;
  for (int z = 0; z < zdim; z++) {
    for (int y = 0; y < ydim; y++) {
      for (int x = 0; x < xdim; x++) {
	for (int c = 0; c < cdim; c++) {
	  //data[counter] = (*faArray)[x][y][z];
	  data[counter] = scalarVolume->getFA (x,y,z,c);
	  if (data[counter] < minVal) {
	    minVal = data[counter];
	  }
	  if (data[counter] > maxVal) {
	    maxVal = data[counter];
	  }
	  counter++;
	}
      }
    }
  }
  if (cmin == 0 && cmax == 0) {
    // this indicates values were not set (according to NIFTI spec)... we'll calculate them ourselves, then...
    cmin = minVal;
    cmax = maxVal;
  }

  FSLIO* fslio;
  FslSetOverrideOutputType(FSL_TYPE_NIFTI_GZ);
  fslio = FslOpen(filename,"wb");
  //  fslio = FslXOpen(filename,"wb",FSL_TYPE_NIFTI_GZ);
  //... set the appropriate header information using FslSet calls ...
  FslSetDim(fslio, xdim, ydim, zdim, 1);
  FslSetDimensionality(fslio, 3);
  FslSetVoxDim(fslio, xSize, ySize, zSize,1);
  FslSetTimeUnits(fslio, "s");
  FslSetDataType(fslio, 16); //float
  //  FslSetIntensityScaling(fslio, slope, intercept);
  FslSetCalMinMax(fslio, cmin, cmax);
  mat44 stdmat;
  double *transformMx = scalarVolume->getTransformMatrix();

  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      stdmat.m[i][j] = transformMx[i*4+j];
      //      cerr << stdmat.m[i][j] << " ";
    }
    //    cerr << endl;
  }
  //  stdmat.m[0][0]=xSize; 
  //  stdmat.m[1][1]=ySize; 
  //  stdmat.m[2][2]=zSize; 
  
  //  stdmat.m[0][2]=stdmat.m[2][0]=stdmat.m[0][1]=stdmat.m[1][0]=stdmat.m[1][2]=stdmat.m[2][1]=0;
  
  FslSetStdXform(fslio, scalarVolume->getSformCode(), stdmat); // should copy this, not set it to 1
  FslSetRigidXform (fslio, scalarVolume->getQformCode(), stdmat); // same as above

  FslWriteHeader(fslio);
	
  //... now can write one or more volumes at a time using 
  FslWriteVolumes(fslio,data,1);
  FslClose(fslio);
}




/*************************************************************************
 * Function Name: DTIVolumeIO::loadScalarVolume
 * Parameters: std::ifstream &myStream
 * Returns: DTIFAArray *
 * Effects: 
 *************************************************************************/
ScalarVolume *
DTIVolumeIO::loadScalarVolume(std::ifstream &myStream)
{
  double xSizeMM = readDouble (myStream);
  double ySizeMM = readDouble (myStream);
  double zSizeMM = readDouble (myStream);

  int xdim = readInt (myStream);
  int ydim = readInt (myStream);
  int zdim = readInt (myStream);
  int cdim = readInt (myStream);

  double *data = new double[xdim*ydim*zdim];
  
  myStream.read ((char *) data, sizeof(double) * xdim*ydim*zdim*cdim);
  //  DTIFAArray *fa = new DTIFAArray (xdim, ydim, zdim, data);

  ScalarVolume *vol = new ScalarVolume();
  //  vol->_fa_array = fa;
  vol->_dim[0] = xdim;
  vol->_dim[1] = ydim;
  vol->_dim[2] = zdim;
  vol->_dim[3] = cdim;
  
  vol->_x_size = xSizeMM;
  vol->_y_size = ySizeMM;
  vol->_z_size = zSizeMM;
  return vol;
}

/* lifted from VTK 5.0: */

typedef double (*SquareMatPtr)[4];

void multiply4x4 (double *a, double *b, double *c) {

  SquareMatPtr aMat = (SquareMatPtr) a;
  SquareMatPtr bMat = (SquareMatPtr) b;
  SquareMatPtr cMat = (SquareMatPtr) c;
  int i, k;
  double Accum[4][4];
     
  for (i = 0; i < 4; i++)
    {
    for (k = 0; k < 4; k++)
      {
      Accum[i][k] = aMat[i][0] * bMat[0][k] +
                    aMat[i][1] * bMat[1][k] +
                    aMat[i][2] * bMat[2][k] +
                    aMat[i][3] * bMat[3][k];
      }
    }
  
  // Copy to final dest
  for (i = 0; i < 4; i++)
    {
    cMat[i][0] = Accum[i][0];
    cMat[i][1] = Accum[i][1];
    cMat[i][2] = Accum[i][2];
    cMat[i][3] = Accum[i][3];
    }
  return;  
}

void 
DTIVolumeIO::adjustMatrices(float xform[4][4], float invXform[4][4])
{
  int i,j;

  // fix for zero vs. one-indexing

  double transMx[4][4];
  
  for (i = 0; i < 4; i++) {
    for (j = 0; j < 4; j++) {
      if (i == j && i < 3) {
	transMx[i][j] = 1;
      }
      else if (j < 3) {
	transMx[i][j] = 0;
      }
      else if (i < 3) {
	transMx[i][j] = 1;
      }
      else {
	transMx[i][j] = 1;
      }
      //      std::cerr << transMx[i][j];
    }
    //    std::cerr << std::endl;
  }

  double qtoxyz[4][4];

  for (i = 0; i < 4; i++) {
    for (j = 0; j < 4; j++) {
      //qtoxyz[i][j] = (fslio->niftiptr->qto_xyz).m[i][j];
      qtoxyz[i][j] = xform[i][j];
    }
  }

  double qtoijk[4][4];

  for (i = 0; i < 4; i++) {
    for (j = 0; j < 4; j++) {
      //      qtoijk[i][j] = (fslio->niftiptr->qto_ijk).m[i][j];
      qtoijk[i][j] = invXform[i][j];
    }
  }

  multiply4x4 ((double *) transMx, (double *) qtoxyz, (double *) xform);

  transMx[0][3] = -1;
  transMx[1][3] = -1;
  transMx[2][3] = -1;
  
  multiply4x4 ((double *) qtoxyz, (double *) transMx, (double *) invXform);
}

/*************************************************************************
 * Function Name: DTIVolumeIO::loadScalarVolumeNifti
 * Parameters: std::ifstream &myStream
 * Returns: DTIFAArray *
 * Effects: 
 *************************************************************************/

ScalarVolume *
DTIVolumeIO::loadScalarVolumeNifti(const char* filename)
{
  //  cerr <<" Loading " << filename << " from disk..." << endl;

  // XXX: Not sure about data creation/deletion who has control?

  double *****ddata;
  FSLIO *fslio;
  void *buffer;
  // Have to do this because fsl function doesn't take const char*???
  char* fname = new char[1024];
  strcpy(fname,filename);
  
  // open nifti dataset 
  FslSetOverrideOutputType(FSL_TYPE_NIFTI_GZ);
  fslio = FslInit();
  buffer = FslReadAllVolumes(fslio,fname);
  if (buffer == NULL) {
    //fprintf(stderr, "\nError opening and reading %s.\n",fname);
    return NULL;
  }
  
  int xdim = fslio->niftiptr->nx;
  int ydim = fslio->niftiptr->ny;
  int zdim = fslio->niftiptr->nz;
  double xSizeMM = fslio->niftiptr->dx;
  double ySizeMM = fslio->niftiptr->dy;
  double zSizeMM = fslio->niftiptr->dz;

  double cmin = fslio->niftiptr->cal_min;
  double cmax = fslio->niftiptr->cal_max;

  //offset[0] = (fslio->niftiptr->sto_xyz).m[0][3] / (fslio->niftiptr->sto_xyz).m[0][0];
  //  offset[1] = (fslio->niftiptr->sto_xyz).m[1][3] / (fslio->niftiptr->sto_xyz).m[1][1];
  //  offset[2] = (fslio->niftiptr->sto_xyz).m[2][3] / (fslio->niftiptr->sto_xyz).m[2][2];
  
  // get data as doubles and scaled 
  ddata = FslGetBufferAsScaledDouble_5D(fslio);

  int colordim = fslio->niftiptr->ndim > 4 ? fslio->niftiptr->nu : 1;
  if (fslio->niftiptr->ndim == 4) {
    // Support for non-standard color RGB files...
    colordim = fslio->niftiptr->nt;
  }
  // Now I must flip the data because we don't do it like anybody else???
  double *data = new double[xdim*ydim*zdim*colordim];
  int counter = 0;
  float minVal = ddata[0][0][0][0][0];
  float maxVal = minVal;
  for (int z = 0; z < zdim; z++) {
    for (int y = 0; y < ydim; y++) {
      for (int x = 0; x < xdim; x++) {
        for (int c = 0; c < colordim; c++) {

          if (fslio->niftiptr->ndim >= 5) {
            data[counter] = ddata[c][0][z][y][x];
          }
          else if (fslio->niftiptr->ndim == 4) {
            data[counter] = ddata[0][c][z][y][x];
          }
          else {
            data[counter] = ddata[0][0][z][y][x];
          }
          if (data[counter] < minVal) {
            minVal = data[counter];
          }
          if (data[counter] > maxVal) {
            maxVal = data[counter];
          }
          counter++;
        }
      }
    }
  }
  
  //  DTIFAArray *fa = new DTIFAArray (xdim, ydim, zdim, colordim, data);
  ScalarVolume *vol = new ScalarVolume();
  vol->_data = data;
  //  vol->_fa_array = fa;
  vol->_x_size = xSizeMM;
  vol->_y_size = ySizeMM;
  vol->_z_size = zSizeMM;
  vol->_dim[0] = xdim;
  vol->_dim[1] = ydim;
  vol->_dim[2] = zdim;
  vol->_dim[3] = colordim;

  if (cmin == 0 && cmax == 0) {
    cmin = minVal; 
    cmax = maxVal;
  }
  vol->_calcminval = cmin;
  vol->_calcmaxval = cmax;

  vol->_qform_code = (fslio->niftiptr->qform_code);
  vol->_sform_code = (fslio->niftiptr->sform_code);

#if 0
  // fix matrix to work on zero-indexed inputs instead of 1-indexed:
  adjustMatrices ((fslio->niftiptr->qto_xyz).m,
		  (fslio->niftiptr->qto_ijk).m);
#endif
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      vol->_qtoxyz[i][j] = (fslio->niftiptr->qto_xyz).m[i][j];
      vol->_qtoijk[i][j] = (fslio->niftiptr->qto_ijk).m[i][j];
      vol->_stoxyz[i][j] = (fslio->niftiptr->sto_xyz).m[i][j];
      vol->_stoijk[i][j] = (fslio->niftiptr->sto_ijk).m[i][j];
    }
  }
  
  // cleanup
  delete fname;

  return vol;
}


/***********************************************************************
 *  Method: DTIVolumeIO::loadFSLTensorVolume
 *  Params: const char *baseFilename
 * Returns: DTIVolume *
 * Effects: 
 ***********************************************************************/
DTIVolume *
DTIVolumeIO::loadFSLTensorVolume(const char *baseFilename)
{
  char eigVec1Filename[255];
  char eigVec2Filename[255];
  char eigVec3Filename[255];
  sprintf (eigVec1Filename, "%s_V1", baseFilename);
  sprintf (eigVec2Filename, "%s_V2", baseFilename);
  sprintf (eigVec3Filename, "%s_V3", baseFilename);
  char eigVal1Filename[255];
  char eigVal2Filename[255];
  char eigVal3Filename[255];
  sprintf (eigVal1Filename, "%s_L1", baseFilename);
  sprintf (eigVal2Filename, "%s_L2", baseFilename);
  sprintf (eigVal3Filename, "%s_L3", baseFilename);

  ScalarVolume *vec[3];
  ScalarVolume *val[3];
  cerr << "Loading eigenvectors and eigenvalues..." << endl;

  vec[0] = loadScalarVolumeNifti (eigVec1Filename);
  vec[1] = loadScalarVolumeNifti (eigVec2Filename);
  vec[2] = loadScalarVolumeNifti (eigVec3Filename);
  val[0] = loadScalarVolumeNifti (eigVal1Filename);
  val[1] = loadScalarVolumeNifti (eigVal2Filename);
  val[2] = loadScalarVolumeNifti (eigVal3Filename);

  cerr << "Done!" << endl;
  DTIVolume *vol = new DTIVolume();

  unsigned int xdim,ydim,zdim,cdim;
  vec[0]->getDimension (xdim,ydim,zdim,cdim);

  double xSize, ySize, zSize;
  vec[0]->getVoxelSize (xSize, ySize, zSize);

  // build a linear array of DTITensors, and feed it to the 3D tensor array.  
  DTITensor *tensors = new DTITensor[xdim*ydim*zdim]; 
  vol->_tensor_array = new DTITensorArray(xdim,ydim,zdim,tensors);
  vol->_x_size = xSize;
  vol->_y_size = ySize;
  vol->_z_size = zSize;
  for (int z = 0; z < zdim; z++) {
    cerr << ".";
    for (int y = 0; y < ydim; y++) {
      for (int x = 0; x < xdim; x++) {
	double *tens = new double[9];
	(*(vol->_tensor_array))[x][y][z] = DTITensor (3, 3, tens);
	//  tensor(:,:,:,1,:) = eigVec(:,:,:,1,1,:).*eigVal(:,:,:,1,1,:).*eigVec(:,:,:,1,1,:) ...
//                  + eigVec(:,:,:,1,2,:).*eigVal(:,:,:,2,1,:).*eigVec(:,:,:,1,2,:) ...
//                  + eigVec(:,:,:,1,3,:).*eigVal(:,:,:,3,1,:).*eigVec(:,:,:,1,3,:);
	//(*(vol->_tensor_array))[i][j][k][0][0] = vec[0]->getFA (i,j,k,0) * val[0]->getFA (i,j,k,0) * vec[0]->getFA (i,j,k,0
	Matrix<double> tempVec(3,3);
	Matrix<double> tempVal(3,3);
	for (int tempx = 0; tempx < 3; tempx++) {
	  for (int tempy = 0; tempy < 3; tempy++) {
	    tempVal[tempx][tempy] = 0.0;
	  }
	}
	tempVal[0][0] = val[0]->getFA(x,y,z,0);
	tempVal[1][1] = val[1]->getFA(x,y,z,0);
	tempVal[2][2] = val[2]->getFA(x,y,z,0);
	for (int k = 0; k < 3; k++) {
	  tempVec[k][0] = vec[k]->getFA(x,y,z,0);
	  tempVec[k][1] = vec[k]->getFA(x,y,z,1);
	  tempVec[k][2] = vec[k]->getFA(x,y,z,2);
	}
	Matrix<double> result = matmult (matmult (transpose(tempVec), tempVal),tempVec);
	for (int xx = 0; xx < 3; xx++) {
	  for (int yy = 0; yy < 3; yy++) {
	    (*(vol->_tensor_array))[x][y][z][xx][yy] = result[xx][yy];
	  }
	}
	TNT::Array1D<double> eigenvalues;
	JAMA::Eigenvalue<double> eig((*(vol->_tensor_array))[x][y][z]);
	eig.getRealEigenvalues (eigenvalues); 
	if (eigenvalues[0] != 0.0) {
	  //  cerr << tempVal[0][0] << ", " << eigenvalues[0] << endl;
//  	  cerr << tempVal[1][1] << ", " << eigenvalues[1] << endl;
//  	  cerr << tempVal[2][2] << ", " << eigenvalues[2] << endl;
//  	  cerr << endl;
	
	  TNT::Array2D<double> eigenVectors;
	  eig.getV (eigenVectors);
	 //   cerr << eigenVectors[0][0] << " " << eigenVectors[1][0] << " " << eigenVectors[2][0];
//  	  cerr << "   " << tempVec[0][0] << " " << tempVec[0][1] << " " << tempVec[0][2] << endl;
//  	   cerr << eigenVectors[0][1] << " " << eigenVectors[1][1] << " " << eigenVectors[2][1];
//  	   cerr << "   " << tempVec[1][0] << " " << tempVec[1][1] << " " << tempVec[1][2] << endl;
//  	    cerr << eigenVectors[0][2] << " " << eigenVectors[1][2] << " " << eigenVectors[2][2];
//  	    cerr << "   " << tempVec[2][0] << " " << tempVec[2][1] << " " << tempVec[2][2] << endl;
	}
      }
    }
  }
//    (*(vol->_tensor_array))[i][j][k][0][0] = ddata[0][k][j][i];
//  	(*(vol->_tensor_array))[i][j][k][1][1] = ddata[1][k][j][i];
//  	(*(vol->_tensor_array))[i][j][k][2][2] = ddata[2][k][j][i];
//  	(*(vol->_tensor_array))[i][j][k][0][1] = ddata[3][k][j][i];
//  	(*(vol->_tensor_array))[i][j][k][1][0] = ddata[3][k][j][i];
//  	(*(vol->_tensor_array))[i][j][k][0][2] = ddata[4][k][j][i];
//  	(*(vol->_tensor_array))[i][j][k][2][0] = ddata[4][k][j][i];
//  	(*(vol->_tensor_array))[i][j][k][1][2] = ddata[5][k][j][i];
//  	(*(vol->_tensor_array))[i][j][k][2][1] = ddata[5][k][j][i];

  return vol;
}


