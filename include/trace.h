#ifndef TRACE_H
#define TRACE_H

#include <memory>
#include "filteredpoint.h"
#include "common.h"
#include <boost/shared_ptr.hpp>
#include <QDateTime>
#include <connectedComponent.h>
#include <iostream>

using namespace std;
//using namespace cv;

const int KALMAN_START_LENGTH = 2;
const float DISPLACEMENT_THRESHOLD = 50;

struct FilteredFeature 
{
	FilteredPoint centroid;
	CvKalman* size;
	CvKalman* width;
	CvKalman* height;
	CvKalman* color_histogram[16];
};

struct myRange 
{
	int min;
	int max;
};

class Trace;
typedef boost::shared_ptr<Trace> TracePtr;
#define DIALATE_FACTOR 0

class Trace 
{
	public:
		Trace(int init_frame);
		static unsigned int id_generator;
		bool is_blob_close(const BlobFeature blob);
		int get_id();
		int get_initial_frame();
		int get_length();
		int get_last_frame();
		void set_color(CvScalar new_color);
		CvScalar get_color();
		void next_frame(const BlobFeature feature);
		~Trace();
		CvRect get_matching_region();

		BlobFeature get_last_feature();

		//To do with Kalman Filtering
		void Perform_Kalman_Filtering();
		void Evolve();
		void Predict_Kalman_Filter();
		void Correct_Kalman_Filter(BlobFeature m);

		//To do with History Matching
		void Combine(TracePtr T);
		void get_kalman_state_post(BlobFeature f);
		void get_kalman_state_pre(BlobFeature f);

		vector<BlobFeature> nodes;
		vector<BlobFeature> filtered_nodes;


		FilteredFeature ff;

		bool is_matched;
		bool kalman_filter_is_initialized;

		const BlobFeature& get_expected_feature() const
		{
			return expected_feature;
		}

		bool is_mobile();
		BlobFeature get_first_feature();
		inline Point get_velocity();
		void insert_changes_to_db(int targetId);
		QDateTime getStartTime() { return startTime; }
		QDateTime getEndTime() { return endTime; }

		std::vector<BlobFeature>::iterator nodeBegin() {return nodes.begin();}
		std::vector<BlobFeature>::iterator nodeEnd() {return nodes.end();}


	private:
		//int get_distance(int x, int y);
		QDateTime startTime, endTime;

		int id;
		int initial_frame;
		int length;
		CvScalar color;
		BlobFeature expected_feature;
		bool mobile;
		myRange x, y;

		//To do with Kalman Filtering
		void Initialize_Kalman_Filter();
};

inline Point Trace::get_velocity()
{
	return ff.centroid.State(1);
}

// Functions involving traces and blob features
double cost_value(const BlobFeature f, const BlobFeature g);
bool f_contains_g(const BlobFeature f, const BlobFeature g);


// Target Class
class Target;
typedef boost::shared_ptr<Target> TargetPtr;

class Target 
{
	public:
		Target();
		int get_number_of_traces();
		TracePtr get_trace(int index);
		void add_trace(TracePtr new_trace);
		unsigned get_id();
		int get_initial_frame();
		int get_last_frame();
		TracePtr get_last_trace();
		void draw(IplImage* image);
		void Evolve();
		bool is_mobile();
		static unsigned id_generator;
		QDateTime getStartTime();
		QDateTime getEndTime();
		vector<TracePtr>& getTraces() {return traces;}


	private:
		unsigned id;
		vector<TracePtr> traces;
		bool mobile;
};
#endif // TRACE_H
