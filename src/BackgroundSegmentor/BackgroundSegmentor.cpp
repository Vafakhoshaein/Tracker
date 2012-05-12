#include "BackgroundSegmentor.h"
#include <QTime>

using namespace cv;

BackgroundSegmentor::BackgroundSegmentor()
{
	reset();
}

void BackgroundSegmentor::reset()
{
	negativeLearningStep = DEFAULT_NEGATIVE_LEARNING_STEP;
	positiveLearningStep = DEFAULT_POSITIVE_LEARNING_STEP;
	maxConfidence = DEFAULT_MAX_CONFIDENCE;
	threshold = DEFAULT_THRESHOLD;
	erodeIterations = DEFAULT_ERODE;
	dilateIterations = DEFAULT_DILATE;
	update = DEFAULT_UPDATE;
	initFlag = true;
}

void BackgroundSegmentor::initialize(const Mat & image, const Mat & colourImage)
{
	setBackgroundModel(image, colourImage);
	initFlag = false;
}

void BackgroundSegmentor::nextImage(const Mat & image)
{
	if(initFlag) initialize(image);

	int ls;

	const unsigned char *img, *imgEnd;
	unsigned char *bin, *model;
	int *conf;

	// Scan the entire image
	for (int y=0; y<image.rows; y++)
	{
		img = image.ptr<unsigned char>(y);
		imgEnd = img + image.cols;
		bin = binaryImage.ptr<unsigned char>(y);
		model = backgroundModel.ptr<unsigned char>(y);
		conf = confidenceImage.ptr<int>(y);

		for(; img != imgEnd; img++, bin++, model++, conf++)
		{
			if(  abs(*img - *model) > threshold )
			{
				*bin = 255;
				ls = negativeLearningStep;
			}
			else
			{
				*bin = 0;
				ls = positiveLearningStep;
			}

			// Model Update
			if (update)
			{
				if(*conf + ls <= 0)
				{
					*conf = positiveLearningStep + negativeLearningStep;
					*model = *img;
				}
				else
				{
					if (*conf + ls > maxConfidence) *conf = maxConfidence;
					else *conf = *conf + ls;
				}
			}
		}
	}

	// Binary Image post processing
	erode(binaryImage, binaryImage, Mat(), Point(-1,-1),erodeIterations);
	dilate(binaryImage, binaryImage, Mat(), Point(-1,-1), dilateIterations);
}

void BackgroundSegmentor::nextImage(const Mat & image, const Mat & colourImage)
{
	if(initFlag) initialize(image, colourImage);

	int ls;

	const unsigned char *img, *imgEnd, *colourImg;
	unsigned char *bin, *model, *colourMdl;
	int *conf;

	// Scan the entire image
	for (int y=0; y<image.rows; y++)
	{
		img = image.ptr<unsigned char>(y);
		imgEnd = img + image.cols;
		bin = binaryImage.ptr<unsigned char>(y);
		model = backgroundModel.ptr<unsigned char>(y);
		conf = confidenceImage.ptr<int>(y);
		colourMdl = colourBackgroundModel.ptr<unsigned char>(y);
		colourImg = colourImage.ptr<unsigned char>(y);

		for(; img != imgEnd; img++, bin++, model++, conf++, colourImg+=3, colourMdl+=3)
		{
			if(  abs(*img - *model) > threshold )
			{
				*bin = 255;
				ls = negativeLearningStep;
			}
			else
			{
				*bin = 0;
				ls = positiveLearningStep;
			}

			// Model Update
			if (update)
			{
				if(*conf + ls <= 0)
				{
					*conf = positiveLearningStep + negativeLearningStep;
					*model = *img;
					*colourMdl = *colourImg;
					colourMdl[1] = colourImg[1];
					colourMdl[2] = colourImg[2];
				}
				else
				{
					if (*conf + ls > maxConfidence) *conf = maxConfidence;
					else *conf = *conf + ls;
				}
			}
		}
	}

	// Binary Image post processing
	erode(binaryImage, binaryImage, Mat(), Point(-1,-1),erodeIterations);
	dilate(binaryImage, binaryImage, Mat(), Point(-1,-1), dilateIterations);
}

void BackgroundSegmentor::setBackgroundModel(const Mat & image, const Mat & colourImage, float confidence)
{
	image.copyTo(backgroundModel);
	colourImage.copyTo(colourBackgroundModel);
	confidenceImage.create(image.size(), CV_32SC1);
	confidenceImage.setTo(confidence*DEFAULT_MAX_CONFIDENCE);
	binaryImage.create(image.size(), CV_8UC1);
}

Mat BackgroundSegmentor::getBinaryImage_8UC3()
{
	Mat retImg;
	retImg.create(binaryImage.size(), CV_8UC3);
	cv::cvtColor(binaryImage, retImg, CV_GRAY2RGB);
	return retImg;
}

