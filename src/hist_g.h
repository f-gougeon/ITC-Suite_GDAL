
// Structure to transfer some info extracted from histogram
//	Specially from ana_hist()

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
	int ipos90;			// position of 05% 
	float mean90;		// mean of values in top 90% of histogram
	int ipos95;			// position of 95% 
	float mean95;		// mean of values in top 95% of histogram
	} hist_inf;



// Typical grey-level histograms from full image 

//void gen_hist(PixVal *image, PIX_FUN_PTR, int64 image_size, int32 bins[]);

void gen_hist(void *image, PIX_FUN_PTR, int64 image_size, int32 bins[]);

//void gen_hist_and_mask(PixVal *image, PIX_FUN_PTR get_val , PixVal * bmp, int64 image_size, int32 bins[]);

void gen_hist_and_mask(void *image, PIX_FUN_PTR get_val , PixVal * bmp, int64 image_size, int32 bins[]);

void gen_hist_not_mask(void *image, PIX_FUN_PTR get_val , PixVal * bmp, int64 image_size, int32 bins[]);

int hist_moments(int32 bins[],int first,int last, double *mean, double *var, double *sdev, double *adev, double *skew, double *kurt);



//************************

// Histogram analysis for poorly populated histograms

void ana_hist(int *, int, hist_inf *);

// Initialize structure about histogram information (as above) to transfer back to a main (for ana_hist() )

void init_histo_struct(hist_inf *); 