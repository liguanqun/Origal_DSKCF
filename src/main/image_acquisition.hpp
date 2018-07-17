
#ifndef IMAGE_ACQUISITION_HPP_
#define IMAGE_ACQUISITION_HPP_

#include "opencv2/highgui/highgui.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <dirent.h>
#include<unistd.h>
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<sys/types.h>
#include<dirent.h>
#include<map>

class ImageAcquisition
	{
	public:
		ImageAcquisition();
		virtual ~ImageAcquisition();
		void Init();

		cv::Mat Get_init_RGB();

		cv::Mat Get_first_RGB();
		cv::Mat Get_Next_RGB();

		cv::Mat Get_Depth_Image_same_time_to_RGB(double & delta_t);

        cv::Rect Get_Init_Rect(void);




		void Get_Time_And_K(std::string str, int & t, int & k);
		cv::Mat Shift_Bit_Depth_Image(cv::Mat& image);

        std::string _name;
        std::string _path;
        int _size;
	private:

		std::map<int, std::string> _FrameID_path, _FrameID_path_depth;

		std::map<int, int> _FrameID_t, _FrameID_t_depth;
		std::map<int, int> _RGB_benchmark_ID,_DEPTH_benchmark_ID;
		int  _rgb_FrameID;

	};

#endif
