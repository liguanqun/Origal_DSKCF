
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

struct Parameters{
    std::string sequencePathRGB;
    std::string sequencePathDepth;
    std::string outputFilePath;
    std::string imgExportPath;
    std::string expansionRGB;
    std::string expansionDepth;
    cv::Rect initBb;
    int deviceRGB;
    int deviceDepth;
    int startFrame;
    bool showOutput;
    bool paused;
    bool repeat;
    bool isMockSequenceRGB;
    bool isMockSequenceDepth;
	bool useDepth;
};

class TrackerRun
{
public:
    TrackerRun(std::string windowTitle);
    virtual ~TrackerRun();
    bool start(int argc, const char** argv);
    void setTrackerDebug(TrackerDebug* debug);

private:
 //   Parameters parseCmdArgs(int argc, const char** argv);
    bool init();
    bool run();
    bool update();
    void printResults(const cv::Rect_<double>& boundingBox, bool isConfident, bool isTracked);
	void printResultsTiming(const std::vector<int64> &singleFrameTiming);
    float Overlap(const cv::Rect_<double>& box1,const cv::Rect_<double>& box2);
//protected:
//    virtual CfTracker* parseTrackerParas(TCLAP::CmdLine& cmd, int argc, const char** argv) = 0;
private:
    std::array< cv::Mat, 2 > _image;
    CfTracker* _tracker;
    std::string _windowTitle;
    float _overlap_sum;
  //  Parameters _paras;
    cv::Rect_<double> _boundingBox;
     ImageAcquisition  _cap;
    std::ofstream _resultsFile,_resultsFileTime;
    TCLAP::CmdLine _cmd;
    TrackerDebug* _debug;
    int _frameIdx;
    bool _isPaused;
    bool _isStep;
    bool _exit;
    bool _hasInitBox;
    bool _isTrackerInitialzed;
    bool _targetOnFrame;
    bool _updateAtPos;
	int _imageIndex;
	std::vector< double > frameTime;
};

#endif
