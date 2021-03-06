
/*
// Original file: https://github.com/Itseez/opencv_contrib/blob/292b8fa6aa403fb7ad6d2afadf4484e39d8ca2f1/modules/tracking/samples/tracker.cpp
// Modified by Klaus Haag file: https://github.com/klahaag/cf_tracking/blob/master/src/3rdparty/cv_ext/tracker_run.cpp
// + Authors: Jake Hall, Massimo Camplan, Sion Hannuna
// * Add a variety of additional features to visualize tracker, save results according to RGBD dataset (see details below) and to save processing
//   time as in the DS-KCF paper
//  Princeton RGBD data: Shuran Song and Jianxiong Xiao. Tracking Revisited using RGBD Camera: Baseline and Benchmark. 2013.
*/


#ifndef TRACKER_RUN_HPP_
#define TRACKER_RUN_HPP_

#include <tclap/CmdLine.h>
#include <array>
#include <fstream>
#include <opencv2/highgui/highgui.hpp>
#include "cf_tracker.hpp"
#include "tracker_debug.hpp"
#include "image_acquisition.hpp"
#include "dskcf_tracker.hpp"



class TrackerRun
{
public:
    TrackerRun(std::string windowTitle);
    virtual ~TrackerRun();
    bool start(int argc, const char** argv);


private:

    bool init();
    bool run();
    bool update();

    float Overlap(const cv::Rect_<double>& boundBox,const cv::Rect_<double>& groundtruth, bool targetOnFrame);
    double distance_err(const cv::Rect_<double>& boundBox, const cv::Rect_<double>& groundtruth, bool targetOnFrame);
private:
    std::array< cv::Mat, 2 > _image;
    CfTracker* _tracker;
    std::string _windowTitle;

    float _overlap_sum;
    double _distance_sum;
    int    _overlap_success_frame,_distance_success_frame;
    int  _overlap_threshold,_distance_threshold;
    std::vector<double> _center_err;
    std::vector<cv::Rect_<double>> _rect_result;
    std::vector<float> _OVERLAP;


    cv::Rect_<double> _boundingBox;
     ImageAcquisition  _cap;

    int _frameIdx;
    bool _hasInitBox;
    bool _targetOnFrame;
	std::vector< double > frameTime;
};

#endif
