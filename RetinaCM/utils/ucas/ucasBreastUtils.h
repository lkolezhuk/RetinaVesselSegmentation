#ifndef _UCAS_BREAST_UTILS
#define _UCAS_BREAST_UTILS

#include "ucasImageUtils.h"
#include "ucasLog.h"

/*****************************************************************
*   Utility methods              								 *
******************************************************************/
namespace ucas
{
	// returns the binary image consisting of the segmented breast
	cv::Mat breastSegment(
		const cv::Mat & image,			// input mammogram (either 8- or 16-bit grayscale image)
		binarizationMethod method = all,// binarization method (brute force attack if not defined)
		bool noBlack = false,			// exclude black (=0) pixels from computation of global threshold
		bool noWhite = false,			// exclude white (=2^depth-1) pixels from computation of global threshold
		bool bracket_histo = true,		// bracket the histogram to the range that holds data to make it quicker
		ucas::StackPrinter *printer = 0)
		throw ( ucas::Error );

	// returns true if the given breast mask contains B white pixels, with minP*size(mask) <= B <= maxP*size(mask)
	bool checkBreastMask(
		const cv::Mat & mask,			// breast mask
		float minF = 0.05,				// minimum fraction of white/total pixels
		float maxF = 0.95				// maximum fraction of white/total pixels
	) throw (Error);
}

#endif