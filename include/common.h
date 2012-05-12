#ifndef COMMON_UTILITY_H
#define COMMON_UTILITY_H

#include <opencv/cv.h>
#include <opencv/highgui.h>
//#include "pair.h"
#include <QString>
#include <QList>
#include <QImage>
#include <sys/time.h>

#define PLCMNT_NONE 0
#define PLCMNT_CENTER 1

#define APPEND_TWO_DIGITS(string, value) \
    string = string.append(value < 10 ? "0%1" : "%1").arg(value);

#define GET_ABSOLUTE_TIME(t) {struct timeval a; gettimeofday(&a, NULL); t.tv_sec = a.tv_sec; t.tv_nsec = a.tv_usec * 1000;}

#define ADD_TIME(result, time1, time2) {\
	 result.tv_sec = time1.tv_sec + time2.tv_sec; \
    	 result.tv_nsec = time1.tv_nsec + time2.tv_nsec; \
	 if (result.tv_nsec >= 1000000000L)  \
		{result.tv_sec++ ;  result.tv_nsec -= 1000000000L;}}

#define SUB_TIME(result, time1, time2) {\
	result.tv_sec = time1.tv_sec - time2.tv_sec; \
	result.tv_nsec = time1.tv_nsec - time2.tv_nsec; \
	if (result.tv_nsec < 0) \
		{result.tv_sec--;   result.tv_nsec += 1000000000L;}}


void append_int(QString* text, int num);
//void add_text(IplImage* image, const char* text, Pair* point, CvFont* font, CvScalar color, int placement);
//CvPoint center_placement(Pair* point, const char* text, CvFont* font);

template <class T>
QList<T> list_intersection(QList<QList<T>*>* lists);

void 	trace_flag_add();
void 	target_flag_add();
void 	trace_flag_zero();
void 	target_flag_zero();
int 	trace_flag_get();
int 	target_flag_get();

using namespace cv;

unsigned char rand_uchar();
int 	rand_int( int min_value, int max_value);
void 	log_clear();
void 	log_text(const char* fmt, ...);
void 	Mat2Str(const char* title, Mat & m, QString & line);
void 	logMatrix(const char* title, Mat & m);
void 	outputMatrix(const char* title, Mat & m);
void 	showLabelImage(Mat & labelImage, Mat & colourImage, int n, const string & windowName);
bool 	boxes_overlap(CvRect b1, CvRect b2);

cv::Mat* QImage2IplImage(QImage *qimg);
QImage*  IplImage2QImage(IplImage *iplImg);
void  	 QImage2CvMat(QImage* img, cv::Mat* mat);
void 	 getCurrentTimeString(char* buf, int size);


#endif // COMMON_UTILITY_H
