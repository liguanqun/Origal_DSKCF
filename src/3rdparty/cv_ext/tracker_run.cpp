#include "tracker_run.hpp"

#include <iostream>
#include <string>
#include <ctype.h>
#include <math_helper.hpp>
#include <numeric>
#include <fstream>
#include "init_box_selector.hpp"
#include "cf_tracker.hpp"
#include "dskcf_tracker.hpp"

using namespace cv;
using namespace std;
using namespace TCLAP;

#define SCALE 2

TrackerRun::TrackerRun(string windowTitle) :
		_cmd(_windowTitle.c_str(), ' ', "0.1")
{
	_isPaused = false;
	_isStep = false;
	_exit = false;
	_hasInitBox = false;
	_isTrackerInitialzed = false;
	_targetOnFrame = false;
	_updateAtPos = false;
	_windowTitle = windowTitle;
	_imageIndex = 0;
	_debug = 0;
	_tracker = 0;
	//	_paras = Parameters();
	_frameIdx = 1;

	_overlap_sum = 0;
}

TrackerRun::~TrackerRun()
{

	if (_tracker)
	{
		delete _tracker;
		_tracker = 0;
	}

	std::cout << "Frame,Time" << std::endl;

	for (int i = 0; i < (int) this->frameTime.size(); i++)
	{
		std::cout << i << "," << this->frameTime[i] << std::endl;
	}

	std::cout << "min,"<< *std::min_element(this->frameTime.begin(), this->frameTime.end())<< std::endl;
	std::cout << "min,"<< *std::max_element(this->frameTime.begin(), this->frameTime.end())<< std::endl;
	std::cout << "mean,"<< std::accumulate(this->frameTime.begin(), this->frameTime.end(),0.0) / static_cast<double>(this->frameTime.size())<< std::endl;
}

bool TrackerRun::start(int argc, const char** argv)
{

	_tracker = new DskcfTracker();

	this->init();
	while (this->run())
	{
		/*				if (!init())
		 {
		 return false;
		 }
		 if(!run())
		 {
		 return false;
		 }*/

		_isTrackerInitialzed = false;
	}

	return false;
}

bool TrackerRun::init()
{
//读取RGB和depth图像
	_cap.Init();

	namedWindow(_windowTitle.c_str());

	_boundingBox = _cap.Get_Init_Rect();
	_hasInitBox = true;

	std::cout << "_hasInitBox is init as " << _hasInitBox << std::endl;

	_frameIdx = 1;
	return true;
}

bool TrackerRun::run()
{

	while (true)
	{
		bool success = update();
		if (!success)
		{
			break;
		}
	}

	return true;
}

bool TrackerRun::update()
{
	int64 tStart = 0;
	int64 tDuration = 0;

	if (_frameIdx == 1)
	{

		_image[0] = _cap.Get_first_RGB();
		_image[1] = _cap.Get_Depth_Image_same_time_to_RGB();
		if (_image[0].empty() || _image[1].empty())
		{
			return false;
		}

	}
	else
	{
		_image[0] = _cap.Get_Next_RGB();
		_image[1] = _cap.Get_Depth_Image_same_time_to_RGB();
		if (_image[0].empty() || _image[1].empty())
		{
			return false;
		}

	}

	if (_frameIdx == 1)
	{
		if (!_hasInitBox)
		{
			std::cout << "no init object Rect" << std::endl;
			return false;
		}

		tStart = getTickCount();

		//把原图像缩小四倍  减少数据量
		std::array<cv::Mat, 2> resized;
		cv::resize(_image[0], resized[0],cv::Size_<double>(_image[0].cols / SCALE,_image[0].rows / SCALE), 0, 0, cv::INTER_LINEAR);
		cv::resize(_image[1], resized[1],cv::Size_<double>(_image[1].cols / SCALE,_image[1].rows / SCALE), 0, 0, cv::INTER_NEAREST);
		cv::Rect_<double> r(_boundingBox.x / SCALE, _boundingBox.y / SCALE,_boundingBox.width / SCALE, _boundingBox.height / SCALE);

		_targetOnFrame = _tracker->reinit(resized, r);

		tDuration = getTickCount() - tStart;

	}
	else if (_frameIdx > 1)
	{
		tStart = getTickCount();

		std::array<cv::Mat, 2> resized;
		cv::resize(_image[0], resized[0],cv::Size_<double>(_image[0].cols / SCALE,_image[0].rows / SCALE), 0, 0, cv::INTER_LINEAR);
		cv::resize(_image[1], resized[1],cv::Size_<double>(_image[1].cols / SCALE,_image[1].rows / SCALE), 0, 0, cv::INTER_NEAREST);

		cv::Rect_<double> r(_boundingBox.x / SCALE, _boundingBox.y / SCALE,_boundingBox.width / SCALE, _boundingBox.height / SCALE);

		//Tracker Running HERE
		std::vector<int64> singleFrameTiming(8);
		//little value
		for (int i = 0; i < (int) singleFrameTiming.size() - 1; i++)
		{
			singleFrameTiming[i] = i + 1;
		}

		_targetOnFrame = _tracker->update(resized, r, singleFrameTiming);
		_boundingBox.x = r.x * SCALE;
		_boundingBox.y = r.y * SCALE;
		_boundingBox.width = r.width * SCALE;
		_boundingBox.height = r.height * SCALE;

		_boundingBox = rectRound(_boundingBox);

		tDuration = getTickCount() - tStart;
	}

	this->frameTime.push_back(static_cast<double>(tDuration)/ static_cast<double>(getTickFrequency()));
	double fps = static_cast<double>(getTickFrequency() / tDuration);

	//显示

	cv::Mat image_2 = _image[1].clone();
	image_2 = image_2 * 8;
	cv::rectangle(image_2, _boundingBox.tl(), _boundingBox.br(),cv::Scalar(65536));
	cv::imshow("image_2", image_2);

	Scalar colour = _targetOnFrame ? Scalar(0, 255, 0) : Scalar(0, 0, 255);
	Mat hudImage;
	_image[0].copyTo(hudImage);

	rectangle(hudImage, _boundingBox, colour, 2);
	Point_<double> center;
	center.x = _boundingBox.x + _boundingBox.width / 2;
	center.y = _boundingBox.y + _boundingBox.height / 2;
	circle(hudImage, center, 3, colour, 2);

	//
	cv::Rect Current_GroundTruth = _cap.Get_Current_GroundTruth_Rect();
	rectangle(hudImage, Current_GroundTruth, Scalar(255, 0, 0), 2);
	Point_<double> center_truth;
	center_truth.x = Current_GroundTruth.x + Current_GroundTruth.width / 2;
	center_truth.y = Current_GroundTruth.y + Current_GroundTruth.height / 2;
	circle(hudImage, center_truth, 3, Scalar(255, 0, 0), 2);

	//计算重合率
	float overlap = this->Overlap(_boundingBox, Current_GroundTruth,_targetOnFrame);
	_overlap_sum += overlap;
	std::cout << "the current overlap is " << overlap<< "   and the sum of it is  " << _overlap_sum << std::endl;

	stringstream ss;
	ss << "FPS: " << fps;
	putText(hudImage, ss.str(), cv::Point(20, 20), FONT_HERSHEY_TRIPLEX, 0.5,
			Scalar(255, 0, 0));

	ss.str("");
	ss.clear();
	ss << "#" << _frameIdx;
	putText(hudImage, ss.str(), cv::Point(hudImage.cols - 60, 20),FONT_HERSHEY_TRIPLEX, 0.5, Scalar(255, 0, 0));

	if (!_targetOnFrame)
	{
		cv::Point_<double> tl = _boundingBox.tl();
		cv::Point_<double> br = _boundingBox.br();

		line(hudImage, tl, br, colour);
		line(hudImage, cv::Point_<double>(tl.x, br.y),cv::Point_<double>(br.x, tl.y), colour);
	}
	std::cout << "current _frameIdx is " << _frameIdx << std::endl;
	imshow(_windowTitle.c_str(), hudImage);

	if (_frameIdx == 1)
		waitKey(0);
	else
		cv::waitKey(33);
	waitKey(0);

	/***********************save the result rect and overlap data**************************/

	ofstream outfile;
	std::string name = _cap._name + ".txt";
	outfile.open(name.c_str(), ios::app);
	//cout.setf(ios::fixed);
	outfile.unsetf(ios::fixed);
	outfile << _boundingBox.x << "," << _boundingBox.y << ","<< _boundingBox.width << "," << _boundingBox.height << ",";
	outfile.setf(ios::fixed);
	outfile << std::setprecision(5) << overlap << ",";
	outfile.unsetf(ios::fixed);
	outfile << _frameIdx << "\n";
	outfile.close();

	++_frameIdx;
	/*		stringstream stream;
	 stream << "train_detect" << _frameIdx<<".jpg";

	 cv::imwrite(stream.str(), hudImage);*/
	std::cout << std::endl << std::endl;

	return true;
}

void TrackerRun::setTrackerDebug(TrackerDebug* debug)
{
	_debug = debug;
}
float TrackerRun::Overlap(const cv::Rect_<double>& boundBox,
		const cv::Rect_<double>& groundtruth, bool targetOnFrame)
{
	if ((!targetOnFrame) && groundtruth.area() == 0)
	{
		return 1.0;
	}
	else if ((!targetOnFrame && groundtruth.area() != 0)|| (targetOnFrame && groundtruth.area() == 0))
	{
		return -1;
	}
	else
	{
		if (boundBox.x > groundtruth.x + groundtruth.width)
		{
			return 0.0;
		}
		if (boundBox.y > groundtruth.y + groundtruth.height)
		{
			return 0.0;
		}
		if (boundBox.x + boundBox.width < groundtruth.x)
		{
			return 0.0;
		}
		if (boundBox.y + boundBox.height < groundtruth.y)
		{
			return 0.0;
		}
		float colInt = min(boundBox.x + boundBox.width,groundtruth.x + groundtruth.width)- max(boundBox.x, groundtruth.x);
		float rowInt = min(boundBox.y + boundBox.height,groundtruth.y + groundtruth.height)- max(boundBox.y, groundtruth.y);
		float intersection = colInt * rowInt;
		float area1 = boundBox.width * boundBox.height;
		float area2 = groundtruth.width * groundtruth.height;
		return intersection / (area1 + area2 - intersection);
	}
}
