#ifndef EVENTTRACKER_H
#define EVENTTRACKER_H

#include <trace.h>
#include "BackgroundSegmentor.h"
#include "connectedComponent.h"
#include <QImage>
#include <sys/time.h>

extern "C"
{
#include <FiniteStateMachine.h>
}
#include <signalprocessor.h>
#include <dispatcher.h>

using namespace cv;

class EventTracker : public SignalProcessor
{
	public:
		explicit EventTracker(Dispatcher* _dispatcher);
		~EventTracker();
		void set_param(const std::string& var, void* param);

		void initialize(double _absoluteMinCost,
				unsigned int _dormantTime,
				unsigned int _transientTime,
				unsigned int _matchTime);

		// Setters
		void setAbsoluteMinCost(double _absoluteMinCost){ absoluteMinCost = _absoluteMinCost; }
		void setDormantTime(unsigned int _dormantTime){ dormantTime = _dormantTime; }
		void setTransientTime(unsigned int _transientTime){ transientTime = _transientTime; }
		void setMatchTime(unsigned int _matchTime){ matchTime = _matchTime; }


		// Getters
		vector<TracePtr> get_traces(){return traces;}
		vector<TargetPtr> get_targets(){return targets;}
		vector<BlobFeature> get_features();
		int get_frame_number(){return frame_number;}

		cv::Mat* nextImage(QImage* image);
		void reset();
		void destroyFeatures();

	protected:
		void process(int code, void *param);
		void destroy_pending_task(int code, void *param);
		void init_fsm(FiniteStateMachine fsm);
		void handle_transition(Transition transition, void* param);

	private:
		bool b_draw_bounding_boxes;
		bool b_draw_traces;
		bool b_draw_targets;
		bool b_record;
		struct timeval recorder_restart_ts;
		char* file_sequence_str;
		char* prefix;
		char* _filename;
		bool file_must_be_kept;


		// Extracting features (connected foreground components)
		void retrieve_image_components();
		void merge_overlapped_components();
		void merge_feature(vector<BlobFeature>& list_to_merge, std::vector<BlobFeature>::iterator);
		bool features_overlap(BlobFeature f, BlobFeature g, float threshold);

		// Matching features to traces
		void match_connected_components_with_traces();
		void get_distance_matrix(Mat & M);
		void get_cost_matrix(Mat & C, const Mat & M);
		void set_best_matches(Mat & M, const Mat & C);  //checks for each component
		double get_minimum_cost_of_component(int j, const Mat & M, const Mat & C); //of component j
		void set_minimum_matches_per_component(Mat & M, const Mat & C, int j, double min_cost); //of component j
		void assign_traces_to_components(const Mat & M);
		int get_matched_component(const Mat & M, int i);
		void create_new_traces();

		// Kalman Filtering
		void perform_kalman_filtering();

		// Retrieving occluded targets
		void predict_occluded_target_position();

		// Matching traces to targets
		void match_traces_with_targets();
		vector<TracePtr> get_traces_for_matching();
		bool target_is_being_tracked(TargetPtr T);
		vector<TargetPtr> get_targets_for_matching();
		void get_target_cost_matrix(Mat & M, vector<TracePtr> traces, vector<TargetPtr> targets);
		bool target_is_recent(TargetPtr T);
		bool trace_motion_is_similar(TracePtr t1, TracePtr t2);
		bool trace_matches_a_target(unsigned & trace_index, unsigned & target_index, const Mat & M);
		void create_new_targets_from_novel_traces(vector<TracePtr> traces_for_matching);


		vector<TracePtr> get_unmatched_traces();       // This function is involved in drawing the traces

		//database & video file related functions
		void process_inactive_targets();
		void createNewVideoFile(cv::Size size);
		void finish_recording();

		// Modules
		BackgroundSegmentor backSeg;

		// System parameters
		double absoluteMinCost;
		unsigned int dormantTime, transientTime, matchTime;

		// System variables
		vector<BlobFeature> features;
		vector<BlobFeature> features_for_matching;      //A temporary list of features used for the matching with traces
		vector<TracePtr> traces;
		vector<TargetPtr> targets;
		Mat grayImage;
		int frame_number;
		cv::VideoWriter* videoWriter;
		cv::VideoWriter* backgroundVideoWriter;

		//drawing functions
		void drawOnImages(QImage* img);
		void drawTrace(TracePtr trace, QImage* img);
		void drawTarget(TargetPtr target, QImage* img);

		//Finite State Machine
		void pairImageToInterface(QImage* colorImg);
		Transition NOT_RECORDING_TO_NOT_RECORDING,
			   READY_TO_NOT_RECORDING,
			   NOT_RECORDING_TO_READY,
			   NOT_RECORDING_TO_RECORDING,
			   RECORDING_TO_NOT_RECORDING,
			   RECORDING_TO_RECORDING_ON_NEW_IMAGE,
			   RECORDING_TO_RECORDING_ON_RESTART,
			   RECORDING_TO_READY;



};

#endif // EVENTTRACKER_H
