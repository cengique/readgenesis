/*
    Adapted for general use (a routine...)
    from Dieter Jaeger's xicanal lx_genread.c
    Alfonso Delgado-Reyes 07.03.2002

    Cengiz Gunay <cgunay@emory.edu> 03.13.2004
    Fixed memory leak of not deallocating memory for the raw data buffer.
    
    o Adapted for MATLAB 5.x and 6.x under:
        - Linux x86/PPC, September 2002
        - MS Windows, September 2002
        - Mac OS 7-9, September 2002
        - Mac OS X 10.x, September 2002
    
    Any cc compiler should work ([ ] = optional)
	
	To compile:
	  In Winblows (MS Visual C++ 6.x):
		mex [-v -O] -DWIN32 -output readgenesis readgenesis.c
	  Anywhere else:
		mex [-v -O] -output readgenesis readgenesis.c
          On new i64 Macs:
                /Applications/MATLAB_R2011a.app/bin/mex -v -O -I/usr/include -I/usr/lib/gcc/i686-apple-darwin10/4.2.1/include/ -I/usr/lib/gcc/powerpc-apple-darwin10/4.2.1/install-tools/include -I/Applications/MATLAB_R2011a.app/extern/include/ -output readgenesis_mac readgenesis_mac.c 
  	  To get a endian-converting loader, enable big endian with: -D__BIG_ENDIAN__
*/

#include <stdio.h>
#include <stdlib.h>
#if defined(WIN32)
#include <io.h>
#else
#include <unistd.h>
#endif
#include <string.h>
#include <fcntl.h>
#if !defined(__APPLE__)
#include <malloc.h>
#endif
#include <float.h>
#include <math.h>
#include <assert.h>

#include <mex.h>
#include <mat.h>

#define ADFBLOCK 12000
#define BUF_SIZE 1024
#define OLD_BUF_SIZE 240

/*#if __DARWIN_BYTE_ORDER == __DARWIN_LITTLE_ENDIAN
#error DLE!
#endif
*/

struct filter_type {
  double lowcut;
  double highcut;
  double notchlow;
  double notchhigh;
};

struct analog_format {
  int		type;
  int		file_type;
  int		file_idx;
  int		cross_idx;
  int		trace_no;
  char		title[OLD_BUF_SIZE];
  int		channel;
  int		color;
  int		plot_group;
  int		select;
  int		comp_sign;
  struct	filter_type filter;
  float		gain;
  int		invert;
  float		offset;
  float		factor;
  float		overlay_pos;
  float		overlay_val;
  float		xmax;
  float		xmin;
  long		no_samples;
  float		samp_frequency;
  int		filled;
  double	*fdata;
};

struct analog_format *raw = NULL;

int get_gendata();

static void wswap(ar1, ar2)
char *ar1, *ar2;
{
    /* swap the bits around for big <--> little endian */
	
	/* little --> big */
	*ar1     = *(ar2+3);
    *(ar1+1) = *(ar2+2);
    *(ar1+2) = *(ar2+1);
    *(ar1+3) = *ar2;
}

void mexFunction(int nlhs, mxArray *plhs[], 
		         int nrhs, const mxArray *prhs[])
{
  char		*filename;
  double	 trace;
  int		 buflen;
  mxArray	*data;
  double	*start_of_pr;
  
  if (nrhs != 2) {
    mexErrMsgTxt("\nUsage: data = readgenesis('filename', plotnumber)");
  }
  else if (nlhs != 1) {
    mexErrMsgTxt("\nreadgenesis has one output argument");
  }
  
  if (mxIsChar(prhs[0]) != 1) {
    mexErrMsgTxt("\nreadgenesis: first argument must be a string");
  }
  else {
    buflen = mxGetN(prhs[0])+1;
    
    filename = (char *) mxCalloc(buflen, sizeof(mxChar));
    mxGetString(prhs[0], filename, buflen); 
  }
  
  if (!mxIsDouble(prhs[1])) {
    mxFree(filename);
    mexErrMsgTxt("\nreadgenesis: argument 2 must be a noncomplex scalar");
  }
  else {
    trace = mxGetScalar(prhs[1]);
  }

  if (get_gendata(filename, (int) trace) >= 0) {
    data = mxCreateDoubleMatrix(raw->no_samples, 1, mxREAL);
    
    if (data == NULL) {
	  mxFree(filename);
  	  if (raw != NULL) free(raw);
      mexErrMsgTxt("\nreadgenesis: could not create mxArray (data)");
    }
	
    start_of_pr = (double *) mxGetPr(data);
    
    memcpy(start_of_pr, raw->fdata, raw->no_samples*1*sizeof(double));
    
    plhs[0] = data;
    
    /* Don't forget to free memory for the raw data buffer! 
       2004/03/13 Cengiz Gunay, <cgunay@emory.edu> */
    free(raw->fdata);
  }
  else {
    mexErrMsgTxt("\nreadgenesis: error... see output above");
  }
  
  mxFree(filename);
  if (raw != NULL) free(raw);
  return;
}

int get_gendata(file, plotno)
char *file;
int   plotno;
{
    int     datatype, i;
    int		noplots, headersize, pci;
    long	noitems, blockpos, readblock;
    long 	fst_pt, lst_pt, dat_points, flength;
    float   pcf, fval, ftemp[ADFBLOCK], startti, tistep, xmax_read, gain;
    char	headstr[BUF_SIZE], titlebuffer[BUF_SIZE], temp_buffer[BUF_SIZE];
    FILE	*fp;
    
    if ((fp = fopen(file, "rb")) == NULL) {
        sprintf(temp_buffer, "\nreadgenesis: could not open file '%s'\n", file);
        assert(strlen(tmp_buffer) < BUF_SIZE);
        mexErrMsgTxt(temp_buffer);
        /*fprintf(stderr, "\nreadgenesis: could not open file '%s'\n", file);
        return(-1);
        causes crashes in Winblows*/
    }
    
    if ((raw = (struct analog_format *)
               malloc(1*sizeof(struct analog_format))) == NULL)
    {
        fclose(fp);
        mexErrMsgTxt("\nreadgenesis: could not malloc data structure\n");
    }
    
    raw->gain     = 1; /* 0.001; */ 
    raw->type     = 3;
    raw->factor   = 1;
    raw->offset   = 0.0;
    raw->xmin     = 0.0;
    raw->xmax     = 100000.0;
    raw->fdata    = NULL;
    raw->trace_no = plotno;
    raw->channel  = 1;
    raw->invert   = 0;
    raw->samp_frequency = 1;
    
    gain = raw->gain;

 #if !defined(__APPLE__)      || \
     !defined(__BIG_ENDIAN__) || \
     !defined(WORDS_BIGENDIAN)
	fval = 0.0;
#endif

    fseek(fp, 0L, SEEK_SET);
    fread(headstr, sizeof(char), 80, fp);
    assert(strlen(headstr) < BUF_SIZE);
    fread(&pcf, sizeof(float), 1, fp);
    
#if (defined(__APPLE__) && (__DARWIN_BYTE_ORDER == __DARWIN_BIG_ENDIAN))      || \
    defined(__BIG_ENDIAN__) || \
    defined(WORDS_BIGENDIAN)
    wswap((char *)&startti, (char *)&pcf);
#else
	startti = pcf;
#endif
    
    fread(&pcf, sizeof(float), 1, fp);
    
#if (defined(__APPLE__) && (__DARWIN_BYTE_ORDER == __DARWIN_BIG_ENDIAN))      || \
    defined(__BIG_ENDIAN__) || \
    defined(WORDS_BIGENDIAN)
    wswap((char *)&tistep, (char *)&pcf);
#else
	tistep = pcf;
#endif

	/* sanity check for endian issue */
	assert(tistep > 1e-8 && tistep < 1);
    
    raw->samp_frequency = 0.001 / tistep;
    
    fread(&pci, sizeof(int), 1, fp);
	
#if (defined(__APPLE__) && (__DARWIN_BYTE_ORDER == __DARWIN_BIG_ENDIAN))      || \
    defined(__BIG_ENDIAN__) || \
    defined(WORDS_BIGENDIAN)
	wswap((char *)&noplots, (char *)&pci);
#else
	noplots = pci;
#endif
	
    fread(&pci, sizeof(int), 1, fp);

#if (defined(__APPLE__) && (__DARWIN_BYTE_ORDER == __DARWIN_BIG_ENDIAN))      || \
    defined(__BIG_ENDIAN__) || \
    defined(WORDS_BIGENDIAN)
	wswap((char *)&datatype, (char *)&pci);
#else
	datatype = pci;
#endif
	
    headersize = 80 + 2 * sizeof(float) +
                 2 * sizeof(int) + 3 * noplots * sizeof(float);
    
    if (plotno < 0 || plotno > noplots) {
        free(raw);
        fclose(fp);
        sprintf(temp_buffer, 
		    "\nreadgenesis: requested plot %i not available (plots = %i)\n",
                plotno, noplots);
        assert(strlen(tmp_buffer) < BUF_SIZE);
        mexErrMsgTxt(temp_buffer);
    }
    
    fseek(fp, 0L, SEEK_END);
    flength = ftell(fp);
    dat_points = (flength - headersize) / (noplots * sizeof(float));
    xmax_read = (float)(dat_points) * tistep * 1000.0;
    
    if (raw->xmin >= xmax_read) {
        free(raw);
        fclose(fp);
        mexErrMsgTxt("\nreadgenesis: no data available above given xmin\n");
    }
    if (raw->xmax > xmax_read) {
        raw->xmax = xmax_read;
    }
    
    fst_pt = (long) (raw->xmin * raw->samp_frequency);
    lst_pt = (long) (raw->xmax * raw->samp_frequency);
    
    raw->no_samples = lst_pt - fst_pt;
    
    raw->fdata = (double *)malloc(raw->no_samples*sizeof(double));
    
    if (raw->fdata == NULL) {
        free(raw);
        fclose(fp);
        mexErrMsgTxt("\nreadgenesis: could not malloc data array\n");
    }
    
    raw->filled    = 1;
    raw->file_type = 3;
    sprintf(titlebuffer, "Plot %i from %s; read from %f to %f msec",
            plotno, file, raw->xmin, raw->xmax);
    assert(strlen(titlebuffer) < BUF_SIZE);
    strncpy(raw->title, titlebuffer, OLD_BUF_SIZE);
    
    fseek(fp, headersize, SEEK_SET);
    fseek(fp, (plotno-1) * sizeof(float), SEEK_CUR);
    fseek(fp, fst_pt * (noplots * sizeof(float)), SEEK_CUR);
    
    noitems = -1;
    blockpos = 0;
    
    readblock = ADFBLOCK - (ADFBLOCK % noplots);
    
    /* fill data pointer, convert to double */
    
    for (i = 0; i < raw->no_samples; i++) {
        if (blockpos >= noitems) {
            if ((noitems = fread(ftemp, sizeof(float), readblock, fp)) == 0)
            {
                break;
            } else {
                blockpos = 0;
            }
        }
#if (defined(__APPLE__) && (__DARWIN_BYTE_ORDER == __DARWIN_BIG_ENDIAN))      || \
    defined(__BIG_ENDIAN__) || \
    defined(WORDS_BIGENDIAN)
        wswap((char *)&fval, (char *)&ftemp[blockpos]);
        raw->fdata[i] = (double)(fval / gain);
#else
		raw->fdata[i] = (double)(ftemp[blockpos] / gain);
#endif
        blockpos += noplots;
    }
    
    fclose(fp);
    sprintf(temp_buffer, 
	      "readgenesis: %s, trace %i of %i (%d points @ %g kHz)",
            file, plotno, noplots, i, raw->samp_frequency);
    assert(strlen(tmp_buffer) < BUF_SIZE);
    mexWarnMsgIdAndTxt("readgenesis:info", temp_buffer);
    
    return(0);
}
