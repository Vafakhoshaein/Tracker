#include "common.h"
int trace_flag;
int target_flag;


void trace_flag_add(){trace_flag++;}
void target_flag_add(){target_flag++;}
void trace_flag_zero(){trace_flag = 0;}
void target_flag_zero(){target_flag = 0;}
int trace_flag_get(){return trace_flag;}
int target_flag_get(){return target_flag;}

void append_int(QString* text, int num)
{
	QString return_string;
	char array[10] = {0};
	sprintf(array, "%d", num);
	text->append(array);
}


template <class T>
QList<T> list_intersection(QList<QList<T>*>* lists)
{
	//Create a list consisting of the elements each list has in common

	//Begin with the first list
	QList<T> common_list = *(lists->first());
	QList<T>* temp_list;

	//eleminate from the common list the elements that are not in each additional list
	//for (QList<T>::iterator list_iter = lists->begin(); list_iter != lists->end(); list_iter++)
	for (int i=0; i<lists->count(); i++)
	{
		temp_list = lists[i];
		//For each element in the common list
		//for (QList<T>::iterator T_iter = common_list.begin(); T_iter != common_list.end(); T_iter++)
		for (int j=0; j<common_list.count(); j++)
		{
			//if the element does not appear in the other list, remove it from the common list
			if (!temp_list->contains(common_list[j]))
			{
				common_list.removeAt(j);
				j--;
			}
		}
	}

	return common_list;
}


unsigned char rand_uchar()
{
	return (unsigned char) floor( ((float)rand() / (float) RAND_MAX) * 256.0f );
}

int rand_int( int min_value, int max_value)
{
	double s = (double)rand() / ((double) RAND_MAX + 0.0001);     // random double between 0 and 0.999..., or [0,1)
	double d = (double) (max_value - min_value + 1);                     // the desired range of values
	return  (int) floor(s*d) + min_value;
}

void log_clear()
{
	remove("logfile.txt");
}

void log_text(const char* fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);

	FILE* fp  = fopen("logfile.txt", "a+");

	vfprintf(fp, fmt, ap);
	fclose(fp);

	va_end(ap);
}

void logMatrix(const char* title, Mat & m)
{
	QString line;
	Mat2Str(title, m, line);
	log_text(line.toAscii().constData());
}


void Mat2Str(const char* title, Mat & m, QString &  line)
{
	line.clear();
	line.append(title);
	line.append("\t");
	line.append(QString::number(m.rows));
	line.append("\t");
	line.append(QString::number(m.cols));
	line.append("\t");
	line.append(QString::number(m.type()));
	line.append("\n");

	if (m.type() == CV_8UC1)
	{
		for (int i=0; i<m.rows; i++)
		{
			unsigned char* p = m.ptr<unsigned char>(i);
			for (int j=0; j<m.cols; j++)
			{
				line.append(QString::number((int)p[j]));
				line.append("\t");
			}
			line.append("\n");
		}
	}
	else if (m.type() == CV_32FC1)
	{
		for (int i=0; i<m.rows; i++)
		{
			float* p = m.ptr<float>(i);
			for (int j=0; j<m.cols; j++)
			{
				line.append(QString::number(p[j]));
				line.append("\t");
			}
			line.append("\n");
		}
	}
	else if (m.type() == CV_32SC1)
	{
		for (int i=0; i<m.rows; i++)
		{
			int* p = m.ptr<int>(i);
			for (int j=0; j<m.cols; j++)
			{
				line.append(QString::number(p[j]));
				line.append("\t");
			}
			line.append("\n");
		}
	}
	else if (m.type() == CV_64FC1)
	{
		for (int i=0; i<m.rows; i++)
		{
			double* p = m.ptr<double>(i);
			for (int j=0; j<m.cols; j++)
			{
				line.append(QString::number(p[j]));
				line.append("\t");
			}
			line.append("\n");
		}
	}
	else if (m.type() == CV_32FC2)
	{
		for (int i=0; i<m.rows; i++)
		{
			float* p = m.ptr<float>(i);
			for (int j=0; j<m.cols; j++)
			{
				line.append("[");
				line.append(QString::number(*p));
				p++;
				line.append(", ");
				line.append(QString::number(*p));
				p++;
				line.append("]\t");
			}
			line.append("\n");
		}
	}
	else if (m.type() == CV_32FC3)
	{
		for (int i=0; i<m.rows; i++)
		{
			float* p = m.ptr<float>(i);
			for (int j=0; j<m.cols; j++)
			{
				line.append("[");
				line.append(QString::number(*p));
				p++;
				line.append(", ");
				line.append(QString::number(*p));
				p++;
				line.append(", ");
				line.append(QString::number(*p));
				p++;
				line.append("]\t");
			}
			line.append("\n");
		}
	}
}



bool
/*Idea:
  Test the projection of b1 and b2 on both X and Y axis:
  Boxes will overlap if and only if the projection of b1 and b2 intersect
  on both X and Y axis
  */
boxes_overlap(CvRect b1, CvRect b2)
{

	CvRect* top = (b1.y <= b2.y) ? &b1 : &b2;
	CvRect* bottom = (top == &b1) ? &b2 : &b1;
	CvRect* left = (b1.x <= b2.x) ? &b1 : &b2;
	CvRect* right = (left == &b1) ? &b2 : &b1;

	return (bottom->y <= top->y+top->height) && (right->x <= left->x + left->width);
}

/******************************************
 * convert qimagei 2 iplimage 
 ******************************************/
cv::Mat* 
QImage2IplImage(QImage *img)
{
	*img = img->convertToFormat(QImage::Format_RGB888);
	cv::Mat* image = new cv::Mat(img->height(), img->width(), CV_8UC3);

	int n = img->width() * img->height();
	uchar* bits = img->bits();
	for (int i = 0; i < n; i++)
	{
		(image->data)[i*3 + 0] = bits[i*3 + 2];
		(image->data)[i*3 + 1] = bits[i*3 + 1];
		(image->data)[i*3 + 2] = bits[i*3 + 0];
	}
	return image;
}

/******************************************
 * convert iplimage 2 qimage
 ******************************************/
QImage*  
IplImage2QImage(IplImage *iplImg)
{
	int h = iplImg->height;
	int w = iplImg->width;
	int channels = iplImg->nChannels;
	QImage *qimg = new QImage(w, h, QImage::Format_ARGB32);
	char *data = iplImg->imageData;

	for (int y = 0; y < h; y++, data += iplImg->widthStep)
	{
		for (int x = 0; x < w; x++)
		{
			char r, g, b, a = 0;
			if (channels == 1)
			{
				r = data[x * channels];
				g = data[x * channels];
				b = data[x * channels];
			}
			else if (channels == 3 || channels == 4)
			{
				r = data[x * channels + 2];
				g = data[x * channels + 1];
				b = data[x * channels];
			}

			if (channels == 4)
			{
				a = data[x * channels + 3];
				qimg->setPixel(x, y, qRgba(r, g, b, a));
			}
			else
			{
				qimg->setPixel(x, y, qRgb(r, g, b));
			}
		}
	}
	return qimg;
}



void 	 getCurrentTimeString(char* buf, int size)
{
	time_t rawtime;
	struct tm * timeinfo;
	time ( &rawtime );
	timeinfo = localtime ( &rawtime );
	strftime (buf,size,"%Y-%m-%d %H:%M:%S",timeinfo);
}

