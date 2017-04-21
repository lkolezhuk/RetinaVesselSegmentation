#ifndef _Image_Management_h
#define _Image_Management_h

#include "aia/aiaConfig.h"
#include "ucas/ucasConfig.h"

using namespace std;

class ImageManagement
{
public:
	ImageManagement(void);
	~ImageManagement(void);
	static vector<cv::Mat> LoadAllInDir(string folder, string ext, bool force_gray);
	static vector<cv::Mat> ApplyMasks(vector<cv::Mat> images, vector<cv::Mat> masks);
};

#endif