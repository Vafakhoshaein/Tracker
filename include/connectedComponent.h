#ifndef CONNECTED_COMPONENT_H
#define CONNECTED_COMPONENT_H

#include <opencv/cv.h>
#include <vector>

#ifdef QT_VERSION
#include <QDateTime>
#endif

typedef struct _BlobFeature* BlobFeature;

/*BlobFeature constructors*/
BlobFeature createBlob(const int* colorHistoram, int size, CvPoint centroid,
                       CvRect rect, int year, int month, int day, int hr,
                       int min, int sec, int ms);
BlobFeature createBlob(const BlobFeature f);
BlobFeature createBlob();



/*BlobFeature destructor*/
void destroyBlobFeature(BlobFeature* f);
void blob_feature_output_stats ();


/*BlobFeature getters*/
const int* getColorHistogram(const BlobFeature f);
int getSize(const BlobFeature f);
CvPoint centroid(const BlobFeature f);
CvRect  rect(const BlobFeature f);
CvPoint topLeft(const BlobFeature f);
CvPoint bottomRight(const BlobFeature f);
void   getBlobDateTime(const BlobFeature f, int* year, int* month, int* day, int* hr, int* min, int* sec, int* ms);
int    getHeight(const BlobFeature f);
int    getWidth(const BlobFeature f);
int    centX(const BlobFeature f);
int    centY(const BlobFeature f);
int    area(const BlobFeature f);

/*setters*/
void    setColorHistogram(BlobFeature f, const int* c);
void    setRect(BlobFeature f, int x, int y, int width, int height);
void    setXY(BlobFeature f, int x, int y);
void    setWidth(BlobFeature f, int w);
void    setHeight(BlobFeature f, int h);
void    setCentroid (BlobFeature f, CvPoint centroid);
void    setSize(BlobFeature f, int size);
void    setBlobDateTime(BlobFeature f, int year, int month, int day, int hr, int min, int sec, int ms);
void    setEqual (BlobFeature x, const BlobFeature y);


/*operations */
bool getComponents(const cv::Mat& binaryImage,
                   const cv::Mat& grayImage,
                   std::vector<BlobFeature>& features);
void mergeFeatures(BlobFeature f, const BlobFeature g);





#endif
