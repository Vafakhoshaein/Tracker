#include "trace.h"
#include <opencv/highgui.h>

#define TIME_STEP 1


Trace::Trace(int init_frame):
	id(id_generator),
	initial_frame(init_frame),
	length(0),
	mobile(false)
{
	startTime = QDateTime::currentDateTime();
	expected_feature = createBlob();
	endTime = startTime;
	id_generator++;
	is_matched = false;
	kalman_filter_is_initialized = false;
	color = cvScalar(rand() % 255, rand() % 255, rand() % 255);
	x.min = INT_MAX;
	x.max = 0;
	y.min = INT_MAX;
	y.max = 0;
}

/*************************************************************************************************
* Vafa
* Date:          Aug 2011
* Comment Added: We assume a blob is close to this trace iff the blob overlaps
*                the bounding bounding box of the expected feature of this trace
*************************************************************************************************/
bool
Trace::is_blob_close(const BlobFeature blob)
{
	return boxes_overlap(get_matching_region(),
			rect(blob));

}

CvRect
Trace::get_matching_region()
/*************************************************************************************************
* Vafa.
* Fixes applied, a matching region is a bounding box conceptually therefore
* the proper way of defining it is using a CvRect
*************************************************************************************************/
{
	CvPoint tl = topLeft(expected_feature);
	CvPoint br = bottomRight(expected_feature);

	return cvRect(tl.x - DIALATE_FACTOR,
			tl.y - DIALATE_FACTOR,
			(br.x + DIALATE_FACTOR) - (tl.x - DIALATE_FACTOR),
			(br.y + DIALATE_FACTOR) - (tl.y - DIALATE_FACTOR));
}


int Trace::get_id() {return id;}
int Trace::get_initial_frame(){return initial_frame;}
int Trace::get_length(){return length;}
int Trace::get_last_frame(){return initial_frame + length - 1;}
//int Trace::get_total_expected_frames(){return total_expected_frames;}
//int Trace::get_recent_expected_frames(){return recent_expected_frames;}
CvScalar Trace::get_color(){return color;}

Trace::~Trace()
{
	if (length >= KALMAN_START_LENGTH)      // if the filters have been created
	{
		cvReleaseKalman(&ff.centroid.x);
		cvReleaseKalman(&ff.centroid.y);
		cvReleaseKalman(&ff.size);
		cvReleaseKalman(&ff.height);
		cvReleaseKalman(&ff.width);
		for (int i=0; i<16; i++)
			cvReleaseKalman(&ff.color_histogram[i]);
	}

	destroyBlobFeature(&expected_feature);
	for (std::vector<BlobFeature>::iterator iter = nodes.begin();iter != nodes.end(); iter++)
	{
		BlobFeature b = *iter;
		destroyBlobFeature(&b);
	}
	for (std::vector<BlobFeature>::iterator iter = filtered_nodes.begin();iter != filtered_nodes.end(); iter++)
	{
		BlobFeature b = *iter;
		destroyBlobFeature(&b);
	}
}


void  Trace::next_frame(const BlobFeature feature)
{
	/*keep a copy of the BlobFeature*/
	nodes.push_back(createBlob(feature));
	endTime = QDateTime::currentDateTime();
	length++;

	if (length == KALMAN_START_LENGTH)
		Initialize_Kalman_Filter();

}

void Trace::Evolve()
{
	//void Evolve(CvKalman* KF, CvMat*z);     //currently not needed
	ff.centroid.Evolve();
}

void Evolve(CvKalman* KF, CvMat*z)
{
	// Evolves the Kalman states using the predicted state as the measurement
	cvKalmanPredict(KF, NULL);
	cvmSet(z,0,0,cvmGet(KF->state_pre,0,0));
	cvKalmanCorrect(KF, z);
}

void Trace::get_kalman_state_post(BlobFeature f)
{
	setSize(f, (int)cvmGet(ff.size->state_post,0,0));
	setWidth(f, (int)cvmGet(ff.width->state_post,0,0));
	setHeight(f, (int)cvmGet(ff.height->state_post,0,0));
	int color_histogram[16];
	for (int i=0; i<16; i++)
		color_histogram[i] = (int)cvmGet(ff.color_histogram[i]->state_post,0,0);
	setColorHistogram(f, color_histogram);
	setCentroid(f, ff.centroid.State(0));
}

void Trace::get_kalman_state_pre(BlobFeature f)
{
	setCentroid(f, cvPoint ((int)cvmGet(ff.centroid.x->state_pre,0,0),
				(int)cvmGet(ff.centroid.y->state_pre,0,0)));
	setSize(f, (int)cvmGet(ff.size->state_pre,0,0));
	setWidth(f, (int)cvmGet(ff.width->state_pre,0,0));
	setHeight(f, (int)cvmGet(ff.height->state_pre,0,0));

	int color_histogram[16];
	for (int i=0; i<16; i++)
		color_histogram[i] = (int)cvmGet(ff.color_histogram[i]->state_pre,0,0);
	setColorHistogram(f, color_histogram);
}

unsigned int Trace::id_generator = 1;

void Trace::Predict_Kalman_Filter()
{
	//Centroid
	cvKalmanPredict(ff.centroid.x, NULL);
	cvKalmanPredict(ff.centroid.y, NULL);

	//Area
	cvKalmanPredict(ff.size, NULL);

	//Width
	cvKalmanPredict(ff.width, NULL);

	//Height
	cvKalmanPredict(ff.height, NULL);

	//Color histogram
	for (int i=0; i<16; i++)
	{
		cvKalmanPredict(ff.color_histogram[i], NULL);
	}
}

void Trace::Correct_Kalman_Filter(BlobFeature m)
{
	CvMat* z = cvCreateMat(1,1,CV_32FC1);
	CvPoint m_centroid=centroid(m);
	//Centroid
	cvmSet(z,0,0,m_centroid.x);
	cvKalmanCorrect(ff.centroid.x, z);

	cvmSet(z,0,0,m_centroid.y);
	cvKalmanCorrect(ff.centroid.y, z);

	//Size
	cvmSet(z,0,0,getSize(m));
	cvKalmanCorrect(ff.size, z);

	CvRect r = rect(m);
	//Width
	cvmSet(z,0,0,r.width);
	cvKalmanCorrect(ff.width, z);

	//Height
	cvmSet(z,0,0,r.height);
	cvKalmanCorrect(ff.height, z);

	const int* color_histogram = getColorHistogram(m);

	//Color histogram
	for (int i=0; i<16; i++)
	{
		cvmSet(z,0,0,color_histogram[i]);
		cvKalmanCorrect(ff.color_histogram[i], z);
	}
	cvReleaseMat(&z);
}


void Trace::Perform_Kalman_Filtering()
{
	//This routine performs the kalman filtering for the trace.
	//It is also used to store the predicted states as the traces 'expected_feature'
	//and to store the filtered features in the vector 'filtered_nodes'.

	BlobFeature f = createBlob();
	if (kalman_filter_is_initialized)
	{
		// Predict the kalman filter states
		Predict_Kalman_Filter();

		//correct the kalman filter states using the latest blob features
		Correct_Kalman_Filter(get_last_feature());

		//set the expected features based on the predicted kalman filter states
		get_kalman_state_pre(expected_feature);

		//set the filtered feature to the corrected kalman state
		get_kalman_state_post(f);
	}
	else
	{
		//set the expected features based on the previous feature
		setEqual (expected_feature, get_last_feature());

		//set the filtered feature to the measured feature until the kalman filter starts running
		setEqual(f, get_last_feature());
	}

	//store the corrected filtered states
	filtered_nodes.push_back(f);

	//Get the range of the trace
	CvPoint f_centroid = centroid(f);
	if (x.min > f_centroid.x) x.min = f_centroid.x;
	if (x.max < f_centroid.x) x.max = f_centroid.x;
	if (y.min > f_centroid.y) y.min = f_centroid.y;
	if (y.max < f_centroid.y) y.max = f_centroid.y;


	setXY(expected_feature, (int)(centX(expected_feature) - 0.5 * getWidth(expected_feature)),
			(int)(centY(expected_feature) - 0.5 * getHeight(expected_feature)));

}

void Trace::Initialize_Kalman_Filter()
{
	Point position;
	Point velocity;
	double s;

	BlobFeature last_feature = nodes[nodes.size()-1];

	//Allocate the Kalman filter memory
	ff.centroid.x = cvCreateKalman(2,1,0);
	ff.centroid.y = cvCreateKalman(2,1,0);
	ff.size = cvCreateKalman(1,1,0);
	ff.height = cvCreateKalman(1,1,0);
	ff.width = cvCreateKalman(1,1,0);
	for (int i=0; i<16; i++)
	{
		ff.color_histogram[i] = cvCreateKalman(1,1,0);
	}

	//CENTROID

	position = centroid(last_feature);
	//velocity = (position - nodes[nodes.size()-2]->centroid)/TIME_STEP;
	velocity.x = (position.x - centX(nodes[nodes.size()-2]))/TIME_STEP;
	velocity.y = (position.y - centY(nodes[nodes.size()-2]))/TIME_STEP;

	ff.centroid.Initialize(TIME_STEP, position, velocity);

	//AREA

	//Initialize the parameters
	cvSetIdentity(ff.size->transition_matrix, cvRealScalar(1));
	//cvmSet(ff.size->transition_matrix,0,1,TIME_STEP);
	cvSetIdentity(ff.size->measurement_matrix, cvRealScalar(1));
	cvSetIdentity(ff.size->process_noise_cov, cvRealScalar(1E-3));
	cvSetIdentity(ff.size->measurement_noise_cov, cvRealScalar(1e-1));

	//Set the initial conditions
	s = getSize(last_feature);
	cvSetIdentity(ff.size->state_post, cvRealScalar(s));
	cvSetIdentity(ff.size->error_cov_post, cvRealScalar(1));

	//HEIGHT

	//Initialize the parameters
	cvSetIdentity(ff.height->transition_matrix, cvRealScalar(1));
	cvSetIdentity(ff.height->measurement_matrix, cvRealScalar(1));
	cvSetIdentity(ff.height->process_noise_cov, cvRealScalar(1E-2));
	cvSetIdentity(ff.height->measurement_noise_cov, cvRealScalar(1E-1));

	//Set the initial conditions
	s = getHeight(last_feature);
	cvSetIdentity(ff.height->state_post, cvRealScalar(s));
	cvSetIdentity(ff.height->error_cov_post, cvRealScalar(1));

	//WIDTH

	//Initialize the parameters
	cvSetIdentity(ff.width->transition_matrix, cvRealScalar(1));
	cvSetIdentity(ff.width->measurement_matrix, cvRealScalar(1));
	cvSetIdentity(ff.width->process_noise_cov, cvRealScalar(1E-2));
	cvSetIdentity(ff.width->measurement_noise_cov, cvRealScalar(1E-1));

	//Set the initial conditions
	s = getWidth(last_feature);
	cvSetIdentity(ff.width->state_post, cvRealScalar(s));
	cvSetIdentity(ff.width->error_cov_post, cvRealScalar(1));


	//COLOR HISTOGRAM
	const int* color_histogram = getColorHistogram(last_feature);
	for (int i=0; i<16; i++)
	{
		//Initialize the parameters
		cvSetIdentity(ff.color_histogram[i]->transition_matrix, cvRealScalar(1));
		cvSetIdentity(ff.color_histogram[i]->measurement_matrix, cvRealScalar(1));
		cvSetIdentity(ff.color_histogram[i]->process_noise_cov, cvRealScalar(1E-3));
		cvSetIdentity(ff.color_histogram[i]->measurement_noise_cov, cvRealScalar(1E-1));

		//Set the initial conditions
		s = color_histogram[i];
		cvSetIdentity(ff.color_histogram[i]->state_post, cvRealScalar(s));
		cvSetIdentity(ff.color_histogram[i]->error_cov_post, cvRealScalar(1));
	}

	kalman_filter_is_initialized = true;

}

BlobFeature Trace::get_last_feature()
{
	return  (nodes.size()) ? *(nodes.end()-1) : NULL;
}

void Trace::set_color(CvScalar new_color)
{
	color = new_color;
}

bool Trace::is_mobile()
{
	if(mobile)
	{
		return true;
	}
	else
	{
		//determine mobility
		int del_x = x.max - x.min;
		int del_y = y.max - y.min;
		int del_max;
		if (del_x > del_y)
		{
			del_max = del_x;
		}
		else
		{
			del_max = del_y;
		}
		mobile = (del_max > DISPLACEMENT_THRESHOLD);
		return mobile;
	}
}

BlobFeature Trace::get_first_feature()
{
	return  (nodes.size()) ? *(nodes.begin()) : NULL;
}


//Functions involving traces and blob features
double cost_value(const BlobFeature f, const BlobFeature g)
{
	float relative_cost(int a, int b);
	float correl(const int x[], const int y[], int n);

	float size_ratio = relative_cost(getSize(f),getSize(g));
	float width_ratio = relative_cost(getWidth(f),getWidth(g));
	float height_ratio = relative_cost(getHeight(f),getHeight(g));
	float color_ratio = 1.0f - correl(getColorHistogram(f), getColorHistogram(g), 16);

	float cv = size_ratio + width_ratio + height_ratio + color_ratio;

	return (double)cv;
}

float relative_cost(int a, int b)
{
	if (a>b) 
		return (float)(a-b)/float(a);
	else 
		return (float)(b-a)/float(b);
}

float correl(const int x[], const int y[], int n)
{
	float mean(const int a[], int n);

	float xm = mean(x,n);
	float ym = mean(y,n);
	float Sxx, Syy, Sxy;
	float xt, yt;
	Sxx = Syy = Sxy = 0.0;
	for (int i=0; i<n; i++)
	{
		xt = x[i]-xm;
		yt = y[i]-ym;

		Sxx += xt*xt;
		Syy += yt*yt;
		Sxy += xt*yt;
	}
	return Sxy/sqrt(Sxx*Syy);

}

float mean(const int a[], int n)
{
	int sum = 0;
	for (int i=0; i<n; i++)
	{
		sum += a[i];
	}
	return (float)sum/(float)n;
}

bool f_contains_g(const BlobFeature f, const BlobFeature g)
{
	Point f_br = bottomRight(f);
	Point g_br = bottomRight(g);
	CvRect f_rect = rect(f);
	CvRect g_rect = rect(g);
	return (f && g &&
			(f_rect.x < g_rect.x) &&
			(f_br.x > g_br.x) &&
			(f_rect.y < g_rect.y) &&
			(f_br.y > g_br.y));
}

void Display_Histograms(int a[], int b[])
{
	//Determine the maximum value
	int max_value = 0;
	// int min_value = 0;
	//int d[16];
	for (int i=0; i<16; i++)
	{
		//d[i] = b[i]-a[i];
		//if (d[i] > max_value) max_value = d[i];
		// if (d[i] < min_value) min_value = d[i];
		if (a[i] > max_value) max_value = a[i];
		if (b[i] > max_value) max_value = b[i];
	}

	//Scale the window according to the maximum value
	int scale_res = 100;      //number of pixels we scale by
	int scale = scale_res * (1 + (max_value)/scale_res);

	int window_height = 200;
	int window_width = 400;
	int bar_width = 20;
	int bar_space = 10;

	float pixel_ratio = float(window_height)/float(scale);

	cvNamedWindow("Histogram");
	IplImage* graph = cvCreateImage(cvSize(window_width, window_height),8,3);

	cvZero(graph);

	//Draw the histograms
	int x_pos = bar_space;
	int ul_x, ul_y, br_x, br_y;

	for (int i=0; i<16; i++)
	{

		ul_x = x_pos;
		br_x = ul_x + bar_width;


		// Determine which value is largest
		if (a[i] > b[i])
		{
			// Draw the combined bar
			br_y = window_height;
			ul_y = window_height - b[i]*pixel_ratio;
			cvRectangle(graph,cvPoint(ul_x,ul_y), cvPoint(br_x,br_y),cvScalar(0,255,0),CV_FILLED);

			// Draw the bar for a
			br_y = ul_y - 1;
			ul_y = window_height - a[i]*pixel_ratio;
			cvRectangle(graph,cvPoint(ul_x,ul_y), cvPoint(br_x,br_y),cvScalar(255,0,0),CV_FILLED);

		}
		else
		{
			// Draw the combined bar
			br_y = window_height;
			ul_y = window_height - a[i]*pixel_ratio;
			cvRectangle(graph,cvPoint(ul_x,ul_y), cvPoint(br_x,br_y),cvScalar(255,0,0),CV_FILLED);

			// Draw the bar for b
			br_y = ul_y - 1;
			ul_y = window_height - b[i]*pixel_ratio;
			cvRectangle(graph,cvPoint(ul_x,ul_y), cvPoint(br_x,br_y),cvScalar(0,255,0),CV_FILLED);
		}


		x_pos += bar_space + bar_width;
	}
	cvShowImage("Histogram", graph);
	cvWaitKey(10);

	cvReleaseImage(&graph);

}




/***** TARGET *******************************************************************/

unsigned int Target::id_generator = 1;

Target::Target():mobile(false)
{
	id = id_generator;
	id_generator++;
}

unsigned Target::get_id()
{
	return id;
}

int Target::get_number_of_traces()
{
	return traces.size();
}

TracePtr Target::get_trace(int index)
{
	return *(traces.begin() + index);
}

TracePtr Target::get_last_trace()
{
	return *(traces.end() - 1);
}

void Target::add_trace(TracePtr new_trace)
{
	traces.push_back(new_trace);
	new_trace->is_matched = true;

	if (get_number_of_traces() > 1)
		new_trace->set_color(get_trace(0)->get_color());
}

int Target::get_initial_frame()
{
	TracePtr tr = *(traces.begin());
	return tr->get_initial_frame();
}

int Target::get_last_frame()
{
	TracePtr tr = get_last_trace();
	return tr->get_last_frame();
}

/*void Target::draw(IplImage* image)
  {
  for (auto t = traces.begin(), tEnd = traces.end(); t!=tEnd; t++)
  {
  (*t)->draw(image);
  }

//For the last trace, get the last blob feature's centroid
TracePtr last_trace = get_last_trace();
Point centroid = last_trace->get_expected_feature().centroid;

//Display the target's id at the centroid
QString text;
append_int(&text,id);
CvFont font;
cvInitFont(&font,CV_FONT_HERSHEY_COMPLEX, 0.8,0.8,0,2);
CvScalar color = last_trace->get_color();
CvPoint _p = centroid.operator CvPoint();
Pair _p_pair;
_p_pair.x = _p.x;
_p_pair.y = _p.y;
add_text(image, text.toAscii().constData(), &_p_pair, &font, color, PLCMNT_CENTER);

}
*/

void Target::Evolve()
{
	get_last_trace()->Evolve();
}

bool Target::is_mobile()
{
	if (mobile)
		return true;
	else
	{
		//Determine Mobility
		for (vector<TracePtr>::iterator t = traces.begin(); t != traces.end(); t++)
		{
			if ((*t)->is_mobile())
			{
				mobile = true;
				return true;
			}
		}
		return false;
	}

}

QDateTime Target::getStartTime()
{
	return (*((TracePtr)(traces.at(0)))).getStartTime();
}

QDateTime Target::getEndTime()
{
	return (*((TracePtr)(traces.at(traces.size()-1)))).getEndTime();
}

