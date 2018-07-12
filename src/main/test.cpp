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
#include <numeric>
#include <stdio.h>
#include<iomanip>
#include<map>
#include <array>

using namespace std;
using namespace cv;
//读取数据
void read_data(std::string path, std::vector<double>& data_save, int fixed_precisious);

//画两条曲线进行对比
void plot_cruve(std::vector<double> data, std::vector<double> data1, std::string windoe_title, int longth, int step, int threshold, cv::Scalar scalar, cv::Scalar scalar_a, bool flag);
void plot_distance_err_cruve(std::vector<double> data, std::vector<double> data1, std::string windoe_title, cv::Scalar scalar, cv::Scalar scalar_a);
void plot_overlap_cruve(std::vector<double> data, std::vector<double> data1, std::string windoe_title, cv::Scalar scalar, cv::Scalar scalar_a);


//可以同时在一张图里画任意多条曲线
bool distance_success_rate(std::vector<std::vector<double>> data, std::string windoe_title);
bool overlap_success_rate(std::vector<std::vector<double>> data, std::string windoe_title);
bool plot_distance_err_cruve(std::vector<std::vector<double>> data, std::string windoe_title);
bool plot_overlap_cruve(std::vector<std::vector<double>> data, std::string windoe_title);

//读取ECO的估计矩形数据，生成距离差文件和重合度文件
void ECO_data(string path_ECO, string path_groundtruth);


int main(int argc, const char** argv)
	{
	//	ECO_data(argv[1],argv[2]);
		std::vector<string> name;
		for (int i = 1; i < argc; i++)
			{
				name.push_back(argv[i]);
			}

		std::vector<std::vector<double>> distance_err, overlap;
		for (int i = 0; i < argc - 1; i++)
			{
				std::vector<double> data;
				std::string path = name[i] + "_distance_err.txt";
				read_data(path, data, 1);
				distance_err.push_back(data);
				data.clear();

				path = name[i] + "_overlap.txt";
				read_data(path, data, 5);
				overlap.push_back(data);
				data.clear();
			}
		cv::namedWindow("distance_success_rate", 0);
		cv::namedWindow("overlap_success_rate", 0);
		distance_success_rate(distance_err, "distance_success_rate");
		overlap_success_rate(overlap, "overlap_success_rate");

		 cv::namedWindow("distance_err", 0);
		 cv::namedWindow("overlap", 0);
		 plot_distance_err_cruve(distance_err, "distance_err");
		 plot_overlap_cruve(overlap, "overlap");
		/*
		 cv::namedWindow("distance_success_rate", 0);
		 cv::namedWindow("overlap_rate", 0);
		 plot_cruve(distance_err[0], distance_err[1], "distance_success_rate", 50, 1, 20, cv::Scalar(255, 0, 0), cv::Scalar(0, 255, 0), true);
		 plot_cruve(overlap[0], overlap[1], "overlap_rate", 100, 100, 50, cv::Scalar(255, 0, 0), cv::Scalar(0, 255, 0), false);

		 cv::namedWindow("distance", 0);
		 cv::namedWindow("overlap", 0);
		 plot_distance_err_cruve(distance_err[0], distance_err[1], "distance", cv::Scalar(255, 0, 0), cv::Scalar(0, 255, 0));
		 plot_overlap_cruve(overlap[0], overlap[1], "overlap", cv::Scalar(255, 0, 0), cv::Scalar(0, 255, 0));
		 */
		cv::waitKey(0);

//cv::waitKey(0);
		return 0;
	}

void ECO_data(string path_ECO, string path_groundtruth)
	{
		std::map<int, cv::Rect> _FrameID_rect;
		ifstream myfile(path_groundtruth.c_str());
		if (!myfile.is_open())
			{
				cout << "can not open the   file" << endl;
			}

		string temp;
		int Rcet_ID = 1;
		while (getline(myfile, temp))
			{
				std::cout<<temp<<std::endl;
				cv::Rect r;
				if (temp.substr(0, temp.find_first_of(',')).c_str() == "NaN")
					{
						r.x = 0;
						r.y = 0;
						r.height = 0;
						r.width = 0;
						_FrameID_rect[Rcet_ID] = r;
					}
				else
					{
						r.x = atoi(temp.substr(0, temp.find_first_of(',')).c_str());
						temp.erase(0, temp.find_first_of(',') + 1);

						r.y = atoi(temp.substr(0, temp.find_first_of(',')).c_str());
						temp.erase(0, temp.find_first_of(',') + 1);

						r.width = atoi(temp.substr(0, temp.find_first_of(',')).c_str());
						temp.erase(0, temp.find_first_of(',') + 1);

						r.height = atoi(temp.substr(0, temp.find_first_of(',')).c_str());
						temp.erase(0, temp.find_first_of(',') + 1);

						int k = atoi(temp.c_str());
						if(k!=Rcet_ID)std::cout<<"read groundtruth rect error k="<<k<<"  Rect_ID == "<<Rcet_ID<<std::endl;
						_FrameID_rect[Rcet_ID] = r;
					}

				Rcet_ID++;
				temp.clear();
			}
/////////////////////////////////////////////////////////////////////////////////////
		std::map<int, cv::Rect> _FrameID_rect_ECO;
		ifstream myfile_eco(path_ECO.c_str());
		if (!myfile_eco.is_open())
			{
				cout << "can not open the   file" << endl;
			}

		Rcet_ID = 1;
		temp.clear();
		while (getline(myfile_eco, temp))
			{
				std::cout<<temp<<std::endl;
				cv::Rect r;
				if (Rcet_ID > _FrameID_rect.size())
					{
						break;
					}
				else if (temp.substr(0, temp.find_first_of(',')).c_str() == "NaN")
					{
						r.x = 0;
						r.y = 0;
						r.height = 0;
						r.width = 0;
						_FrameID_rect_ECO[Rcet_ID] = r;
					}
				else
					{

						r.x = atoi(temp.substr(0, temp.find_first_of(',')).c_str());
						temp.erase(0, temp.find_first_of(',') + 1);

						r.y = atoi(temp.substr(0, temp.find_first_of(',')).c_str());
						temp.erase(0, temp.find_first_of(',') + 1);

						r.width = atoi(temp.substr(0, temp.find_first_of(',')).c_str());
						temp.erase(0, temp.find_first_of(',') + 1);

						r.height = atoi(temp.substr(0, temp.find_first_of(',')).c_str());
						temp.erase(0, temp.find_first_of(',') + 1);

						int k = atoi(temp.c_str());
						if(k!=Rcet_ID)std::cout<<"read eco result rect error k="<<k<<"  Rect_ID == "<<Rcet_ID<<std::endl;
						_FrameID_rect_ECO[Rcet_ID] = r;
					}
            temp.clear();
				Rcet_ID++;
			}
//////////////////////////////////////////////////////////////////////////
		ofstream outfile_distance_err;
		std::string name = "csr-dcf_distance_err.txt";
		outfile_distance_err.open(name.c_str(), ios::app);
		outfile_distance_err.setf(ios::fixed);
		for (int j = 1; j < _FrameID_rect.size(); j++)
			{
				double err;
				cv::Point_<double> center;
				center.x = _FrameID_rect_ECO[j].x + _FrameID_rect_ECO[j].width / 2;
				center.y = _FrameID_rect_ECO[j].y + _FrameID_rect_ECO[j].height / 2;

				Point_<double> center_truth;
				center_truth.x = _FrameID_rect[j].x + _FrameID_rect[j].width / 2;
				center_truth.y = _FrameID_rect[j].y + _FrameID_rect[j].height / 2;

				err = std::abs(center.x - center_truth.x) + std::abs(center.y - center_truth.y);
				outfile_distance_err << std::setprecision(1) << err << ",";
			}
		outfile_distance_err << "\n";
		outfile_distance_err.close();
/////////////////////////////////////////////////////////////////////////
		ofstream overlap;
		std::string name_overlap = "csr-dcf_overlap.txt";
		overlap.open(name_overlap.c_str(), ios::app);
		overlap.setf(ios::fixed);
		for (int j = 1; j < _FrameID_rect.size(); j++)
			{
				double overlap_value;

				float colInt = min(_FrameID_rect_ECO[j].x + _FrameID_rect_ECO[j].width, _FrameID_rect[j].x + _FrameID_rect[j].width) - max(_FrameID_rect_ECO[j].x, _FrameID_rect[j].x);
				float rowInt = min(_FrameID_rect_ECO[j].y + _FrameID_rect_ECO[j].height, _FrameID_rect[j].y + _FrameID_rect[j].height) - max(_FrameID_rect_ECO[j].y, _FrameID_rect[j].y);
				float intersection = colInt * rowInt;
				float area1 = _FrameID_rect_ECO[j].width * _FrameID_rect_ECO[j].height;
				float area2 = _FrameID_rect[j].width * _FrameID_rect[j].height;
				overlap_value = intersection / (area1 + area2 - intersection);

				overlap << std::setprecision(5) << overlap_value << ",";
			}
		overlap << "\n";
		overlap.close();

	}

void read_data(std::string path, std::vector<double>& data_save, int fixed_precisious)
	{
		data_save.clear();

		ifstream myfile(path.c_str());
		if (!myfile.is_open())
			{
				cout << "can not open the   file from the path:" << path << endl;

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
				if (data_save.size() > 162)
			break;
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
				std::vector<int> result, result_a;
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
						result.push_back(a);
						result_a.push_back(b);

					}
				std::cout << std::endl;

				std::cout << "success " << result[threshold] << "  VS  " << result_a[threshold] << std::endl;

				cv::Mat hist_picture(data.size(), result.size() * 3, CV_8UC3, cv::Scalar(255, 255, 255));
				for (int i = 1; i < result.size(); i++)
					{
						cv::Point p1 = cv::Point((i - 1) * 3, hist_picture.rows - result[i - 1]);
						cv::Point p2 = cv::Point(i * 3, hist_picture.rows - result[i]);
						cv::line(hist_picture, p1, p2, scalar, 1);
					}
				for (int i = 1; i < result.size(); i++)
					{
						cv::Point p1 = cv::Point((i - 1) * 3, hist_picture.rows - result_a[i - 1]);
						cv::Point p2 = cv::Point(i * 3, hist_picture.rows - result_a[i]);
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
		else
			{
				std::cout << "data isn't the same size " << std::endl;
			}
	}

bool distance_success_rate(std::vector<std::vector<double>> data, std::string windoe_title)
	{
		std::vector<cv::Scalar> color;
		color.push_back(cv::Scalar(255, 0, 0));
		color.push_back(cv::Scalar(0, 255, 0));
		color.push_back(cv::Scalar(0, 0, 255));
		color.push_back(cv::Scalar(0, 0, 0));

		for (int i = 1; i < data.size(); i++)
			{
				if (data[i].size() != data[i - 1].size())
					{
						std::cout << "data isn't the same size " << std::endl;
						return false;
					}
			}

		std::vector<std::vector<int>> result;
		for (int i = 0; i < data.size(); i++)
			{
				std::vector<int> a;
				for (int j = 0; j < 50; j++)
					{
						int aa = 0;
						for (int k = 0; k < data[0].size(); k++)
							{

								if (data[i][k] <= j)
									{
										aa++;
									}
							}
						a.push_back(aa);
					}
				result.push_back(a);
			}

		for (int j = 0; j < result[0].size(); j++)
			{
				for (int i = 0; i < result.size(); i++)
					{
						std::cout << " " << result[i][j] << "  ";
					}
				std::cout << ";";
			}
		std::cout << std::endl;
		std::cout << "distance success： ";
		for (int i = 0; i < result.size(); i++)
			{
				std::cout << result[i][19] << "  ";
			}
		std::cout << std::endl;

		cv::Mat hist_picture(data[0].size(), result[0].size() * 3, CV_8UC3, cv::Scalar(255, 255, 255));
		for (int i = 0; i < result.size(); i++)
			{
				for (int j = 1; j < result[0].size(); j++)
					{
						cv::Point p1 = cv::Point((j - 1) * 3, hist_picture.rows - result[i][j - 1]);
						cv::Point p2 = cv::Point(j * 3, hist_picture.rows - result[i][j]);
						cv::line(hist_picture, p1, p2, color[i % 4], 1);

					}
			}

		for (int i = 4; i < 50; i++)
			{
				if ((i + 1) % 5 == 0)
					{
						cv::Point p1 = cv::Point(i * 3, hist_picture.rows);
						cv::Point p2 = cv::Point(i * 3, hist_picture.rows - 5);
						cv::line(hist_picture, p1, p2, cv::Scalar(0, 0, 0), 1);
					}
			}
		for (int i = 0; i < data[0].size(); i++)
			{
				if ((i + 1) % ((int) (data[0].size() / 20)) == 0)
					{
						cv::Point p1 = cv::Point(0, i);
						cv::Point p2 = cv::Point(5, i);
						cv::line(hist_picture, p1, p2, cv::Scalar(0, 0, 0), 1);
					}
			}
		cv::Point p1 = cv::Point(19 * 3, 0);
		cv::Point p2 = cv::Point(19 * 3, hist_picture.rows);
		cv::line(hist_picture, p1, p2, cv::Scalar(0, 0, 0), 1);

		cv::imshow(windoe_title, hist_picture);

		return true;

	}

bool overlap_success_rate(std::vector<std::vector<double>> data, std::string windoe_title)
	{
		std::vector<cv::Scalar> color;
		color.push_back(cv::Scalar(255, 0, 0));
		color.push_back(cv::Scalar(0, 255, 0));
		color.push_back(cv::Scalar(0, 0, 255));
		color.push_back(cv::Scalar(0, 0, 0));

		for (int i = 1; i < data.size(); i++)
			{
				if (data[i].size() != data[i - 1].size())
					{
						std::cout << "data isn't the same size " << std::endl;
						return false;
					}
			}

		std::vector<std::vector<int>> result;
		for (int i = 0; i < data.size(); i++)
			{
				std::vector<int> a;
				for (int j = 0; j < 100; j++)
					{
						int aa = 0;
						for (int k = 0; k < data[0].size(); k++)
							{

								if (data[i][k] * 100 >= j)
									{
										aa++;
									}
							}
						a.push_back(aa);
					}
				result.push_back(a);
			}

		for (int j = 0; j < result[0].size(); j++)
			{
				for (int i = 0; i < result.size(); i++)
					{
						std::cout << " " << result[i][j] << "  ";
					}
				std::cout << ";";
			}
		std::cout << std::endl;

		std::cout << "overlap success： ";
		for (int i = 0; i < result.size(); i++)
			{
				std::cout << result[i][49] << "  ";
			}
		std::cout << std::endl;

		cv::Mat hist_picture(data[0].size(), result[0].size() * 3, CV_8UC3, cv::Scalar(255, 255, 255));
		for (int i = 0; i < result.size(); i++)
			{
				for (int j = 1; j < result[0].size(); j++)
					{
						cv::Point p1 = cv::Point((j - 1) * 3, hist_picture.rows - result[i][j - 1]);
						cv::Point p2 = cv::Point(j * 3, hist_picture.rows - result[i][j]);
						cv::line(hist_picture, p1, p2, color[i % 4], 1);

					}
			}
		cv::Point p1 = cv::Point(49 * 3, 0);
		cv::Point p2 = cv::Point(49 * 3, hist_picture.rows);
		cv::line(hist_picture, p1, p2, cv::Scalar(0, 0, 0), 1);
		cv::imshow(windoe_title, hist_picture);

		return true;

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
bool plot_overlap_cruve(std::vector<std::vector<double>> data, std::string windoe_title)
	{

		std::vector<cv::Scalar> color;
		color.push_back(cv::Scalar(255, 0, 0));
		color.push_back(cv::Scalar(0, 255, 0));
		color.push_back(cv::Scalar(0, 0, 255));
		color.push_back(cv::Scalar(0, 0, 0));

		for (int i = 1; i < data.size(); i++)
			{
				if (data[i].size() != data[i - 1].size())
					{
						std::cout << "data isn't the same size " << std::endl;
						return false;
					}
			}

		cv::Mat hist_picture_overlap(600, data[0].size() * 3, CV_8UC3, cv::Scalar(255, 255, 255));
		cv::Point p5 = cv::Point(0, hist_picture_overlap.rows - 50 * 6);
		cv::Point p6 = cv::Point(data[0].size() * 3 - 1, hist_picture_overlap.rows - 50 * 6);
		cv::line(hist_picture_overlap, p5, p6, cv::Scalar(0, 0, 0), 1);
		for (int i = 0; i < data.size(); i++)
			{
				for (int j = 1; j < data[0].size(); j++)
					{
						cv::Point p1 = cv::Point((j - 1) * 3, hist_picture_overlap.rows - data[i][j - 1] * 100 * 6);
						cv::Point p2 = cv::Point(j * 3, hist_picture_overlap.rows - data[i][j] * 100 * 6);
						cv::line(hist_picture_overlap, p1, p2, color[i % 4], 1);
					}

			}
		for (int i = 9; i < data[0].size(); i++)
			{
				if ((i + 1) % 10 == 0)
					{
						cv::Point p1 = cv::Point(i * 3, 0);
						cv::Point p2 = cv::Point(i * 3, 10);
						cv::line(hist_picture_overlap, p1, p2, cv::Scalar(0, 0, 0), 1);
					}
				if ((i + 1) % 50 == 0)
					{
						cv::Point p1 = cv::Point(i * 3, 0);
						cv::Point p2 = cv::Point(i * 3, 15);
						cv::line(hist_picture_overlap, p1, p2, cv::Scalar(0, 0, 0), 1);
					}
			}

		cv::imshow(windoe_title, hist_picture_overlap);
		return 0;
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

bool plot_distance_err_cruve(std::vector<std::vector<double>> data, std::string windoe_title)
	{

		std::vector<cv::Scalar> color;
		color.push_back(cv::Scalar(255, 0, 0));
		color.push_back(cv::Scalar(0, 255, 0));
		color.push_back(cv::Scalar(0, 0, 255));
		color.push_back(cv::Scalar(0, 0, 0));

		for (int i = 1; i < data.size(); i++)
			{
				if (data[i].size() != data[i - 1].size())
					{
						std::cout << "data isn't the same size " << std::endl;
						return false;
					}
			}

		cv::Mat hist_picture(600, data[0].size() * 3, CV_8UC3, cv::Scalar(255, 255, 255));

		cv::Point p3 = cv::Point(0, hist_picture.rows - 20 * 6);
		cv::Point p4 = cv::Point(data[0].size() * 3 - 1, hist_picture.rows - 20 * 6);
		cv::line(hist_picture, p3, p4, cv::Scalar(0, 0, 0), 1);

		for (int i = 0; i < data.size(); i++)
			{
				for (int j = 1; j < data[0].size(); j++)
					{
						cv::Point p1 = cv::Point((j - 1) * 3, hist_picture.rows - data[i][j - 1] * 6);
						cv::Point p2 = cv::Point(j * 3, hist_picture.rows - data[i][j] * 6);
						cv::line(hist_picture, p1, p2, color[i % 4], 1);
					}
			}

		for (int i = 9; i < data[0].size(); i++)
			{
				if ((i + 1) % 10 == 0)
					{
						cv::Point p1 = cv::Point(i * 3, hist_picture.rows);
						cv::Point p2 = cv::Point(i * 3, hist_picture.rows - 10);
						cv::line(hist_picture, p1, p2, cv::Scalar(0, 0, 0), 1);
					}
				if ((i + 1) % 50 == 0)
					{
						cv::Point p1 = cv::Point(i * 3, hist_picture.rows);
						cv::Point p2 = cv::Point(i * 3, hist_picture.rows - 15);
						cv::line(hist_picture, p1, p2, cv::Scalar(0, 0, 0), 1);
					}
			}
		//cv::namedWindow("distance success", 0);
		cv::imshow(windoe_title, hist_picture);
		return true;
	}
