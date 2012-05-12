#ifndef BACKGROUNDSEGMENTOR_H
#define BACKGROUNDSEGMENTOR_H


#include <cv.h>
#define DEFAULT_THRESHOLD 30
#define DEFAULT_ERODE 1
#define DEFAULT_DILATE 2
#define DEFAULT_NEGATIVE_LEARNING_STEP -1
#define DEFAULT_POSITIVE_LEARNING_STEP 3
#define DEFAULT_MAX_CONFIDENCE 50 
#define DEFAULT_UPDATE true

using namespace cv;


class BackgroundSegmentor
{
	public:
		BackgroundSegmentor();

		// Processing function
		void nextImage(const Mat & img);
		void nextImage(const Mat & img, const Mat & colourImage);

		// Initialization
		void initialize(const Mat & image, const Mat & colourImage = Mat());
		void reset();

		// Getters
		Mat getBackgroundModel(){return backgroundModel;}
		Mat getColourBackgroundModel(){return colourBackgroundModel;}
		Mat& getBinaryImage(){return binaryImage;}
		Mat getBinaryImage_8UC3();
		Mat getConfidenceImage(){return confidenceImage;}
		int getThreshold(){return threshold;}
		int getErodeIterations(){return erodeIterations;}
		int getDilateIterations(){return dilateIterations;}
		int getNegativeLearningStep(){return negativeLearningStep;}
		int getPositiveLearningStep(){return negativeLearningStep;}
		int getMaxConfidence(){return maxConfidence;}
		bool getUpdateFlag(){return update;}

		// Setters
		void setBackgroundModel(const Mat & image, const Mat & colourImage = Mat(), float confidence = 1.0f);
		void setThreshold(int _threshold){threshold = _threshold;}
		void setErodeIterations(int _erodeIterations){erodeIterations = _erodeIterations;}
		void setDilateIterations(int _dilateIterations){dilateIterations = _dilateIterations;}
		void setNegativeLearningStep(int _negativeLearningStep){negativeLearningStep = _negativeLearningStep;}
		void setPositiveLearningStep(int _positiveLearningStep){positiveLearningStep = _positiveLearningStep;}
		void setMaxConfidence(int _maxConfidence){maxConfidence = _maxConfidence;}
		void setUpdateFlag(bool _update);

	private:
		//Images
		Mat binaryImage;
		Mat backgroundModel;
		Mat confidenceImage;
		Mat colourBackgroundModel;

		// Parameters
		int threshold;
		int erodeIterations;
		int dilateIterations;
		int negativeLearningStep;
		int positiveLearningStep;
		int maxConfidence;
		bool update;

		// Flags
		bool initFlag;
};


#endif // BACKGROUNDSEGMENTOR_H
