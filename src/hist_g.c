/* hist_g.c (see also hist_g.h)

	Functions related to histogram acquisition and analysis
	

Simon Alexander	v1.0	Sep 96	
			- For itcvfol.c, converted to c from fortran
			
François Gougeon v1.1	Nov. Dec. 2000			
			- Added function ana_hist() and later "mode" within ana_hist()

François Gougeon v1.2	Sept. 2016
			- Some (int64) to deal with images bigger than 2GB
			- One of my machine has 64GB of memory and we sometimes want to deal
			 with images that are bigger than 2Gpixels in one shot (not by sections)
			 
François Gougeon v1.3	July - Sept.2021			 
			 
			- Added to function ana_hist() to get more  histogram stats
			This is more about "cruder", less populated histograms (features like heights in a stand)	
			Since there are lots of stast to return, I used a structure
			NOTE: Needed to fix ITCVFOL and NVEGMASK which were the only other 
			programs using  ana_hist()
			- Added Structure "histo_info" to transfer some info extracted from histogram (see hist.h)
			- Added init_histo_struct(hist_inf *) to init that structure
			
			- RENAMED hist_g.c (with hist_g.h) to make distinctions more obvious

François Gougeon v1.4	Oct. 2023

			- Minor modsto deal with 16bit and 32 bit images (also in hist_g.h)

*/


#include <math.h>
//#include "pci.h"
//#include "itc_io.h"
#include "itc_io_g.h"
#include "bitops.h"
#include "hist_g.h"


/***************************************************************/

// Generate a histogram of the pixel values in an IMAGE


//void gen_hist(PixVal *image, PIX_FUN_PTR get_val, int64 image_size, int bins[])

void gen_hist(void *image, PIX_FUN_PTR get_val, int64 image_size, int bins[])
{
	int64 j;
	
	for (j = 0; j < image_size; j++) bins[(*get_val) (image, j)]++;
}


/***************************************************************/

/* generate a histogram of pixels under mask bitmap*/

//void gen_hist_and_mask(PixVal *image, PIX_FUN_PTR get_val , PixVal * bmp, int64 image_size, int bins[]) 

void gen_hist_and_mask(void *image, PIX_FUN_PTR get_val , PixVal * bmp, int64 image_size, int bins[]) 
{
	int64 j;

	for (j = 0; j <= image_size - 1; j++)
		if (testbit(bmp, j))
			bins[(*get_val) (image, j)]++;
}

/***************************************************************/

/* generate a histogram of pixels not under mask bitmap*/

void gen_hist_not_mask(void *image, PIX_FUN_PTR get_val , PixVal * bmp, int64 image_size, int bins[]) 
{
	int64   j;

	for (j = 0; j <= image_size - 1; j++)
		if (!testbit(bmp, j))
			bins[(*get_val) (image, j)]++;
}

/***************************************************************/

/*
*	Calculate some standard moments for histogram data
*	range of bins to be considered is given by "first" and "last"
*/

int hist_moments(int bins[], int first, int last, double *mean, 
			double *var, double *sdev, double *adev, double *skew, double *kurt)
{
	int             i;
	int           n = 0;
	double          tmp = 0.0, tmp2 = 0.0, eps = 0.0;

	*mean = (*var) = (*sdev) = (*adev) = (*skew) = (*kurt) = 0.0;

	for (i = first; i <= last; i++) {
		*mean += i * bins[i];
		n += bins[i];
	}
	*mean /= n;

	for (i = first; i <= last; i++) {
		tmp = (i - (*mean));
		eps += bins[i] * tmp;
		*adev += bins[i] * fabs(tmp);
		tmp2 = tmp * tmp;
		*var += bins[i] * tmp2;
		tmp2 *= tmp;
		*skew += bins[i] * tmp2;
		tmp2 *= tmp;
		*kurt += bins[i] * tmp2;
	}

	*var = (*var - eps * eps / n) / (n - 1);	/* corrected for
							 * roundoff error */
	*sdev = sqrt(*var);
	*adev /= n;
	if (*var) {
		*skew /= (n * (*var) * (*sdev));
		*kurt /= (n * (*var) * (*var));
		*kurt -= 3.0;
	} else {
		return FALSE;
	}

	return TRUE;
}

/***************************************************************/

/* 	Function to get first and last meaningful values in an histogram 
	Also report on maximum count (i.e., mode) and mode position

Meaningful:  	There may be some noise in the histogram, but we are interested
		in the beginning and the end of main population.
		
		NOTE: Previous functions (above) were more about dealing with image grey level histograms
			This is more about "cruder", less populated histograms (of object features)
*/

	// Structure to transfer info extracted from histogram (***NOW*** define in hist.h)

/*
typedef struct histo_info 
	{
	int samples;		// number of samples in histogram
	int first;			// first bin of interest
	int last;			// last bin of interest
	int range;			// range of useful value
	int m_count;		// count at mode
	int mode;			//mode position in histogram
	int min_count;		//count threshold to remove not so well populated bins
	float mean;			// crude mean as gathered from histogram
	float st_dev;		// crude standard deviation as gathered from histogram
	int ipos90;			// position of 90% 
	float mean90;		// mean of values in top 90% of histogram
	int ipos95;			// position of 95% 
	float mean95;		// mean of values in top 95% of histogram
	} hist_inf;
	 
*/

// WAS   void ana_hist(int bins[], int hsize, int *first, int *last, int *m_count, int *mode

void ana_hist(int bins[], int hsize, hist_inf *ph)
{
	int i;
	int prev, curr, next;
	int64 hist_count=0, icount=0, isum=0;
	int ifirst, ilast, irange, min_count, ipos90, ipos95 ;
	float mean,  mean90=0.0, mean95=0.0;
	int	m_count = 0, mode=0;
	float st_dev, sum = 0.0;
	//double  st_dev, sum = 0.0;
		
	FILE *Report;  
	Report = fopen("NUL", "w");			// to get rid of lots of printing
	//Report = stdout;	  				// to all the debugging info
	  

	
	for (i = 1; i < hsize; i++) hist_count += bins[i];	
	fprintf(Report,"ana_hist() : Number of items in the histogram : %Id \n", hist_count);
	
	printf("\n ana_hist() : Number of items in the histogram : %Id \n", hist_count);
	ph->samples = hist_count;




// Forget about bin for DN = 0 cause in the case of image, it could just be background pixels

	// Smooth histogram (a bit imbred) 
/*
	for (i = 2; i < (hsize-1); i++)	
	  bins[i] = ( bins[i-1] + bins[i] + bins[i+1] ) / 3;
*/

		
	// Smooth histogram - less imbred??  ### DO LATER AFTER FIRST AND LAST ASCERTAINED

	prev = bins[1];			// minor issue if first few elements are zero
	curr = bins[2];

	for (i = 2; i < (hsize-1); i++) 
	  {
	  next = bins[i + 1];
	  bins[i] = (prev + curr + next) / 3;
	  //if (next == 0)  bins[i] = (prev + curr) / 2;	// trailing zeros - not an issue if hsize is real size
	  prev = curr;		// prep for next item 
	  curr = next;
	  }
	  
 
	
	/* get maximum (mode) count */  
	
	m_count = 0; mode=0;	
	for (i = 1; i < hsize; i++) { if( bins[i] >= m_count) {m_count=bins[i]; mode=i;} }

	ph->m_count = m_count;		// transfer to main program via structure
	ph->mode = mode;
		
	/* prep to remove non-significant quantities in some bins */

	min_count = m_count /100 ; 		// 1% of max count 
	
	/* clean up the histogram tails */
	
	for (i = 1; i < hsize; i++) { {if (bins[i] < min_count)  bins[i] = 0; } }

	/* find first nonzero element */
	
	for (i = 1; i < hsize; i++) {if (bins[i] > 0) { ifirst = i; break; } }

	/* find last nonzero element */
	
	for (i = hsize - 1; i >= 0; i--) {if (bins[i] > 0) { ilast = i; break; } }

	irange = ilast - ifirst;
	ph->first = ifirst;			// transfer to main program via structure
	ph->last = ilast;
	ph->range = irange;
	
// Print some histogram to check

/*
	printf("\nLow Res Histogram (by bins of 5): ");
	for (i = 10; i <= 100; i=i+5)  printf("%2d, ",bins[i]);
	printf("\n");
*/
		
// Dont do further analysis if histogram is too sparse

	if( hist_count < 30)  return;

	
// Get ROUGH Mean of data (as per histogram)

	icount=0, isum=0;
	for (i = ifirst; i <= ilast; i++)
	  {
	  icount += bins[i];
	  isum += i * bins[i];
	  }
	mean = ((float)isum / icount);
	ph->mean = mean;
	fprintf(Report,"Mean of histogram is  %5.1f \n", mean);


// Get ROUGH Standard Deviation  (as per histogram)

	icount=0, sum=0.0;
	for (i = ifirst; i <= ilast; i++)
	  {
	  icount += bins[i];
	  sum +=  (float)bins[i] *  pow( ( (float)i - mean ), 2.0 );
	  //sum +=  pow( (double)bins[i] * ((double)i - mean), 2.0 );
	  //sum +=  pow( ( (float)(i*bins[i]) - mean), 2.0);
	  //sum += pow((float)(i*(bins[i]-(int)mean)), (float) 2.0);	  
	  //sum += ( (i*(bins[i]-(int)mean)) *  (i*(bins[i]-(int)mean)) );		  
	  }
	st_dev = sqrt( (sum / icount) );
	
	ph->st_dev = st_dev;
	fprintf(Report,"Standard deviation of histogram is  %5.2f \n", st_dev);
	
	//st_dev = irange / 3;			// heuristic
	//fprintf(Report,"Heuristic - Standard deviation of histogram is  %3.2f \n", st_dev);
	fprintf(Report,"Range is from %d to %d thus %d \n", ifirst, ilast, irange);

// Find position of 95% of data 

	ipos90 = ilast -  lround(0.10*irange);
	fprintf(Report,"Position of start of top 90%% is : %d \n", ipos90);
	ph->ipos90 = ipos90;
	
// Find position of 95% of data 

	ipos95 = ilast -  lround(0.05*irange);
	fprintf(Report,"Position of start of top 95%% is : %d \n", ipos95);
	ph->ipos95 = ipos95;

// Get mean between ipos90 and last
	icount=0, sum=0.0;

	for (i = ipos90; i <= ilast; i++)
	  {
	  icount += bins[i];
	  sum += (float) i * bins[i];
	  }
	mean90 = (sum/icount);
	ph->mean90 = mean90;
	fprintf(Report,"Mean of top 90%% entries is : %3.2f \n", mean90);



// Get mean between ipos95 and last
	icount=0, sum=0.0;

	for (i = ipos95; i <= ilast; i++)
	  {
	  icount += bins[i];
	  sum += (float) i * bins[i];
	  }
	mean95 = (sum/icount);
	ph->mean95 = mean95;
	fprintf(Report,"Mean of top 95%% entries is : %3.2f \n", mean95);



	return;

}

//*************************************************************

// Initialize structure about histogram information


void init_histo_struct(hist_inf *p)
	{
	p->samples = 0;			// number of samples in histogram		
	p->first = 0;			// first bin of interest
	p->last = 0;			// last bin of interest
	p->range = 0;			// range of useful value
	p->m_count = 0;			// count at mode
	p->mode = 0;			//mode position in histogram
	p->min_count = 0;		//count threshold to remove not so well populated bins
	p->mean = 0.0;			// crude mean as gathered from histogram
	p->st_dev = 0.0;		// crude standard deviation as gathered from histogram
	p->ipos90 = 0;			// position of 95% 
	p->mean90 = 0.0;		// mean of values in top 95% of histogram
	p->ipos95 = 0;			// position of 95% 
	p->mean95 = 0.0;		// mean of values in top 95% of histogram
	}
	 


/***************************************************************/
/***************************************************************/
