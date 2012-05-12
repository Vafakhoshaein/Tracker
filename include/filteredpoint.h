#ifndef FILTEREDPOINT_H
#define FILTEREDPOINT_H

#include <opencv/cv.h>
using namespace cv;

class FilteredPoint
{
	public:
		FilteredPoint();
		void Initialize(double time_step, Point init_position, Point init_velocity);
		void Next_Measurement(Point & measurement);
		Point State(const unsigned & index);
		Point Predicted_state(const unsigned & index);
		void Evolve();

		CvKalman* x;
		CvKalman* y;

	private:

};

#endif // FILTEREDPOINT_H
