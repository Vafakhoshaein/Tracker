#include <connectedComponent.h>

extern "C"
{
#include <FlagImage.h>
}

#include <cv.hpp>
#include <queue>

typedef struct _BlobFeature
{
	int color_histogram[16];    //the grayscale histogram
	int size;                   // number of pixels in the component
	CvPoint centroid;
	CvRect rect;

	/*timestamp info*/
	int year, month, day, hr, min, sec, ms;
} _BlobFeature;


/*BlobFeature getters*/
const int* getColorHistogram(const BlobFeature f)
{
	return f->color_histogram;
}

int getSize(const BlobFeature f)
{
	return f->size;
}

CvPoint centroid(const BlobFeature f)
{
	return f->centroid;
}

int    centX(const BlobFeature f)
{
	return f->centroid.x;
}

int    centY(const BlobFeature f)
{
	return f->centroid.y;
}


CvRect  rect(const BlobFeature f)
{
	return f->rect;
}

int    getHeight(const BlobFeature f)
{
	return f->rect.height;
}

int    getWidth(const BlobFeature f)
{
	return f->rect.width;
}

int    area(const BlobFeature f)
{
	return f->rect.height * f->rect.width;
}

void    getBlobDateTime(const BlobFeature f, int* year, int* month, int* day, int* hr, int* min, int* sec, int* ms)
{
	if (f)
	{
		if (year)
			*year = f->year;
		if (month)
			*month = f->month;
		if (day)
			*day = f->day;
		if (hr)
			*hr = f->hr;
		if (min)
			*min = f->min;
		if (sec)
			*sec = f->sec;
		if (ms)
			*ms = f->ms;
	}
}

CvPoint topLeft(const BlobFeature f)
{
	return cvPoint (f->rect.x, f->rect.y);
}

CvPoint bottomRight(const BlobFeature f)
{
	return cvPoint (f->rect.x + f->rect.width,
			f->rect.y + f->rect.height);
}



/*BlobFeature setters*/
void    setBlobDateTime(BlobFeature f, int year, int month, int day, int hr, int min, int sec, int ms)
{
	if (f)
	{
		f->year = year;
		f->month = month;
		f->day = day;
		f->hr = hr;
		f->min = min;
		f->sec = sec;
		f->ms = ms;
	}
}

void    setRect(BlobFeature f, int x, int y, int width, int height)
{
	f->rect.x = x;
	f->rect.y = y;
	f->rect.width = width;
	f->rect.height = height;
}

void    setXY(BlobFeature f, int x, int y)
{
	f->rect.x = x;
	f->rect.y = y;
}

void    setWidth(BlobFeature f, int w)
{
	f->rect.width = w;
}

void    setHeight(BlobFeature f, int h)
{
	f->rect.height = h;
}

void    setCentroid (BlobFeature f, CvPoint centroid)
{
	f->centroid = centroid;
}

void    setSize(BlobFeature f, int size)
{
	f->size = size;
}

void    setColorHistogram(BlobFeature f, const int* c)
{
	memcpy(f->color_histogram, c, sizeof(int)*16);
}

void    setEqual (BlobFeature x, const BlobFeature y)
{
	memcpy(x, y, sizeof(_BlobFeature));
}

int 	n_created_blobs=0;
/*BlobFeature constructor*/
BlobFeature createBlob(const int* colorHistogram, int size, CvPoint centroid, CvRect rect, int year, int month, int day, int hr, int min, int sec, int ms)
{
	n_created_blobs ++;
	BlobFeature f = (BlobFeature) malloc(sizeof(_BlobFeature));
	if (f)
	{
		memcpy(f->color_histogram, colorHistogram, sizeof(int) * 16);
		f->size = size;
		f->centroid = centroid;
		f->rect = rect;
		f->year = year;
		f->month = month;
		f->day = day;
		f->hr = hr;
		f->min = min;
		f->sec = sec;
		f->ms = ms;
	}
	return f;
}

BlobFeature createBlob()
{
	n_created_blobs ++;
	BlobFeature f = (BlobFeature) malloc (sizeof(_BlobFeature));
	if (f)
		memset(f, 0, sizeof(_BlobFeature));
	return f;
}

BlobFeature createBlob(const BlobFeature f)
{
	n_created_blobs ++;
	BlobFeature retVal = (BlobFeature) malloc(sizeof(_BlobFeature));
	if (retVal)
		memcpy(retVal, f, sizeof(_BlobFeature));
	return retVal;
}

int n_dest_blobs = 0;
/*BlobFeature destructor*/
void destroyBlobFeature(BlobFeature* f)
{
	if (f && *f)
	{
		n_dest_blobs++;	

		free(*f);
		*f = NULL;
	}
}

//static unsigned char pixelAt(const IplImage* img, int width, int height)
//{
//    return img->imageData[height*img->width + width];
//}

static BlobFeature getComponentAt(int width, int height,
                                  const cv::Mat& binaryImage,
                                  const cv::Mat& grayImage,
                                  image_handle flagImage)
{
    BlobFeature f = (BlobFeature) malloc(sizeof(_BlobFeature));
    memset(f, 0, sizeof(_BlobFeature));
    std::queue<CvPoint> Q;
    Q.push(cvPoint(width, height));
    f->rect = cvRect(width, height, 0, 0);
    int min_x = width;
    int min_y = height;
    int max_x = width;
    int max_y = height;
    while (!Q.empty())
    {
        CvPoint currentPoint = Q.front();
        Q.pop();
        f->size++;
        f->centroid.x += currentPoint.x;
        f->centroid.y += currentPoint.y;
        unsigned char grayPixel = grayImage.at<unsigned char>(cv::Point2i(currentPoint.x, currentPoint.y));
        f->color_histogram[grayPixel/16]++;

        if (min_x > currentPoint.x)
            min_x = currentPoint.x;
        else if (max_x < currentPoint.x)
            max_x = currentPoint.x;

        if (min_y > currentPoint.y)
            min_y = currentPoint.y;
        else if (max_y < currentPoint.y)
            max_y = currentPoint.y;

        width = currentPoint.x;
        height = currentPoint.y;

        /*top-left*/
        if (!isFlaged(flagImage, width-1, height-1) && binaryImage.at<unsigned char>(cv::Point2i(width-1, height-1)))
        {
            setFlag(flagImage, width-1, height-1);
            Q.push(cvPoint(width-1, height-1));
        }

        /*top*/
        if (!isFlaged(flagImage, width, height-1) && binaryImage.at<unsigned char>(cv::Point2i(width, height-1)))
        {
            setFlag(flagImage, width, height-1);
            Q.push(cvPoint(width, height-1));
        }

        /*top-right*/
        if (!isFlaged(flagImage, width+1, height-1) && binaryImage.at<unsigned char>(cv::Point2i(width+1, height-1)))
        {
            setFlag(flagImage, width+1, height-1);
            Q.push(cvPoint(width+1, height-1));
        }

        /*left*/
        if (!isFlaged(flagImage, width-1, height) && binaryImage.at<unsigned char>(cv::Point2i(width-1, height)))
        {
            setFlag(flagImage, width-1, height);
            Q.push(cvPoint(width-1, height));
        }

        /*right*/
        if (!isFlaged(flagImage, width+1, height) && binaryImage.at<unsigned char>(cv::Point2i(width+1, height)))
        {
            setFlag(flagImage, width+1, height);
            Q.push(cvPoint(width+1, height));
        }

        /*bottom-left*/
        if (!isFlaged(flagImage, width-1, height+1) && binaryImage.at<unsigned char>(cv::Point2i(width-1, height+1)))
        {
            setFlag(flagImage, width-1, height+1);
            Q.push(cvPoint(width-1, height+1));
        }

        /*bottom*/
        if (!isFlaged(flagImage, width, height+1) && binaryImage.at<unsigned char>(cv::Point2i(width, height+1)))
        {
            setFlag(flagImage, width, height+1);
            Q.push(cvPoint(width, height+1));
        }

        /*bottom-right*/
        if (!isFlaged(flagImage, width+1, height+1) && binaryImage.at<unsigned char>(cv::Point2i(width+1, height+1)))
        {
            setFlag(flagImage, width+1, height+1);
            Q.push(cvPoint(width+1, height+1));
        }

    }
    f->centroid.x /= f->size;
    f->centroid.y /= f->size;
    f->rect.x = min_x;
    f->rect.y = min_y;
    f->rect.width = max_x - min_x;
    f->rect.height = max_y - min_y;
    n_created_blobs++;
    return f;
}

bool getComponents(const cv::Mat& binaryImage, const cv::Mat& grayImage, std::vector<BlobFeature>& features)
{

    /*check channels*/
    if (grayImage.channels() != 1 || binaryImage.channels() != 1)
        return false;

    /*check depth*/
    if (grayImage.depth() != CV_8U || binaryImage.depth() != CV_8U)
        return false;

    /*check association*/
    if (grayImage.cols!=binaryImage.cols || grayImage.rows!= binaryImage.rows)
        return false;

    image_handle flagImage = CreateImage(binaryImage.cols, binaryImage.rows);

    if (flagImage)
    {
        int i,j;
        for (i=0;i<binaryImage.rows;i++)
            for (j=0;j<binaryImage.cols;j++)
            {
                /*scan for the next pixel until we find a foreground pixel that has not yet been processed*/
                if (!isFlaged(flagImage, j, i) && binaryImage.at<unsigned char>(cv::Point2i(j, i)))
                    features.push_back(getComponentAt(j, i, binaryImage, grayImage, flagImage));
            }

        DestroyImage(&flagImage);
        return true;
    }
    else
        return false;
}


/*merges BlobFeature f with g and the results will be in f*/
void mergeFeatures(BlobFeature f,const BlobFeature g)
{
    //Determine upper left corner
    int x = (f->rect.x < g->rect.x) ? f->rect.x : g->rect.x;
    int y = (f->rect.y < g->rect.y) ? f->rect.y : g->rect.y;

    int f_br_x = f->rect.x + f->rect.width;
    int f_br_y = f->rect.y + f->rect.height;
    int g_br_x = g->rect.x + g->rect.width;
    int g_br_y = g->rect.y + g->rect.height;

    int width = (f_br_x > g_br_x) ? f_br_x - x : g_br_x - x;
    int height = (f_br_y > g_br_y) ? f_br_y - y : g_br_y - y;
    setRect(f, x, y, width, height);

    //Determine the size and centroid
    int size = f->size+g->size;
    float w_f = float(f->size)/float(size);
    float w_g = float(g->size)/float(size);
    CvPoint centroid;
    centroid.x = f->centroid.x*w_f + g->centroid.x*w_g;
    centroid.y = f->centroid.y*w_f + g->centroid.y*w_g;
    f->size = size;
    f->centroid = centroid;

    //Determine the color histogram
    for (int i=0; i<16; i++)
        f->color_histogram[i] += g->color_histogram[i];
}


void blob_feature_output_stats ()
{
	printf ("\n********************************************\n");
	printf ("created blobs = %d", n_created_blobs);
	printf ("\ndestroyed blobs = %d", n_dest_blobs);
	printf ("\n********************************************\n");

}
