/*
 * test.cpp
 *
 *  Created on: Jun 27, 2018
 *      Author: orbbec
 */

#include <iostream>
#include <ctype.h>
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <string>
#include <fstream>
#include <stdio.h>
#include<iomanip>

using namespace std;
using namespace cv;
void read_data(std::string path, std::vector<double>& data_save, int fixed_precisious);
void plot_cruve(std::vector<double> data, std::string windoe_title, int longth, int step, int threshold, cv::Scalar scalar, bool flag);
int main()
	{

		std::vector<double> weight_distance_err, distance_err;
		std::vector<double> weight_overlap, overlap;

		std::string path = "weight_child_no1_distance_err.txt";
		read_data(path, weight_distance_err, 1);

		path = "weight_child_no1_overlap.txt";
		read_data(path, weight_overlap, 5);
		cv::namedWindow("distance_err", 0);
		cv::namedWindow("overlap", 0);
		plot_cruve(weight_distance_err, "distance_err", 50, 1, 20, cv::Scalar(255, 0, 0), true);
		plot_cruve(weight_overlap, "overlap", 100, 100, 50, cv::Scalar(0, 255, 0), false);
		cv::waitKey(0);

//cv::waitKey(0);
		return 0;
	}
void read_data(std::string path, std::vector<double>& data_save, int fixed_precisious)
	{
		data_save.clear();

		ifstream myfile(path.c_str());
		if (!myfile.is_open())
			{
				cout << "can not open the init file from the path:" << path << endl;
			}

		string tmp;
		std::cout << "read from " << path << std::endl;
		//std::cout << setiosflags(ios::fixed);
		//cout << setprecision(fixed_precisious);
		getline(myfile, tmp);

		double distance_err_tmp;
		//std::cout << tmp << std::endl;
		while (!tmp.empty())
			{
				distance_err_tmp = atof(tmp.substr(0, tmp.find_first_of(",")).c_str());
				//	std::cout << distance_err_tmp << "   ";
				data_save.push_back(distance_err_tmp);
				tmp.erase(0, tmp.find_first_of(",") + 1);
			}
		//	std::cout << std::endl;

		myfile.close();
	}

void plot_cruve(std::vector<double> data, std::string windoe_title, int longth, int step, int threshold, cv::Scalar scalar, bool flag)
	{
		std::vector<int> resut;
		for (int i = 0; i < longth; i++)
			{
				int a = 0;
				for (int j = 0; j < data.size(); j++)
					{
						if (flag)
							{
								if (data[j] * step <= i)
									a++;
							}
						else
							{
								if (data[j] * step >= i)
									a++;
							}

					}
				std::cout << a << "  ";
				resut.push_back(a);

			}
		std::cout << std::endl;

		cv::Mat hist_picture(data.size(), resut.size() * 3, CV_8UC3, cv::Scalar(255, 255, 255));
		for (int i = 1; i < resut.size(); i++)
			{
				cv::Point p1 = cv::Point((i - 1) * 3, hist_picture.rows - resut[i - 1]);
				cv::Point p2 = cv::Point(i * 3, hist_picture.rows - resut[i]);
				cv::line(hist_picture, p1, p2, scalar, 2);
			}
		for (int i = 4; i < longth; i++)
			{
				if ((i + 1) % 5 == 0)
					{
						cv::Point p1 = cv::Point(i * 3, hist_picture.rows);
						cv::Point p2 = cv::Point(i * 3, hist_picture.rows - 5);
						cv::line(hist_picture, p1, p2, cv::Scalar(0,0,0), 1);
					}
			}
		for (int i = 0; i < data.size(); i++)
			{
				if ((i + 1) % ((int)(data.size()/20)) == 0)
					{
						cv::Point p1 = cv::Point(0 ,i);
						cv::Point p2 = cv::Point(5 ,i);
						cv::line(hist_picture, p1, p2, cv::Scalar(0,0,0), 1);
					}
			}
		cv::Point p1 = cv::Point((threshold-1) * 3, 0);
		cv::Point p2 = cv::Point((threshold-1) * 3, hist_picture.rows);
		cv::line(hist_picture, p1, p2, scalar, 1);
		cv::imshow(windoe_title, hist_picture);

	}
