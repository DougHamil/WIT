#include "extended_fslio.h"

#define FSLIOERR(x) { fprintf(stderr,"Error:: %s\n",(x)); fflush(stderr); exit(EXIT_FAILURE); }

/***************************************************************
 * d5matrix
 ***************************************************************/
double *****d5matrix(int uh, int th, int zh,  int yh, int xh)
/* allocate a double 5matrix with range t[0..uh][0..th][0..zh][0..yh][0..xh] */
/* adaptation of Numerical Recipes in C nrutil.c allocation routines */
{

	int j;
	int nvol = uh+1;
	int nt = th+1;
	int nslice = zh+1;
	int nrow = yh+1;
	int ncol = xh+1;
        double *****t;


	/** allocate pointers to vols */
        t=(double *****) malloc((size_t)((nvol)*sizeof(double****)));
        if (!t) FSLIOERR("d5matrix: allocation failure");

	/** allocate pointers to times */
	t[0]=(double ****) malloc ((size_t)((nvol*nt)*sizeof (double***)));
	if (!t[0]) FSLIOERR("d5matrix: allocation failure");

	/** allocate pointers to slices */
        t[0][0]=(double ***) malloc((size_t)((nvol*nt*nslice)*sizeof(double**)));
        if (!t[0][0]) FSLIOERR("d5matrix: allocation failure");

	/** allocate pointers for ydim */
        t[0][0][0]=(double **) malloc((size_t)((nvol*nt*nslice*nrow)*sizeof(double*)));
        if (!t[0][0][0]) FSLIOERR("d5matrix: allocation failure");


	/** allocate the data blob */
        t[0][0][0][0]=(double *) malloc((size_t)((nvol*nt*nslice*nrow*ncol)*sizeof(double)));
        if (!t[0][0][0][0]) FSLIOERR("d5matrix: allocation failure");


	/** point everything to the data blob */
        for(j=1;j<nrow*nslice*nt*nvol;j++) t[0][0][0][j]=t[0][0][0][j-1]+ncol;
        for(j=1;j<nslice*nt*nvol;j++) t[0][0][j]=t[0][0][j-1]+nrow;
	for(j=1;j<nt*nvol;j++) t[0][j]=t[0][j-1]+nslice;
        for(j=1;j<nvol;j++) t[j]=t[j-1]+nt;

        return t;
}

/***************************************************************
 * FslGetBufferAsScaledDouble_5D
 ***************************************************************/
/*! \fn double **** FslGetBufferAsScaledDouble(FSLIO *fslio)
    \brief Return the fslio data buffer of a 1-4D dataset as a 4D array of 
        scaled doubles. This function is modified from the original 4D version
	(FslGetBufferAsScaledDouble) in fslio.c

        Array is indexed as buf[0..sdim-1][0..tdim-1][0..zdim-1][0..ydim-1][0..xdim-1].  
        <br>The array will be byteswapped to native-endian.
        <br>Array values are scaled as per fslio header slope and intercept fields.

    \param fslio pointer to open dataset
    \return Pointer to 5D double array, NULL on error
 */

double *****FslGetBufferAsScaledDouble_5D(FSLIO *fslio)
{
  double *****newbuf;
  int xx,yy,zz,tt,uu;
  int ret;
  float inter, slope;

  if (fslio==NULL)  FSLIOERR("FslGetBufferAsScaledDouble: Null pointer passed for FSLIO");

  if ((fslio->niftiptr->dim[0] <= 0) || (fslio->niftiptr->dim[0] > 5))
	FSLIOERR("FslGetBufferAsScaledDouble: Incorrect dataset dimension, 1-5D needed");

  /***** nifti dataset */
  if (fslio->niftiptr!=NULL) {
	xx = (fslio->niftiptr->nx == 0 ? 1 : (long)fslio->niftiptr->nx);
	yy = (fslio->niftiptr->ny == 0 ? 1 : (long)fslio->niftiptr->ny);
	zz = (fslio->niftiptr->nz == 0 ? 1 : (long)fslio->niftiptr->nz);
	tt = (fslio->niftiptr->nt == 0 ? 1 : (long)fslio->niftiptr->nt);
	uu = (fslio->niftiptr->nu == 0 ? 1 : (long)fslio->niftiptr->nu);

	if (fslio->niftiptr->scl_slope == 0) {
		slope = 1.0;
		inter = 0.0;
	}
	else {
		slope = fslio->niftiptr->scl_slope;
		inter = fslio->niftiptr->scl_inter;
	}
	
    /** allocate new 5D buffer */
    newbuf = d5matrix(uu-1,tt-1,zz-1,yy-1,xx-1);

    /** cvt it */
    ret = convertBufferToScaledDouble(newbuf[0][0][0][0], fslio->niftiptr->data, (long)(xx*yy*zz*tt*uu), slope, inter, fslio->niftiptr->datatype);

    if (ret == 0)
        return(newbuf);
    else
        return(NULL);

  } /* nifti data */

  if (fslio->mincptr!=NULL) {
    fprintf(stderr,"Warning:: Minc is not yet supported\n");
  }

  return(NULL);
}

