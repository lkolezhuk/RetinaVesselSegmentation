#ifndef _UCAS_IMAGE_UTILS_H
#define _UCAS_IMAGE_UTILS_H

#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "ucasMathUtils.h"
#include "ucasExceptions.h"
#include <vector>

/*****************************************************************
*   OpenCV utility methods       								 *
******************************************************************/
namespace ucas
{
	// converts the OpenCV depth flag into the corresponding bitdepth
	int     imdepth(int ocv_depth);

	// extends the OpenCV "imshow" function with the addition of a scale factor
	void    imshow(const std::string winname, const cv::InputArray arr, bool wait = true, float scale = 1.0);

	// extends the OpenCV "imread" function by adding support to grayscale 8-16 bits DICOM images (.dcm)
	// please note that for DICOM images:
	// - 'opencv_flags' are ignored
	// - pixel values are NOT changed
	// - 'bits_used' returns the bits used 
	cv::Mat imread(const std::string & path, int opencv_flags = 1, int *bits_used = 0) throw (ucas::Error);

	// rescale the image from 'bits_in' to 'bits_out'
	void    imrescale(cv::Mat & image, int bits_in, int bits_out) throw (ucas::Error);
}

/*****************************************************************
*   Image binarization methods   								 *
******************************************************************/
namespace ucas
{
	enum binarizationMethod
	{
		all,											// try all implemented binarization techniques
		otsuopencv,										// Otsu binarization (opencv)
		otsu,											// Otsu binarization 
		isodata,										// see [T.W. Ridler, S. Calvard, "Picture thresholding using an iterative selection method", IEEE Trans. System, Man and Cybernetics, SMC-8 (1978) 630-632]
		triangle,										// see [Zack GW, Rogers WE, Latt SA (1977), "Automatic measurement of sister chromatid exchange frequency", J. Histochem. Cytochem. 25 (7): 741–53]
		mean,											// see [C. A. Glasbey, "An analysis of histogram-based thresholding algorithms," CVGIP: Graphical Models and Image Processing, vol. 55, pp. 532-537, 1993]
		minerror,										// see [ Kittler and J. Illingworth, "Minimum error thresholding," Pattern Recognition, vol. 19, pp. 41-47, 1986]
		maxentropy,										// see [ Kapur J.N., Sahoo P.K., and Wong A.K.C. (1985) "A New Method for Gray-Level Picture Thresholding Using the Entropy of the Histogram" Graphical Models and Image Processing, 29(3): 273-285]
		renyientropy,									// see [ Kapur J.N., Sahoo P.K., and Wong A.K.C. (1985) "A New Method for Gray-Level Picture Thresholding Using the Entropy of the Histogram" Graphical Models and Image Processing, 29(3): 273-285]
		yen												// see [ Yen J.C., Chang F.J., and Chang S. (1995) "A New Criterion for Automatic Multilevel Thresholding" IEEE Trans. on Image Processing, 4(3): 370-378]
	};
	std::string binarizationMethod_toString(binarizationMethod code);
	binarizationMethod binarizationMethod_toInt(const std::string & code);
	std::string binarizationMethods();

	// C. A. Glasbey, "An analysis of histogram-based thresholding algorithms,"
	// CVGIP: Graphical Models and Image Processing, vol. 55, pp. 532-537, 1993.
	int getMeanThreshold(const std::vector<int> & data) throw (ucas::Error);

	// Kittler and J. Illingworth, "Minimum error thresholding," Pattern Recognition, vol. 19, pp. 41-47, 1986.
	// C. A. Glasbey, "An analysis of histogram-based thresholding algorithms," CVGIP: Graphical Models and Image Processing, vol. 55, pp. 532-537, 1993.
	int getMinErrorIThreshold(const std::vector<int> & data) throw (ucas::Error);

	// Iterative procedure based on the isodata algorithm [T.W. Ridler, S. Calvard, Picture 
	// thresholding using an iterative selection method, IEEE Trans. System, Man and Cybernetics, SMC-8 (1978) 630-632.] 
	int getIsoDataAutoThreshold(const std::vector<int> & data) throw (ucas::Error);

	// Zack, G. W., Rogers, W. E. and Latt, S. A., 1977,
	// Automatic Measurement of Sister Chromatid Exchange Frequency, Journal of Histochemistry and Cytochemistry 25 (7), pp. 741-753
	int getTriangleAutoThreshold(const std::vector<int> & data) throw (ucas::Error);

	// Kapur J.N., Sahoo P.K., and Wong A.K.C. (1985) 
	// "A New Method for Gray-Level Picture Thresholding Using the Entropy of the Histogram" Graphical Models and Image Processing, 29(3): 273-285
	int getMaxEntropyAutoThreshold(const std::vector<int> & data) throw (ucas::Error);

	// Kapur J.N., Sahoo P.K., and Wong A.K.C. (1985) 
	// "A New Method for Gray-Level Picture Thresholding Using the Entropy of the Histogram" Graphical Models and Image Processing, 29(3): 273-285
	int getRenyiEntropyAutoThreshold(const std::vector<int> & data) throw (ucas::Error);

	//Yen J.C., Chang F.J., and Chang S. (1995) 
	// "A New Criterion for Automatic Multilevel Thresholding" IEEE Trans. on Image Processing, 4(3): 370-378
	int getYenyAutoThreshold(const std::vector<int> & data) throw (ucas::Error);

	// Otsu's threshold algorithm
	int getOtsuAutoThreshold(const std::vector<int> & data) throw (ucas::Error);

	// output(x,y) = maxval if input(x,y) > threshold, 0 otherwise
	// *** WARNING *** : binarization is done in place (8-bit output)
	cv::Mat binarize(cv::Mat & image, int threshold) throw (ucas::Error);
	/*-------------------------------------------------------------------------------------------------------------------------*/
}

/*****************************************************************
*   Image processing functions									 *
******************************************************************/
namespace ucas
{
	//return image histogram (the number of bins is automatically set to 2^imgdepth if not provided)
	std::vector<int> histogram(const cv::Mat & image, int bins = -1 ) throw (ucas::Error);
	
	// bracket the histogram to the range that holds data
	std::vector<int> compressHistogram(std::vector<int> &histo, int & minbin);
}

#endif