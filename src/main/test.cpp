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
void plot_cruve(std::vector<double> data, std::vector<double> data1, std::string windoe_title, int longth, int step, int threshold, cv::Scalar scalar, cv::Scalar scalar_a, bool flag);
void plot_distance_err_cruve(std::vector<double> data, std::vector<double> data1, std::string windoe_title, cv::Scalar scalar, cv::Scalar scalar_a);
void plot_overlap_cruve(std::vector<double> data, std::vector<double> data1, std::string windoe_title, cv::Scalar scalar, cv::Scalar scalar_a);
int main(int argc, const char** argv)
	{

		if (argc == 2)
			{
				string name = argv[1];
				std::vector<double> weight_distance_err, distance_err;
				std::vector<double> weight_overlap, overlap;

				std::string path = "weight_" + name + "_distance_err.txt";
				read_data(path, weight_distance_err, 1);

				path = "weight_" + name + "_overlap.txt";
				read_data(path, weight_overlap, 5);

				path = name + "_distance_err.txt";
				read_data(path, distance_err, 1);

				path = name + "_overlap.txt";
				read_data(path, overlap, 5);

				cv::namedWindow("distance_err_rate", 0);
				cv::namedWindow("overlap_rate", 0);
				plot_cruve(weight_distance_err, distance_err, "distance_err_rate", 50, 1, 20, cv::Scalar(255, 0, 0), cv::Scalar(0, 255, 0), true);
				plot_cruve(weight_overlap, overlap, "overlap_rate", 100, 100, 50, cv::Scalar(255, 0, 0), cv::Scalar(0, 255, 0), false);

				cv::namedWindow("distance", 0);
				cv::namedWindow("overlap", 0);
				plot_distance_err_cruve(weight_distance_err, distance_err, "distance", cv::Scalar(255, 0, 0), cv::Scalar(0, 255, 0));
				plot_overlap_cruve(weight_overlap, overlap, "overlap", cv::Scalar(255, 0, 0), cv::Scalar(0, 255, 0));
				cv::waitKey(0);
			}

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

void plot_cruve(std::vector<double> data, std::vector<double> data1, std::string windoe_title, int longth, int step, int threshold, cv::Scalar scalar, cv::Scalar scalar_a, bool flag)
	{
		std::cout << "size " << data.size() << "  " << data1.size() << std::endl;
		if (data.size() == data1.size())
			{
				std::vector<int> resut, resut_a;
				for (int i = 0; i < longth; i++)
					{
						int a = 0, b = 0;
						for (int j = 0; j < data.size(); j++)
							{
								if (flag)
									{
										if (data[j] * step <= i)
											a++;
										if (data1[j] * step <= i)
											b++;
									}
								else
									{
										if (data[j] * step >= i)
											a++;
										if (data1[j] * step >= i)
											b++;
									}

							}
						std::cout << a << " " << b << " ; ";
						resut.push_back(a);
						resut_a.push_back(b);

					}
				std::cout << std::endl;

				cv::Mat hist_picture(data.size(), resut.size() * 3, CV_8UC3, cv::Scalar(255, 255, 255));
				for (int i = 1; i < resut.size(); i++)
					{
						cv::Point p1 = cv::Point((i - 1) * 3, hist_picture.rows - resut[i - 1]);
						cv::Point p2 = cv::Point(i * 3, hist_picture.rows - resut[i]);
						cv::line(hist_picture, p1, p2, scalar, 1);
					}
				for (int i = 1; i < resut.size(); i++)
					{
						cv::Point p1 = cv::Point((i - 1) * 3, hist_picture.rows - resut_a[i - 1]);
						cv::Point p2 = cv::Point(i * 3, hist_picture.rows - resut_a[i]);
						cv::line(hist_picture, p1, p2, scalar_a, 1);
					}
				for (int i = 4; i < longth; i++)
					{
						if ((i + 1) % 5 == 0)
							{
								cv::Point p1 = cv::Point(i * 3, hist_picture.rows);
								cv::Point p2 = cv::Point(i * 3, hist_picture.rows - 5);
								cv::line(hist_picture, p1, p2, cv::Scalar(0, 0, 0), 1);
							}
					}
				for (int i = 0; i < data.size(); i++)
					{
						if ((i + 1) % ((int) (data.size() / 20)) == 0)
							{
								cv::Point p1 = cv::Point(0, i);
								cv::Point p2 = cv::Point(5, i);
								cv::line(hist_picture, p1, p2, cv::Scalar(0, 0, 0), 1);
							}
					}
				cv::Point p1 = cv::Point((threshold - 1) * 3, 0);
				cv::Point p2 = cv::Point((threshold - 1) * 3, hist_picture.rows);
				cv::line(hist_picture, p1, p2, scalar, 1);
				cv::imshow(windoe_title, hist_picture);
			}
	}
void plot_overlap_cruve(std::vector<double> data, std::vector<double> data1, std::string windoe_title, cv::Scalar scalar, cv::Scalar scalar_a)
	{
		if (data.size() == data1.size())
			{
				cv::Mat hist_picture_overlap(600, data.size() * 3, CV_8UC3, cv::Scalar(255, 255, 255));
				cv::Point p5 = cv::Point(0, hist_picture_overlap.rows - 50 * 6);
				cv::Point p6 = cv::Point(data.size() * 3 - 1, hist_picture_overlap.rows - 50 * 6);
				cv::line(hist_picture_overlap, p5, p6, cv::Scalar(0, 0, 255), 1);
				for (int i = 1; i < data.size(); i++)
					{
						cv::Point p1 = cv::Point((i - 1) * 3, hist_picture_overlap.rows - data[i - 1] * 100 * 6);
						cv::Point p2 = cv::Point(i * 3, hist_picture_overlap.rows - data[i] * 100 * 6);
						cv::line(hist_picture_overlap, p1, p2, scalar, 1);
						cv::Point p3 = cv::Point((i - 1) * 3, hist_picture_overlap.rows - data1[i - 1] * 100 * 6);
						cv::Point p4 = cv::Point(i * 3, hist_picture_overlap.rows - data1[i] * 100 * 6);
						cv::line(hist_picture_overlap, p3, p4, scalar_a, 1);
					}
				for (int i = 9; i < data.size(); i++)
					{
						if ((i + 1) % 10 == 0)
							{
								cv::Point p1 = cv::Point(i * 3, 0);
								cv::Point p2 = cv::Point(i * 3, 10);
								cv::line(hist_picture_overlap, p1, p2, cv::Scalar(0, 0, 255), 1);
							}
						if ((i + 1) % 50 == 0)
							{
								cv::Point p1 = cv::Point(i * 3, 0);
								cv::Point p2 = cv::Point(i * 3, 15);
								cv::line(hist_picture_overlap, p1, p2, cv::Scalar(0, 0, 255), 1);
							}
					}

				cv::imshow(windoe_title, hist_picture_overlap);
			}
		else
			{
				std::cout << "data isn't the same size " << std::endl;
			}
	}

void plot_distance_err_cruve(std::vector<double> data, std::vector<double> data1, std::string windoe_title, cv::Scalar scalar, cv::Scalar scalar_a)
	{
		if (data.size() == data1.size())
			{
				cv::Mat hist_picture(600, data.size() * 3, CV_8UC3, cv::Scalar(255, 255, 255));

				cv::Point p3 = cv::Point(0, hist_picture.rows - 20 * 6);
				cv::Point p4 = cv::Point(data.size() * 3 - 1, hist_picture.rows - 20 * 6);
				cv::line(hist_picture, p3, p4, cv::Scalar(0, 0, 0), 1);

				for (int i = 1; i < data.size(); i++)
					{
						cv::Point p1 = cv::Point((i - 1) * 3, hist_picture.rows - data[i - 1] * 6);
						cv::Point p2 = cv::Point(i * 3, hist_picture.rows - data[i] * 6);
						cv::line(hist_picture, p1, p2, scalar, 1);
						cv::Point p4 = cv::Point((i - 1) * 3, hist_picture.rows - data1[i - 1] * 6);
						cv::Point p5 = cv::Point(i * 3, hist_picture.rows - data1[i] * 6);
						cv::line(hist_picture, p4, p5, scalar_a, 1);
					}

				for (int i = 9; i < data.size(); i++)
					{
						if ((i + 1) % 10 == 0)
							{
								cv::Point p1 = cv::Point(i * 3, hist_picture.rows);
								cv::Point p2 = cv::Point(i * 3, hist_picture.rows - 10);
								cv::line(hist_picture, p1, p2, cv::Scalar(0, 0, 255), 1);
							}
						if ((i + 1) % 50 == 0)
							{
								cv::Point p1 = cv::Point(i * 3, hist_picture.rows);
								cv::Point p2 = cv::Point(i * 3, hist_picture.rows - 15);
								cv::line(hist_picture, p1, p2, cv::Scalar(0, 0, 255), 1);
							}
					}
				//cv::namedWindow("distance success", 0);
				cv::imshow(windoe_title, hist_picture);
			}
		else
			{
				std::cout << "data isn't the same size " << std::endl;
			}
	}
