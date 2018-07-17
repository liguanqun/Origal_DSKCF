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
#include<iomanip>


using namespace cv;
using namespace std;
using namespace TCLAP;

#define SCALE 1

TrackerRun::TrackerRun(string windowTitle)
	{
		_hasInitBox = false;
		_targetOnFrame = false;
		_windowTitle = windowTitle;
		_tracker = 0;
		_frameIdx = 1;

	}

TrackerRun::~TrackerRun()
	{

		if (_tracker)
			{
				delete _tracker;
				_tracker = 0;
			}


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

//bool TrackerRun::start(int argc, const char** argv)
bool TrackerRun::start(int argc, std::string path,double mul)
{
		if (argc == 3)
			{
				this->_cap._path =path;
				this->_mul = mul;//
				//this->_cap._path =argv[1];
				//this->_mul = std::atof(argv[2]);//
			}
		else
			{
				std::cout << "there is not correct parametrs enter" << std::endl;
				return false;
			}
		this->_tracker = new DskcfTracker(this->_mul);
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

		double delta_t = 0;
		if (_frameIdx == 1)
			{
				_image[0] = _cap.Get_first_RGB();
				_image[1] = _cap.Get_Depth_Image_same_time_to_RGB(delta_t);
				if (_image[0].empty() || _image[1].empty())
					{
						return false;
					}
			}
		else
			{
				_image[0] = _cap.Get_Next_RGB();
				_image[1] = _cap.Get_Depth_Image_same_time_to_RGB(delta_t);
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

				//double mul = 1;
				//if (delta_t < 1)mul = this->_mul * (1 - delta_t);
				//std::cout << std::setiosflags(ios::fixed)<<"delta_t from RGB and depth is " << std::setprecision(2)<<delta_t << "  mul == " << mul<<std::setprecision(0) << std::endl;


				_targetOnFrame = _tracker->update(resized, r, singleFrameTiming, this->_mul);
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

		//cv::Point_<double> prdicted = this->_tracker->get_predicted_point() * SCALE;
		//std::cout << "for show prdicted  " << prdicted.x << " * " << prdicted.y << std::endl;
		//circle(hudImage, prdicted, 3, cv::Scalar(0, 0, 255), 2);

		stringstream ss;
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
		//保存结果
		ofstream outfile_result_rect;
		char mul_str[256];
		sprintf(mul_str, "%.2f", this->_mul);
		string mul_result = mul_str;
		//std::string name = "weight_" + mul_result + "_" + _cap._name + "_result.txt";
		std::string name = _cap._name + ".txt";
		outfile_result_rect.open(name.c_str(), ios::app);
		if (_targetOnFrame)
			{
				outfile_result_rect << _boundingBox.x << ","<<_boundingBox.y<<","<<_boundingBox.width<<","<<_boundingBox.height<<","<<'0';
			}
		else
			{
				outfile_result_rect <<"NaN,NaN,NaN,NaN,"<<'1';
			}
		outfile_result_rect << "\n";
		outfile_result_rect.close();

		waitKey(1);
		//if(_frameIdx>150)waitKey(0);
		++_frameIdx;

		std::cout << std::endl << std::endl;

		return true;
	}

