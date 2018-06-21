#ifndef _DSKCF_TRACKER_HPP_
#define _DSKCF_TRACKER_HPP_

#include <array>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/traits.hpp>
#include <opencv2/core/core.hpp>

#include "cf_tracker.hpp"
#include "kcf_tracker.hpp"
#include "math_helper.hpp"
#include "DepthSegmenter.hpp"
#include "ScaleAnalyser.hpp"
#include "FeatureExtractor.hpp"
#include "OcclusionHandler.hpp"
#include "ScaleChangeObserver.hpp"

/**
 * DskcfTracker implements a depth scaling kernelised correlation filter as
 * described in \cite DSKCF.
 *
 *  [1] S. Hannuna, M. Camplani, J. Hall, M. Mirmehdi, D. Damen, T. Burghardt, A. Paiement, L. Tao,
 *  DS-KCF: A ~real-time tracker for RGB-D data, Journal of Real-Time Image Processing
 */
class DskcfTracker: public CfTracker
	{
	public:
		DskcfTracker();
		virtual ~DskcfTracker();
		virtual bool update(const std::array<cv::Mat, 2> & frame, cv::Rect_<double> & boundingBox);
		virtual bool update(const std::array<cv::Mat, 2> & frame, cv::Rect_<double>& boundingBox, std::vector<int64> &timePerformanceVector);
		virtual bool reinit(const std::array<cv::Mat, 2> & frame, cv::Rect_<double> & boundingBox);

		virtual TrackerDebug* getTrackerDebug();
		virtual const std::string getId();
	private:

		/** The occlusion handler associated with this object */
		std::shared_ptr<OcclusionHandler> m_occlusionHandler;
	};

#endif
