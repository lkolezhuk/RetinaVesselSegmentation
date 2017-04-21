#ifndef _project_0_h
#define _project_0_h

#include "aia/aiaConfig.h"
#include <opencv2/core/core.hpp>

// open namespace "aia"
namespace aia
{
	// define a new namespace "project0"  within "aia"
	namespace project0
	{
		// this is just an example: find all faces in the given image using HaarCascade Face Detection
		cv::Mat faceRectangles(const cv::Mat & frame) throw (aia::error);
	}
}

#endif // _project_0_h