/* 
 * CCDMOM: take a moment along an axis in an image
 *
 *	quick and dirty:  8-jun-95		pjt
 *      19-oct-95   very dirty axis=3           pjt
 *	12-dec-98   0.3  more implementations   pjt
 *      31-dec-98   0.4  keep reduced axis, and replace it with new value PJT
 *      20-feb-01   0.4a implemented special mom=3 for a 3point poly fit to peak  PJT
 *                       also fixed ccdmom 
 *      25-mar-01   0.5  added mom=-1 as mean value along an axis, fixed bugs
 *      18-oct-05   0.6  added mom=-2 as dispersion around mean along an axis
 * ??   16-sep-11   ???  added clip= and  rngmsk= ???   code lost ???
 *      19-jul-12   0.7  allow peak (mom=3) to find 2nd peak
 *                      
 * TODO : cumulative along an axis, sort of like numarray.accumulate()
 *        man page talks about clip= and  rngmsk=, where is this code?
 */


#include <stdinc.h>
#include <getparam.h>
#include <vectmath.h>
#include <filestruct.h>
#include <image.h>

string defv[] = {
        "in=???\n       Input image file",
	"out=???\n      Output image file",
	"axis=1\n       Axis to take moment along (1=x 2=y 3=z)",
	"mom=0\n	Moment to take [0=sum,1=mean loc,2=disp loc,3=peak loc, -1=mean val, -2=disp val]",
	"keep=f\n	Keep moment axis in full length, and replace all values",
	"cumulative=f\n Cumulative axis (only valid for mom=0)",
	"peak=1\n       Find N-th peak in case of peak finding (mom=3)",
	"VERSION=0.7\n  19-jul-2012 PJT",
	NULL,
};

string usage = "moment along an axis of an image";
string cvsid="$Id$";

local real peak_axis(imageptr iptr, int i, int j, int k, int axis);
local int  peak_find(int n, real *data, int *mask, int npeak);
local bool out_of_range(real);



void nemo_main()
{
    stream  instr, outstr;
    int     nx, ny, nz, nx1, ny1, nz1;
    int     axis, mom;
    int     i,j,k, apeak, apeak1, cnt;
    imageptr iptr=NULL, iptr1=NULL, iptr2=NULL;      /* pointer to images */
    real    tmp0, tmp1, tmp2, tmp00, newvalue, peakvalue, scale, offset;
    real    *spec;
    int     *smask;
    bool    Qkeep = getbparam("keep");
    int     npeak = getiparam("peak");

    instr = stropen(getparam("in"), "r");
    mom = getiparam("mom");
    if (mom < -2 || mom > 3)  error("Illegal value mom=%d",mom);
    axis = getiparam("axis");
    if (axis < 0 || axis > 3) error("Illegal value axis=%d",axis);

    if (mom==3 && axis!=3 && npeak>1) error("Nth-peak>1 finding only axis=3");

    if (getbparam("cumulative"))
      axis = -axis;

    read_image( instr, &iptr);

    nx1 = nx = Nx(iptr);	
    ny1 = ny = Ny(iptr);
    nz1 = nz = Nz(iptr);
    if (Qkeep) {
        dprintf(0,"Keeping %d*%d*%d cube\n",nx1,ny1,nz1);
	spec = smask = NULL;
    } else {
        if (axis==1) {
            nx1 = 1;    ny1 = ny;   nz1 = nz;
	    spec = (real *) allocate(nx*sizeof(real));
	    smask = (int *) allocate(nx*sizeof(int));
        } else if (axis==2) {
            nx1 = nx;   ny1 = 1;    nz1 = nz;
	    spec = (real *) allocate(ny*sizeof(real));
	    smask = (int *) allocate(ny*sizeof(int));
        } else if (axis==3) {
            nx1 = nx;   ny1 = ny;   nz1 = 1;
	    spec = (real *) allocate(nz*sizeof(real));
	    smask = (int *) allocate(nz*sizeof(int));
        } else if (axis < 0) {
	    nx1 = nx;   ny1 = ny;   nz1 = nz;
	    spec = smask = NULL;
	} else
            error("Invalid axis: %d (Valid: 1,2,3)",axis);
        dprintf(0,"Reducing %d*%d*%d to a %d*%d*%d cube\n",
                   nx,ny,nz, nx1,ny1,nz1);
    }

    outstr = stropen(getparam("out"), "w");

    if (axis > 0) {
      create_cube(&iptr1,nx1,ny1,nz1);
      create_cube(&iptr2,nx1,ny1,nz1);
    } else {
      copy_image(iptr,&iptr1);
    }

    if (axis==1) {
      scale = Dx(iptr);
      offset = Xmin(iptr);
      for (k=0; k<nz; k++)
        for (j=0; j<ny; j++) {
	    tmp0 = tmp00 = tmp1 = tmp2 = 0.0;
	    cnt = 0;
	    peakvalue = CubeValue(iptr,0,j,k);
            for (i=0; i<nx; i++) {
	        spec[i] = CubeValue(iptr,i,j,k);
 	        if (out_of_range(CubeValue(iptr,i,j,k))) continue;
		cnt++;
    	        tmp0 += CubeValue(iptr,i,j,k);
		tmp00 += sqr(CubeValue(iptr,i,j,k));
    	        tmp1 += i*CubeValue(iptr,i,j,k);
    	        tmp2 += i*i*CubeValue(iptr,i,j,k);
		if (CubeValue(iptr,i,j,k) > peakvalue) {
		  apeak = i;
		  peakvalue = CubeValue(iptr,i,j,k);
		}
            }
	    if (cnt==0 || tmp0==0.0) {
	      newvalue = 0.0;
	    } else {
	      if (mom==-1)
		newvalue = tmp0/cnt;
	      else if (mom==-2)
		newvalue = sqrt(tmp00/cnt - sqr(tmp0/cnt));
	      else if (mom==0) 
		newvalue = tmp0;
	      else if (mom==1)
		newvalue = offset + scale*(tmp1/tmp0);
	      else if (mom==2)
		newvalue = scale*sqrt(tmp2/tmp0 - sqr(tmp1/tmp0));
	      else if (mom==3)
		newvalue = scale*(apeak + peak_axis(iptr,apeak,j,k,axis)) + offset;
	    }
	    for (i=0; i<nx1; i++)
	      CubeValue(iptr1,i,j,k) = newvalue;
        }

        /* TODO: fix up Qkeep headers */

        Xmin(iptr1) = Xmin(iptr) + 0.5*(nx-1)*Dx(iptr);
        Ymin(iptr1) = Ymin(iptr);
        Zmin(iptr1) = Zmin(iptr);
        Dx(iptr1) = nx * Dx(iptr);
        Dy(iptr1) = Dy(iptr);
        Dz(iptr1) = Dz(iptr);
        
        Namex(iptr1) = Namex(iptr); /* care: we're passing a pointer */
        Namey(iptr1) = Namey(iptr);
        Namez(iptr1) = Namez(iptr);

    } else if (axis==2) {                      
      scale = Dy(iptr);
      offset = Ymin(iptr);
      for (k=0; k<nz; k++)
        for (i=0; i<nx; i++) {
            tmp0 = tmp00 = tmp1 = tmp2 = 0.0;
	    cnt = 0;
	    peakvalue = CubeValue(iptr,i,0,k);
            for (j=0; j<ny; j++) {
	        spec[j] = CubeValue(iptr,i,j,k); 
 	        if (out_of_range(CubeValue(iptr,i,j,k))) continue;
		cnt++;
    	        tmp0 += CubeValue(iptr,i,j,k);
		tmp00 += sqr(CubeValue(iptr,i,j,k));
    	        tmp1 += j*CubeValue(iptr,i,j,k);
    	        tmp2 += j*j*CubeValue(iptr,i,j,k);
		if (CubeValue(iptr,i,j,k) > peakvalue) {
		  apeak = j;
		  peakvalue = CubeValue(iptr,i,j,k);
		}
            }
	    if (cnt==0 || tmp0==0.0) {
	      newvalue = 0.0;
	    } else {
	      if (mom==-1)
		newvalue = tmp0/cnt;
	      else if (mom==-2)
		newvalue = sqrt(tmp00/cnt - sqr(tmp0/cnt));
	      else if (mom==0)
		newvalue = tmp0;
	      else if (mom==1)
		newvalue = scale*(tmp1/tmp0) + offset;
	      else if (mom==2)
		newvalue = scale*sqrt(tmp2/tmp0 - sqr(tmp1/tmp0));
	      else if (mom==3)
		newvalue = scale*(apeak + peak_axis(iptr,i,apeak,k,axis)) + offset;
	    }
            for (j=0; j<ny1; j++)
                CubeValue(iptr1,i,j,k) = newvalue;
        }

        /* TODO: */

        Xmin(iptr1) = Xmin(iptr);
        Ymin(iptr1) = Ymin(iptr) + 0.5*(ny-1)*Dy(iptr);
        Zmin(iptr1) = Zmin(iptr);
        Dx(iptr1) = Dx(iptr);
        Dy(iptr1) = ny * Dy(iptr);
        Dz(iptr1) = Dz(iptr);
        
        Namex(iptr1) = Namex(iptr); /* care: we're passing a pointer */
        Namey(iptr1) = Namey(iptr);
        Namez(iptr1) = Namez(iptr);

    } else if (axis==3) {                       /* this one is tested */
        scale = Dz(iptr);
	offset = Zmin(iptr);
    	for(j=0; j<ny; j++)
    	for(i=0; i<nx; i++) {
    	    tmp0 = tmp00 = tmp1 = tmp2 = 0.0;
	    cnt = 0;
	    peakvalue = CubeValue(iptr,i,j,0);
    	    for(k=0; k<nz; k++) {
	        spec[k] = CubeValue(iptr,i,j,k);
 	        if (out_of_range(CubeValue(iptr,i,j,k))) continue;
		cnt++;
    	        tmp0 += CubeValue(iptr,i,j,k);
		tmp00 += sqr(CubeValue(iptr,i,j,k));
    	        tmp1 += k*CubeValue(iptr,i,j,k);
    	        tmp2 += k*k*CubeValue(iptr,i,j,k);
		if (CubeValue(iptr,i,j,k) > peakvalue) {
		  apeak = k;
		  peakvalue = CubeValue(iptr,i,j,k);
		}
    	    }
	    if (cnt==0 || tmp0==0.0) {
	      newvalue = 0.0;
	    } else {
	      if (mom==-1)
		newvalue = tmp0/cnt;
	      else if (mom==-2)
		newvalue = sqrt(tmp00/cnt - sqr(tmp0/cnt));
	      else if (mom==0)
		newvalue = tmp0;
	      else if (mom==1)
		newvalue = scale*(tmp1/tmp0) + offset;
	      else if (mom==2)
		newvalue = scale*sqrt(tmp2/tmp0 - sqr(tmp1/tmp0));
	      else if (mom==3) {
		if (npeak == 0) {
		  newvalue = scale*(apeak + peak_axis(iptr,i,j,apeak,axis)) + offset;
		} else {
		  for (k=0; k<nz; k++) smask[k] = 1;
		  apeak1 = peak_find(nz, spec, smask, 1);
		  if (apeak1 > 0) {
		    if  (apeak1 != apeak && (apeak!=0 && apeak!=nz-1)) {
		      for (k=0; k<nz; k++)
			printf("%d %g  %d\n",k,spec[k],smask[k]);
		      warning("peak_find no good (%d,%d) %d %d",i,j,apeak,apeak1);
		    }
		    newvalue = scale*(apeak1 + peak_axis(iptr,i,j,apeak1,axis)) + offset;
		  } else
		    newvalue = 0.0;
		  if (npeak > 1) { 
		    for (k=2; k<npeak; k++)
		      apeak1 = peak_find(nz,spec,smask,k);
		    if (apeak1 > 0) {
		      newvalue = scale*(apeak1 + peak_axis(iptr,i,j,apeak1,axis)) + offset;
		    } else
		      newvalue = 0;
		  }
		}
	      }
	    }
            for (k=0; k<nz1; k++)
	      CubeValue(iptr1,i,j,k) = newvalue;
    	}

        /* TODO: */

        Xmin(iptr1) = Xmin(iptr);
        Ymin(iptr1) = Ymin(iptr);
        Zmin(iptr1) = Zmin(iptr) + 0.5*(nz-1)*Dz(iptr);
        Dx(iptr1) = Dx(iptr);
        Dy(iptr1) = Dy(iptr);
        Dz(iptr1) = nz * Dz(iptr);
        
        Namex(iptr1) = Namex(iptr); /* care: we're passing a pointer */
        Namey(iptr1) = Namey(iptr);
        Namez(iptr1) = Namez(iptr);
        
    } else if (axis == -1) {
      for (k=0; k<nz; k++)
        for (j=0; j<ny; j++) {
	  tmp0 = 0.0;
	  for (i=0; i<nx; i++) {
	    tmp0 += CubeValue(iptr,i,j,k);
	    CubeValue(iptr1,i,j,k) = tmp0;
	  }
        }
    } else if (axis == -2) {                      
      for (k=0; k<nz; k++)
        for (i=0; i<nx; i++) {
	  tmp0 = 0.0;
	  for (j=0; j<ny; j++) {
	    tmp0 += CubeValue(iptr,i,j,k);
	    CubeValue(iptr1,i,j,k) = tmp0;
	  }
        }
    } else if (axis == -3) {               
    	for(j=0; j<ny; j++)
	  for(i=0; i<nx; i++) {
    	    tmp0 = 0.0;
    	    for(k=0; k<nz; k++) {
	      tmp0 += CubeValue(iptr,i,j,k);
	      CubeValue(iptr1,i,j,k) = tmp0;
	    }
	  }
    } else
        error("Cannot do axis %d",axis);

    write_image(outstr, iptr1);
}


/*
 * return location of peak for (-1,y1) (0,y2) (1,y3)
 * as determined from the 2d polynomial going through
 * these 3 points
 */ 

local real peak_axis(imageptr iptr, int i, int j, int k, int axis)
{
  real y1, y2, y3;
  y2 = CubeValue(iptr,i,j,k);
  if (axis==1)
    i -= 1;
  else if (axis==2)
    j -= 1;
  else if (axis==3)
    k -= 1;
  y1 = CubeValue(iptr,i,j,k);
  if (axis==1)
    i += 2;
  else if (axis==2)
    j += 2;
  else if (axis==3)
    k += 2;
  y3 = CubeValue(iptr,i,j,k);

  if (y1+y3 == 2*y2) return 0.0;
  return 0.5*(y1-y3)/(y1+y3-2*y2);
}

/* 
 * this routine can be called multiple times
 * each time it will find a peak, and then walk down the peak
 * and set the mask to be 0, so it's not found again
 * Before the first call, the mask[] array needs to be set to all 1's.
 */


local int peak_find(int n, real *data, int *mask, int npeak)
{
  int i, ipeak=0, apeak=-1;
  real peakvalue, oldvalue;

  dprintf(1,"peak_find %d\n",n);

  while (1) {                   /* iterate until a good peak found ? */
    ipeak++;
    dprintf(1,"iter ipeak=%d npeak=%d\n",ipeak,npeak);
    if (ipeak > npeak+3) break;

    for(i=0; i<n; i++) {        /* go over all good points and find the peak */
      if (mask[i]) {            /* for all good data not yet masked out */
	if (apeak<0) {          /* make sure we initialize first good value as peak */
	  peakvalue = data[i];   
	  apeak = i; 
	  continue;
	}
	if (data[i] > peakvalue) {   /* find largest values and remember where it was */
	  peakvalue = data[i];
	  apeak = i;
	}
      }
    }
    dprintf(1,"apeak=%d\n",apeak);
    if (apeak==0) {             /* never allow first point... */
      mask[apeak] = 0;
      apeak = -1;
      for (i=1, oldvalue=peakvalue; mask[i] && i<n; i++) {   /* walk down, and mask out */
	if (data[i] > oldvalue) break;                       /* util data increase again */
	oldvalue = data[i];
	mask[i] = 0;
      }
      continue;
    }
    if (apeak==(n-1)) {         /* ...or last point, since they have no neighbors */
      mask[apeak]= 0;
      apeak = -1;
      for (i=n-2; oldvalue=peakvalue, mask[i] && i>0; i--) {  /* walk down, and mask out */
	if (data[i] > oldvalue) break;                        /* until data increase again */
	oldvalue = data[i];
	mask[i] = 0;
      }
      break;

      continue;
    }
    if (apeak > 0) {            /* done, found a peak, but mask down the peak */
      mask[apeak] = 0;
      for (i=apeak+1, oldvalue=peakvalue; mask[i]; i++) {
	if (data[i] > oldvalue) break;
	oldvalue = data[i];
	mask[i] = 0;
      }
      for (i=apeak-1; oldvalue=peakvalue, mask[i]; i--) {
	if (data[i] > oldvalue) break;
	oldvalue = data[i];
	mask[i] = 0;
      }
      break;
    }

  } /* while() */
  return apeak;
}



local bool out_of_range(real x)
{
  return FALSE;
}
