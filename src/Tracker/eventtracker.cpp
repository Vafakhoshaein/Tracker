#include "eventtracker.h"
#include <QDateTime>
#include <highgui.h>
#include <SignalCode.h>
#include <stdio.h>
#include <string>
#include <connectedComponent.h>
#include <QPainter>
#include <glib.h>
#include <QRgb>
#include <sys/time.h>

#define ET_DEFAULT_ABSOLUTE_MIN_COST 1.5
#define ET_DEFAULT_DORMANT_TIME 80
#define ET_DEFAULT_TRANSIENT_TIME 3
#define ET_DEFAULT_MATCH_TIME 3

using namespace cv;

void
EventTracker::handle_transition(Transition transition, void* param)
{
	if (transition == NOT_RECORDING_TO_NOT_RECORDING)
	{
		QImage* img = reinterpret_cast<QImage*>(param);
		cv::Mat* cvImg = nextImage(img);

		if (traces.empty() && targets.empty())
		{
			pairImageToInterface(img);
			destroyFeatures();
			delete cvImg;
		}
		else if (!b_record)   /*targets/traces are available but we do not want to record*/
		{
			match_traces_with_targets();
			drawOnImages(img);
			pairImageToInterface(img);
			destroyFeatures();
			process_inactive_targets();
			delete cvImg;
		}
		else
		{
			std::pair<QImage*, cv::Mat*>* p = new std::pair<QImage*, cv::Mat*>();
			p->first = img;
			p->second = cvImg;
			apply_event("start-recording", p);
		}
		return;
	}
	else if (transition == RECORDING_TO_NOT_RECORDING)
	{
		finish_recording();
		if (!file_must_be_kept)
		{
			printf("The last sequence that was recorded (%s) was useless therefore it will be removed from the hard-drive\n", _filename);
			remove(_filename);
		}

		return;
	}
	else if (transition == RECORDING_TO_RECORDING_ON_NEW_IMAGE)
	{
		QImage* img = reinterpret_cast<QImage*>(param);
		cv::Mat* cvImg = nextImage(img);
		match_traces_with_targets();
		drawOnImages(img);
		pairImageToInterface(img);
		destroyFeatures();
		process_inactive_targets();

		if (traces.empty() && targets.empty())
		{
			delete cvImg;
			apply_event("stop-recording", NULL);
		}
		else
		{
			struct timeval temp;
			gettimeofday(&temp, NULL);
			if (temp.tv_sec - recorder_restart_ts.tv_sec >= 60)
			{
				apply_event("restart-recorder", cvImg);
			}
			else
			{
				*videoWriter << *cvImg;
				*backgroundVideoWriter << backSeg.getBinaryImage_8UC3();
				delete cvImg;
			}
		}
		return;
	}
	else if (transition == RECORDING_TO_RECORDING_ON_RESTART)
	{
		cv::Mat* cvImg = reinterpret_cast<cv::Mat*>(param);
		finish_recording();
		createNewVideoFile(cvImg->size());

		if (videoWriter && backgroundVideoWriter) {
			*videoWriter << *cvImg;
			*backgroundVideoWriter << backSeg.getBinaryImage_8UC3();
		} else  {
			apply_event("stop", NULL);
			dispatcher->send_signal("controller", FAILURE, NULL);
		}

		delete cvImg;
		return;
	}

	if (transition == READY_TO_NOT_RECORDING)
	{
	}
	else if (transition == RECORDING_TO_READY)
	{
		finish_recording();

		vector<TargetPtr>* _targets = new vector<TargetPtr>();
		for (vector<TargetPtr>::iterator iter = targets.begin(); iter != targets.end(); iter++)
			_targets->push_back(*iter);
		targets.clear();
		dispatcher->send_signal("controller", DB_INSERT_TARGET , _targets);
		reset();
	}
	else if (transition == NOT_RECORDING_TO_RECORDING)
	{
		std::pair<QImage*, cv::Mat*>* p = reinterpret_cast<std::pair<QImage*, cv::Mat*>*>(param);
		QImage* img = p->first;
		cv::Mat* cvImg = p->second;
		delete p;
		match_traces_with_targets();
		drawOnImages(img);
		pairImageToInterface(img);
		destroyFeatures();
		file_must_be_kept = false;
		createNewVideoFile(cvImg->size());

		if (videoWriter && backgroundVideoWriter) {
			*videoWriter << *cvImg;
			*backgroundVideoWriter << backSeg.getBinaryImage_8UC3();
		}
		else {
			apply_event("stop", NULL);
			dispatcher->send_signal("controller", FAILURE, NULL);
		}

		delete cvImg;
		return;
	}
	else if (transition == NOT_RECORDING_TO_READY)
		reset();
}

void 
EventTracker::process(int code, void *param)
{
	switch(code)
	{
		case NEW_IMAGE:
			{
				if (queue_size() > 10) {
					destroy_pending_task(code, param);
				} else {
					Transition t;
					if (!apply_event("new-image", param)) {
						destroy_pending_task(code, param);
					}
				}
				break;
			}
		case START:
			{
				apply_event("start", NULL);
				break;
			}
		case STOP:
			{
				apply_event("stop", NULL);
				break;
			}
		case TURN_ON_BOUNDING_BOX:
			{
				b_draw_bounding_boxes = true;
				break;
			}
		case TURN_OFF_BOUNDING_BOX:
			{
				b_draw_bounding_boxes = false;
				break;
			}
		case TURN_ON_DRAW_TRACES:
			{
				this->b_draw_traces = true;
				break;
			}
		case TURN_OFF_DRAW_TRACES:
			{
				this->b_draw_traces = false;
				break;
			}
		case TURN_ON_DRAW_TARGETS:
			{
				this->b_draw_targets = true;
				break;
			}
		case TURN_OFF_DRAW_TARGETS:
			{
				this->b_draw_targets = false;
				break;
			}
	}
}

void 
EventTracker::destroy_pending_task(int code, void *param)
{
	switch (code)
	{
		case NEW_IMAGE:
			QImage* img = reinterpret_cast<QImage*>(param);
			delete img;
	}
}

//constructor
EventTracker::EventTracker(Dispatcher* _dispatcher):
	SignalProcessor("tracker", _dispatcher),
        frame_number(0),
        b_draw_bounding_boxes(false),
        b_draw_traces(false),
        b_draw_targets(false),
	b_record(true),
	videoWriter(NULL),
	backgroundVideoWriter(NULL)
{
	_init_fsm(this);
	initialize(ET_DEFAULT_ABSOLUTE_MIN_COST,
			ET_DEFAULT_DORMANT_TIME,
			ET_DEFAULT_TRANSIENT_TIME,
			ET_DEFAULT_MATCH_TIME);
}

void
EventTracker::init_fsm(FiniteStateMachine fsm)
{
	fsm_add_state(fsm, "recording");
	fsm_add_state(fsm, "not-recording");
	prefix=strdup("");
	_filename=g_strdup("");

	READY_TO_NOT_RECORDING = fsm_link_states(fsm, "start", "ready", "not-recording");
	NOT_RECORDING_TO_NOT_RECORDING = fsm_link_states(fsm, "new-image", "not-recording", "not-recording");
	NOT_RECORDING_TO_READY = fsm_link_states(fsm, "stop", "not-recording", "ready");
	NOT_RECORDING_TO_RECORDING = fsm_link_states(fsm, "start-recording", "not-recording", "recording");
	RECORDING_TO_NOT_RECORDING = fsm_link_states(fsm, "stop-recording", "recording", "not-recording");
	RECORDING_TO_RECORDING_ON_NEW_IMAGE = fsm_link_states(fsm, "new-image", "recording", "recording");
	RECORDING_TO_RECORDING_ON_RESTART = fsm_link_states(fsm, "restart-recorder", "recording", "recording");
	RECORDING_TO_READY = fsm_link_states(fsm, "stop", "recording", "ready");
}

EventTracker::~EventTracker()
{
	free(prefix);
}

void 
EventTracker::initialize(double _absoluteMinCost,
                              unsigned int _dormantTime,
                              unsigned int _transientTime,
                              unsigned int _matchTime)
{
	setAbsoluteMinCost(_absoluteMinCost);
	setDormantTime(_dormantTime);
	setTransientTime(_transientTime);
	setMatchTime(_matchTime);
}

/*send the images to the GUI*/
void 
EventTracker::pairImageToInterface(QImage* img)
{
	cv::Mat& binImage = backSeg.getBinaryImage();
	QImage* qbinImage = new QImage(QSize(binImage.cols, binImage.rows), QImage::Format_Indexed8);
	qbinImage->setColor(0, qRgb (0,0,0));
	qbinImage->setColor(255, qRgb(255,255,255));

	for (int i=0;i<qbinImage->height();i++)
		for (int j=0;j<qbinImage->width();j++)
		{
			if (binImage.at<char>(i,j))
				qbinImage->setPixel(j,i,255);
			else
				qbinImage->setPixel(j,i,0);
		}


	std::pair<QImage*, QImage*>* temp_pair = new std::pair<QImage*, QImage*>;
	temp_pair->first = img;
	temp_pair->second = qbinImage;
	dispatcher->send_signal("controller", PAIR_IMAGE, temp_pair);
}

int motionFreeFrameCount = 0;
//process the next frame and if the img is NULL, end the tracking.
cv::Mat* 
EventTracker::nextImage(QImage* qimg)
{
/*	static int count = 0;
	count++;
	struct timeval t1, t2;
	gettimeofday(&t1, NULL);*/

	cv::Mat* image = QImage2IplImage(qimg);
//	gettimeofday(&t2, NULL);

	frame_number++;
	cvtColor(*image,grayImage, CV_BGR2GRAY);
	retrieve_image_components();
	match_connected_components_with_traces();
	perform_kalman_filtering();
	predict_occluded_target_position();

/*	if (count==20) {
		count=0;
		int _t1 = (t1.tv_sec * 1000000) + (t1.tv_usec);
		int _t2 = (t2.tv_sec * 1000000) + (t2.tv_usec);
		printf ("It took %d microseconds to process an image\n", _t2 - _t1);
	}*/

	return image;
}

void 
EventTracker::destroyFeatures()
{

	for (std::vector<BlobFeature>::iterator iter = features.begin(); iter != features.end(); iter++)
	{
		BlobFeature feature = *iter;
		destroyBlobFeature(&feature);
	}
	features.clear();
}

//returns the distance matrix
void 
EventTracker::get_distance_matrix(Mat & M)
{
	M.create(traces.size(), features_for_matching.size(), CV_8UC1);

	//setup M matrix
	unsigned char* m = M.data;

	for (vector<TracePtr>::iterator t = traces.begin(); t != traces.end(); t++)
		for (vector<BlobFeature>::iterator f = features_for_matching.begin(); f != features_for_matching.end(); f++, m++)
			*m = (*t)->is_blob_close(*f) ? 1 : 0;
}

//returns the cost matrix
void 
EventTracker::get_cost_matrix(Mat & C, const Mat & M)
{
	C.create(traces.size(), features_for_matching.size(), CV_64FC1);

	//setup C matrix
	unsigned char *m = M.data;
	double *c = (double*)C.data;
	for (vector<TracePtr>::iterator t = traces.begin(); t != traces.end(); t++)
		for (vector<BlobFeature>::iterator f = features_for_matching.begin(); f != features_for_matching.end(); f++, m++, c++)
			*c = (*m) ? cost_value((*t)->get_expected_feature(), *f) : -1;
}

//match traces with the best matching component from the component list.
void 
EventTracker::match_connected_components_with_traces()
{
	features_for_matching = features;

	//if there's any trace to match the components, do the matching
	if (!traces.empty() && !features_for_matching.empty())
	{
		Mat M, C;

		get_distance_matrix(M); //scan each component and each trace and set up M matrix if they are in a close distance from each other
		get_cost_matrix(C, M);    //create a cost matrix only for the matches from matrix M
		set_best_matches(M, C); //modify the matrix M (set the lowest costs to 1 and the rest to 0)
		assign_traces_to_components(M); //matrix M is ready for the matching

	}
	else
		if (features_for_matching.empty()) // if there's no component but there are traces
			traces.clear();

	//create new traces from the components that are left
	create_new_traces();
}

//add new frames to each trace according to M matrix
void 
EventTracker::assign_traces_to_components(const Mat & M)
{
	for (int i=0, iEnd = (int)traces.size(); i<iEnd; i++)
	{
		int j = get_matched_component(M, i);
		if (j != -1)
		{
			traces[i]->next_frame(features_for_matching[j]);
			features_for_matching[j] = NULL;
		}
		else
			traces[i].reset();
	}

	//remove all the nulls from vector traces
	vector<TracePtr> _traces;
	for (vector<TracePtr>::iterator iter = traces.begin(), iterEnd = traces.end(); iter != iterEnd; iter++)
		if (*iter != NULL)
			_traces.push_back(*iter);

	std::vector<BlobFeature> _features;
	//remove all the nulls from features_for_matching list
	for (std::vector<BlobFeature>::iterator iter = features_for_matching.begin(); iter != features_for_matching.end(); iter++)
		if (*iter != NULL)
			_features.push_back(*iter);


	traces = _traces;
	features_for_matching = _features;
}

//finds the index of the component associated to trace i.
//if no component is found, it returns -1
int 
EventTracker::get_matched_component(const Mat & M, int i)
{
	const unsigned char *m = M.ptr(i);
	for (int j=0; j<M.cols; m++, j++) if (*m) return j;
	return -1;
}

//create traces from the components that are still in the list(are not processed)
void 
EventTracker::create_new_traces()
{
	for (std::vector<BlobFeature>::iterator f = features_for_matching.begin(); f != features_for_matching.end();  f++)
	{
		if (*f)
		{
			TracePtr trace(new Trace(frame_number));
			trace->next_frame(*f);
			traces.push_back(trace);
		}
	}
	features_for_matching.clear();
}



//select the best(minimum cost) match for each component. set_best_matches() only modifies the M(match) matrix.
void 
EventTracker::set_best_matches(Mat & M, const Mat & C)
{
	//Do a "per component" matching
	for (int j=0, jMax = (int)features_for_matching.size(); j<jMax; j++)
		set_minimum_matches_per_component(M, C, j, get_minimum_cost_of_component(j, M, C));
}

//returns the minimum cost of component j
double 
EventTracker::get_minimum_cost_of_component(int j, const Mat & M, const Mat & C)
{
	double cost, minCost = 1E308; //start with a very large cost
	for (int i=0; i<M.rows; i++)
	{
		cost = C.ptr<double>(i)[j];
		if (cost < minCost && M.ptr(i)[j] == 1) minCost = cost;
	}
	return minCost;
}

//sets the matrix M where the cost in matrix C is equal to min_cost
void 
EventTracker::set_minimum_matches_per_component(Mat & M, const Mat & C, int j, double min_cost)
{
	if (min_cost > absoluteMinCost)
		for (int i=0; i<M.rows; i++) M.ptr(i)[j] = 0;
	else
	{
		for (int i=0; i<M.rows; i++)
			if (C.ptr<double>(i)[j] != min_cost) M.ptr(i)[j] = 0;
			else min_cost = -2; // the rest of the elements are going to be set to 0
	}
}

//extract connected components into components vector
void 
EventTracker::retrieve_image_components()
{
	backSeg.nextImage(grayImage);
	cv::Mat& binImage = backSeg.getBinaryImage();
	getComponents(binImage, grayImage, features);
	merge_overlapped_components();
}

void 
EventTracker::merge_overlapped_components()
{
	/*this process will create a bunch of NULL pointers in the list*/
	for (std::vector<BlobFeature>::iterator iter = features.begin(); iter != features.end(); iter++)
		merge_feature(features, iter);

	/*remove the NULL pointers in the list*/
	vector<BlobFeature> merged_features;
	for (std::vector<BlobFeature>::iterator f = features.begin(), fEnd = features.end(); f!=fEnd; f++)
	{
		if (*f)
			merged_features.push_back(*f);
	}

	features = merged_features;
}


/*this function assumes that the iterator is associated with the vector passed here*/
/*
  New updates by Vafa (Aug 15th):
  We dont start from the begining of the vector everytime,
  the iterator is passed and we can start from the
  next element in the list
 */
void 
EventTracker::merge_feature(vector<BlobFeature>& list_to_merge,
                                 std::vector<BlobFeature>::iterator iter)
{
	BlobFeature f  = *iter;
	if (f)
	{
		for (vector<BlobFeature>::iterator i = iter+1; i!=list_to_merge.end(); i++)
		{
			BlobFeature g = *i;
			if (g && features_overlap(f,g,0.5))
			{
				mergeFeatures(f,g);
				destroyBlobFeature(&g);
				*i = NULL;
			}
		}
	}
}

bool 
EventTracker::features_overlap(BlobFeature f, BlobFeature g, float threshold)
{
	CvPoint f_br = bottomRight(f);
	CvPoint g_br = bottomRight(g);
	CvRect f_rect = rect(f);
	CvRect g_rect = rect(g);

	int A1 = area(f);
	int A2 = area(g);

	int x1,x2;
	if (f_rect.x < g_rect.x)
	{
		x1 = g_rect.x;
		x2 = f_br.x;
	}
	else
	{
		x1 = f_rect.x;
		x2 = g_br.x;
	}

	int Lx = x2 - x1;
	if (Lx < 0) return false;

	int y1,y2;
	if (f_rect.y < g_rect.y)
	{
		y1 = g_rect.y;
		y2 = f_br.y;

	}
	else
	{
		y1 = f_rect.y;
		y2 = g_br.y;
	}

	int Ly = y2-y1;
	if (Ly < 0) return false;

	float A_overlap = (float)Lx*(float)Ly;

	float A_min;
	if (A1 < A2) A_min = (float)A1;
	else A_min = (float)A2;

	return (A_min > 0 && (A_overlap/A_min) > threshold);
}


void 
EventTracker::reset()
{
	traces.clear();
	targets.clear();
	backSeg.reset();
	frame_number = 0;
}

void 
EventTracker::perform_kalman_filtering()
{
	//For each trace being tracked, perform kalman filtering
	for (std::vector<TracePtr>::iterator t = traces.begin(), tEnd = traces.end(); t!=tEnd; t++)
		(*t)->Perform_Kalman_Filtering();
}

void 
EventTracker::predict_occluded_target_position()
{
	//For each target thought to be occluded, predict its position
	TargetPtr T;
	for (vector<TargetPtr>::iterator t = targets.begin(); t!=targets.end(); t++)
	{
		T = (*t);
		if (!target_is_being_tracked(T) && target_is_recent(T))
			T->Evolve();
	}
}

void 
EventTracker::match_traces_with_targets()
{
	vector<TracePtr> traces_for_matching = get_traces_for_matching();                 //Get traces that require a match
	vector<TargetPtr> targets_for_matching = get_targets_for_matching();              //Get targets that require a match

	if (traces_for_matching.size() > 0)
	{
		if (targets_for_matching.size() > 0)
		{
			Mat M;

			//Get the similarity matrix for the traces and targets that require a match
			get_target_cost_matrix(M, traces_for_matching, targets_for_matching);

			for (unsigned int i=0; i<M.rows; i++)          // for each trace
			{
				unsigned int j;
				if (trace_matches_a_target(i, j, M))        // if the trace has a matching target
				{
					targets_for_matching[j]->add_trace(traces_for_matching[i]);
					traces_for_matching[i].reset();
				}

			}
		}

		create_new_targets_from_novel_traces(traces_for_matching);
	}
}

void
EventTracker::create_new_targets_from_novel_traces(vector<TracePtr> traces_for_matching)
{
	int novel_length = transientTime + matchTime;
	TargetPtr target;

	for (vector<TracePtr>::iterator t = traces_for_matching.begin(); t != traces_for_matching.end() ; t++)
	{
		if (*t && (*t)->get_length() > novel_length)
		{
			file_must_be_kept = true;
			target.reset(new Target);
			target->add_trace(*t);
			targets.push_back(target);
		}
	}
}

static void
minMaxLoc_CV_64FC1(const cv::Mat& mat,
                               double* min_val,
                               double* max_val,
                               Point* minLoc)
{
	CV_Assert(mat.type() == CV_64FC1);
	if (minLoc)
	{
		minLoc->x = -1;
		minLoc->y = -1;
	}

	double _min_val;
	if (!min_val)
		min_val = &_min_val;
	double* m = (double*) mat.data;

	for (int i=0; i< mat.cols; i++)
		for (int j=0; j<mat.rows; j++, m++)
		{
			double currentValue = *m;
			CV_Assert(currentValue >= 0);
			if (i == 0 && j == 0)
			{
				*min_val = currentValue;
				if (max_val)
					*max_val = currentValue;
				if (minLoc)
					minLoc->x = minLoc->y = 0;
			}
			else
			{
				if (max_val && currentValue > *max_val)
					*max_val = currentValue;
				if (currentValue < *min_val)
				{
					*min_val = currentValue;
					if (minLoc)
					{
						minLoc->x = i;
						minLoc->y = j;
					}
				}
			}
		}
}

bool 
EventTracker::trace_matches_a_target(unsigned & trace_index, unsigned & target_index, const Mat& M)
{
	//returns true if the trace with index 'trace_index' matches a target and
	//returns the matching target's index: 'target_index'

	double row_min, col_min;
	Point minLoc;

	minMaxLoc_CV_64FC1(M.row(trace_index), &row_min, 0, &minLoc);
	target_index = minLoc.x;
	minMaxLoc_CV_64FC1(M.col(target_index), &col_min, 0, 0);

	if (row_min == col_min)
		return (row_min < absoluteMinCost);
	else
		return false;
}

void 
EventTracker::get_target_cost_matrix(Mat & M,
                                          vector<TracePtr> traces_for_matching,
                                          vector<TargetPtr> targets_for_matching)
{
	M.create(traces_for_matching.size(), targets_for_matching.size(), CV_64FC1);
	double *m = (double*)M.data;

	TargetPtr T;
	TracePtr F;
	TracePtr G;
	BlobFeature f = createBlob();      //Blob feature associated with trace F
	BlobFeature g = createBlob();      //Blob feature associated with trace G

	for (vector<TracePtr>::iterator tr = traces_for_matching.begin(); tr != traces_for_matching.end(); tr++)
	{
		F = *tr;
		F->get_kalman_state_post(f);

		//For each target, get the similarity to trace tr
		for (vector<TargetPtr>::iterator tg = targets_for_matching.begin(); tg != targets_for_matching.end(); tg++, m++)
		{
			T = (*tg);
			G = T->get_last_trace();

			// if the dormant trace has not been dormant long, match using the expected position and velocity
			if (target_is_recent(T))
			{
				G->get_kalman_state_post(g);
				*m = (trace_motion_is_similar(F,G)) ? cost_value(f,g) : DBL_MAX;
			}
			else        // match using the object's appearance only - not robust, needs revision
				*m = DBL_MAX;

			CV_Assert (*m>=0);
		}
	}
	destroyBlobFeature(&f);
	destroyBlobFeature(&g);
}


vector<TracePtr> 
EventTracker::get_traces_for_matching()
{
	vector<TracePtr> traces_for_matching;
	TracePtr tr;
	for (vector<TracePtr>::iterator t = traces.begin(); t != traces.end(); t++)    // for every trace
	{
		tr = (*t);
		if (!tr->is_matched)                //if the trace is not yet matched to a target
		{
			if (tr->get_length() > (int)transientTime)                //if the trace is non-transient and the Kalman Filters have been running for a few frames
			{
				traces_for_matching.push_back(tr);
			}
		}
	}
	return traces_for_matching;
}

vector<TargetPtr> 
EventTracker::get_targets_for_matching()
{
	vector<TargetPtr> targets_for_matching;
	TargetPtr T;

	for (vector<TargetPtr>::iterator t = targets.begin(); t != targets.end();  t++)
	{
		T = (*t);
		if (!target_is_being_tracked(T))        // if not already being tracked, i.e. the target is dormant
		{
			targets_for_matching.push_back(T);
		}
	}
	return targets_for_matching;
}

bool 
EventTracker::target_is_being_tracked(TargetPtr T)
{
	return (T->get_last_frame() == frame_number);
}

bool 
EventTracker::target_is_recent(TargetPtr T)
{
	int time_elapsed = frame_number - T->get_last_frame();          //the time since the target was last seen
	return (time_elapsed < (int)dormantTime);
}

bool 
EventTracker::trace_motion_is_similar(TracePtr t1, TracePtr t2)
{
	Point tmp = t1->ff.centroid.State(0) - t2->ff.centroid.State(0);
	double del_p = sqrt((double)tmp.dot(tmp));    //difference in position
	tmp = t1->ff.centroid.State(1) - t2->ff.centroid.State(1);
	double del_s = sqrt((double)tmp.dot(tmp));    //difference in velocity
	tmp = t1->ff.centroid.State(1);
	double s1 = sqrt((double)tmp.dot(tmp));      //the speed of trace 1
	tmp = t2->ff.centroid.State(1);
	double s2 = sqrt((double)tmp.dot(tmp));      //the speed of trace 2
	double s_max = (s1>s2) ? s1:s2;

	bool velocity_is_similar;
	if (s_max > 0)
	{
		velocity_is_similar = (del_s/s_max < 1);
	}
	else
	{
		velocity_is_similar = true;
	}
	bool position_is_similar = (del_p < 100);

	return position_is_similar && velocity_is_similar;
}



vector<TracePtr> 
EventTracker::get_unmatched_traces()
{
	vector<TracePtr> unmatched_traces;
	TracePtr tr;
	for (vector<TracePtr>::iterator t = traces.begin(); t != traces.end(); t++)    // for every trace
	{
		tr = (*t);
		if (!tr->is_matched)                //if the trace is not yet matched to a target
		{
			unmatched_traces.push_back(tr);
		}
	}
	return unmatched_traces;
}


vector<BlobFeature> 
EventTracker::get_features() 
{ 
	return features; 
}


void 
EventTracker::process_inactive_targets()
{
	vector<TargetPtr> new_targets;
	vector<TargetPtr>* db_targets = new vector<TargetPtr>();

	for (vector<TargetPtr>::iterator t = targets.begin(); t != targets.end(); t++)
	{
		if (target_is_recent(*t))
			new_targets.push_back(*t);
		else
			db_targets->push_back(*t);
	}
	targets = new_targets;
	if (db_targets->size())
		dispatcher->send_signal("controller", DB_INSERT_TARGET, db_targets);
	else
		delete db_targets;
}


void
EventTracker::drawTrace(TracePtr trace, QImage* img)
{
	CvScalar _color = trace->get_color();
	QColor color(_color.val[0],_color.val[1],_color.val[2]);
	QBrush brush(color);
	QPainter painter(img);
	painter.setPen(color);
	painter.setBrush(brush);

	for (std::vector<BlobFeature>::iterator blobIter = trace->nodeBegin()+1; blobIter != trace->nodeEnd(); blobIter++)
	{
		BlobFeature prevBlob = * (blobIter - 1);
		BlobFeature currentBlob = * (blobIter);
		CvPoint prevCent = centroid(prevBlob);
		CvPoint currentCent = centroid(currentBlob);
		painter.drawLine(QPoint(prevCent.x, prevCent.y), QPoint(currentCent.x, currentCent.y));
	}

}

void 
EventTracker::drawTarget(TargetPtr target, QImage* img)
{
	int trace_count = target->get_number_of_traces();
	for (int i = 0; i < trace_count; i++)
	{
		TracePtr trace = target->get_trace(i);
		drawTrace(trace, img);
	}
}

/*this function draws bounding boxes, traces, targets as required*/
void 
EventTracker::drawOnImages(QImage* img)
{
	if (b_draw_bounding_boxes)
	{
		for (std::vector<BlobFeature>::iterator iter = features.begin(); iter != features.end(); iter++)
			cv::rectangle(backSeg.getBinaryImage(), topLeft(*iter), bottomRight(*iter), cvScalar(255,255,255));
	}

	if (b_draw_traces)
		for (std::vector<TracePtr>::iterator iter = traces.begin(); iter != traces.end(); iter++)
			drawTrace(*iter, img);

	if (b_draw_targets)
	{
		for (std::vector<TargetPtr>::iterator iter = targets.begin(); iter != targets.end(); iter++)
			drawTarget(*iter, img);
	}
}

void 
EventTracker::finish_recording()
{
	if (videoWriter)
		delete videoWriter;
	if (backgroundVideoWriter)
		delete backgroundVideoWriter;
	videoWriter = NULL;
	backgroundVideoWriter = NULL;
	char timestring [100];
	getCurrentTimeString(timestring, 100);
	dispatcher->send_signal("controller", 
			DB_INSERT_VIDEO_SEQUENCE,
		       	g_strdup_printf("%s, '%s', %d", file_sequence_str, timestring, frame_number));
	g_free(file_sequence_str);
}

void 
EventTracker::createNewVideoFile(cv::Size size)
{
	blob_feature_output_stats();
	gettimeofday(&recorder_restart_ts, NULL);
	char* filename, *bfilename, *_bfilename;
	char timestring[100];
	g_free(_filename);
	getCurrentTimeString(timestring, 100);
	filename = g_strdup_printf("%d_%s-%d.avi", getpid(), timestring, frame_number);
	bfilename = g_strdup_printf("%d_%s-%d_bg.avi", getpid(), timestring, frame_number);
	file_sequence_str = g_strdup_printf("'%s', '%s', %d", filename, timestring, frame_number); 
		
	_filename = g_strdup_printf("%s/%s", prefix, filename);
	_bfilename = g_strdup_printf("%s/%s", prefix, bfilename);
	try
	{
		videoWriter = new VideoWriter(_filename, CV_FOURCC('D','I','V','X'), 30, size, true);
		backgroundVideoWriter = new VideoWriter(_bfilename, CV_FOURCC('D','I','V','X'), 30, size, true);
	}
	catch (cv::Exception& e)
	{
		if (videoWriter)
			delete videoWriter;
		if (backgroundVideoWriter)
			delete backgroundVideoWriter;
		backgroundVideoWriter = NULL;
		videoWriter = NULL;
	}
	g_free(filename);
	g_free(bfilename);
	g_free(_bfilename);
}


void 
EventTracker::set_param(const std::string& var, void* param)
{
	if (!var.compare("prefix"))
	{
		free(prefix);
		prefix = strdup ((char*) param);
	}
	else if (!var.compare("recorder-switch"))
	{
		b_record = (*((bool*) param));
	}
}

