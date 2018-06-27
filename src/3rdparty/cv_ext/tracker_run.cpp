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

TrackerRun::TrackerRun(string windowTitle)
	{
		_hasInitBox = false;
		_targetOnFrame = false;
		_windowTitle = windowTitle;
		_tracker = 0;
		_frameIdx = 1;

		_overlap_sum = 0;
		_distance_sum = 0;
		_overlap_threshold = 0.5;
		_distance_threshold = 20;
		_overlap_success_frame = 0;
		_distance_success_frame = 0;
	}

TrackerRun::~TrackerRun()
	{

		if (_tracker)
			{
				delete _tracker;
				_tracker = 0;
			}
		this->_frameIdx -= 1;
		std::cout << "total frame is " << this->_frameIdx << std::endl;
		std::cout << "distance success " << this->_distance_success_frame << "  frames " << std::endl;
		std::cout << "overlap success  " << this->_overlap_success_frame << "  frames " << std::endl;

		/**************************************************************************/
		ofstream outfile_distance_err;
		std::string name ="weight"+ _cap._name + "_distance_err.txt";
		outfile_distance_err.open(name.c_str(), ios::app);
		outfile_distance_err.setf(ios::fixed);
		for (int j = 0; j < this->_center_err.size(); j++)
			{
				double err;
				err = this->_center_err[j];
				outfile_distance_err << std::setprecision(1) << err << ",";
			}
		outfile_distance_err << "\n";
		outfile_distance_err.close();

///*******************************************************************************/
		ofstream outfile_overlap;
		std::string name_overlap ="weight"+ _cap._name + "_overlap.txt";
		outfile_overlap.open(name_overlap.c_str(), ios::app);
		outfile_overlap.setf(ios::fixed);
		for (int j = 0; j < this->_OVERLAP.size(); j++)
			{
				double overlap = this->_OVERLAP[j];
				outfile_overlap << std::setprecision(5) << overlap << ",";

			}
		outfile_overlap << "\n";
		outfile_overlap.close();

		std::cout << "Frame,Time" << std::endl;

		/*		for (int i = 0; i < (int) this->frameTime.size(); i++)
		 {
		 //frameID 是从1开始的，这里对齐
		 std::cout << i+1 << "," << this->frameTime[i] << std::endl;
		 }*/

		std::cout << "min," << *std::min_element(this->frameTime.begin(), this->frameTime.end()) << std::endl;
		std::cout << "max," << *std::max_element(this->frameTime.begin(), this->frameTime.end()) << std::endl;
		std::cout << "mean," << std::accumulate(this->frameTime.begin(), this->frameTime.end(), 0.0) / static_cast<double>(this->frameTime.size()) << std::endl;
	}

bool TrackerRun::start(int argc, const char** argv)
	{

		this->_tracker = new DskcfTracker();

		if (argc == 2)
			{
				this->_cap._path = argv[1];
			}
		else
			{
				return false;
			}
		this->init();
		while (this->run())
			;

		return false;
	}

bool TrackerRun::init()
	{
//读取RGB和depth图像
		_cap.Init();

		namedWindow(_windowTitle.c_str());

		_boundingBox = _cap.Get_Init_Rect();
		_hasInitBox = true;

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

		return false;
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
				cv::resize(_image[0], resized[0], cv::Size_<double>(_image[0].cols / SCALE, _image[0].rows / SCALE), 0, 0, cv::INTER_LINEAR);
				cv::resize(_image[1], resized[1], cv::Size_<double>(_image[1].cols / SCALE, _image[1].rows / SCALE), 0, 0, cv::INTER_NEAREST);
				cv::Rect_<double> r(_boundingBox.x / SCALE, _boundingBox.y / SCALE, _boundingBox.width / SCALE, _boundingBox.height / SCALE);

				_targetOnFrame = _tracker->reinit(resized, r);

				tDuration = getTickCount() - tStart;

			}
		else if (_frameIdx > 1)
			{
				tStart = getTickCount();

				std::array<cv::Mat, 2> resized;
				cv::resize(_image[0], resized[0], cv::Size_<double>(_image[0].cols / SCALE, _image[0].rows / SCALE), 0, 0, cv::INTER_LINEAR);
				cv::resize(_image[1], resized[1], cv::Size_<double>(_image[1].cols / SCALE, _image[1].rows / SCALE), 0, 0, cv::INTER_NEAREST);

				cv::Rect_<double> r(_boundingBox.x / SCALE, _boundingBox.y / SCALE, _boundingBox.width / SCALE, _boundingBox.height / SCALE);

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

		this->frameTime.push_back(static_cast<double>(tDuration) / static_cast<double>(getTickFrequency()));
		double fps = static_cast<double>(getTickFrequency() / tDuration);

		//显示

		cv::Mat image_2 = _image[1].clone();
		image_2 = image_2 * 8;
		cv::rectangle(image_2, _boundingBox.tl(), _boundingBox.br(), cv::Scalar(65536));
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

		//计算重合率 和 移动距离
		this->_rect_result.push_back(_boundingBox);
		float overlap = this->Overlap(_boundingBox, Current_GroundTruth, _targetOnFrame);
		double distance =this->distance_err(_boundingBox, Current_GroundTruth, _targetOnFrame);
		this->_OVERLAP.push_back(overlap);
		this->_center_err.push_back(distance);
		this->_overlap_sum += overlap;
		std::cout << "the current overlap is " << overlap << "   and the sum of it is  " << _overlap_sum << std::endl;
		this->_distance_sum += distance;
		std::cout << "the distance  is  " << distance << "   sum distance is " << _distance_sum << std::endl;
		//画出实时的曲线
		cv::Mat hist_picture(600, this->_cap._size * 3, CV_8UC3, cv::Scalar(255, 255, 255));

		cv::Point p3 = cv::Point(0, hist_picture.rows - 20 * 6);
		cv::Point p4 = cv::Point(this->_cap._size * 3 - 1, hist_picture.rows - 20 * 6);
		cv::line(hist_picture, p3, p4, cv::Scalar(255, 0, 0), 1);

		for (int i = 1; i < this->_center_err.size(); i++)
			{
				cv::Point p1 = cv::Point((i - 1) * 3, hist_picture.rows - this->_center_err[i - 1] * 6);
				cv::Point p2 = cv::Point(i * 3, hist_picture.rows - this->_center_err[i] * 6);
				cv::line(hist_picture, p1, p2, cv::Scalar(255, 0, 0), 1);
			}
		cv::namedWindow("distance success", 0);
		cv::imshow("distance success", hist_picture);
		cv::Mat hist_picture_overlap(600, this->_cap._size * 3, CV_8UC3, cv::Scalar(255, 255, 255));
		cv::Point p5 = cv::Point(0, hist_picture_overlap.rows - 50 * 6);
		cv::Point p6 = cv::Point(this->_cap._size * 3 - 1, hist_picture_overlap.rows - 50 * 6);
		cv::line(hist_picture_overlap, p5, p6, cv::Scalar(0, 0, 255), 1);
		for (int i = 1; i < this->_OVERLAP.size(); i++)
			{
				cv::Point p1 = cv::Point((i - 1) * 3, hist_picture_overlap.rows - this->_OVERLAP[i - 1] * 100 * 6);
				cv::Point p2 = cv::Point(i * 3, hist_picture_overlap.rows - this->_OVERLAP[i] * 100 * 6);
				cv::line(hist_picture_overlap, p1, p2, cv::Scalar(0, 0, 255), 1);
			}
		cv::namedWindow("overlap success", 0);
		cv::imshow("overlap success", hist_picture_overlap);

		if (overlap >= this->_overlap_threshold)
			this->_overlap_success_frame += 1;
		if (distance <= this->_distance_threshold)
			this->_distance_success_frame += 1;

		std::cout << "overlap success " << this->_overlap_success_frame << " times" << std::endl << "distance success " << _distance_success_frame << "times" << std::endl;

		stringstream ss;
		ss << "FPS: " << fps << "current overlap:" << overlap << " distance:" << distance;
		putText(hudImage, ss.str(), cv::Point(20, 20), FONT_HERSHEY_TRIPLEX, 0.5, Scalar(255, 0, 0));

		ss.str("");
		ss.clear();
		ss << "#" << _frameIdx;
		putText(hudImage, ss.str(), cv::Point(hudImage.cols - 60, 20), FONT_HERSHEY_TRIPLEX, 0.5, Scalar(255, 0, 0));

		if (!_targetOnFrame)
			{
				cv::Point_<double> tl = _boundingBox.tl();
				cv::Point_<double> br = _boundingBox.br();

				line(hudImage, tl, br, colour);
				line(hudImage, cv::Point_<double>(tl.x, br.y), cv::Point_<double>(br.x, tl.y), colour);
			}

		imshow(_windowTitle.c_str(), hudImage);

		waitKey(0);
		++_frameIdx;

		std::cout << std::endl << std::endl;

		return true;
	}

float TrackerRun::Overlap(const cv::Rect_<double>& boundBox, const cv::Rect_<double>& groundtruth, bool targetOnFrame)
	{
		if ((!targetOnFrame) && groundtruth.area() == 0)
			{
				return 1.0;
			}
		else if ((!targetOnFrame && groundtruth.area() != 0) || (targetOnFrame && groundtruth.area() == 0))
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
				float colInt = min(boundBox.x + boundBox.width, groundtruth.x + groundtruth.width) - max(boundBox.x, groundtruth.x);
				float rowInt = min(boundBox.y + boundBox.height, groundtruth.y + groundtruth.height) - max(boundBox.y, groundtruth.y);
				float intersection = colInt * rowInt;
				float area1 = boundBox.width * boundBox.height;
				float area2 = groundtruth.width * groundtruth.height;
				return intersection / (area1 + area2 - intersection);
			}
	}
double TrackerRun::distance_err(const cv::Rect_<double>& boundBox, const cv::Rect_<double>& groundtruth, bool targetOnFrame)
	{
		double distance = 0;
		if (groundtruth.area() != 0)
			{
				Point_<double> center;
				center.x = boundBox.x + boundBox.width / 2;
				center.y = boundBox.y + boundBox.height / 2;

				Point_<double> center_truth;
				center_truth.x = groundtruth.x + groundtruth.width / 2;
				center_truth.y = groundtruth.y + groundtruth.height / 2;

				distance = std::abs(center.x -center_truth.x) +  std::abs(center.y -center_truth.y);
			}

		return distance;

	}
