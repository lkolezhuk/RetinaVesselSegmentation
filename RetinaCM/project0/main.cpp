// include aia and ucas utility functions
#include "aia/aiaConfig.h"
#include "image_management.h"
#include <iostream>

using namespace std;
int main() 
{
		
		std::vector<cv::Mat> images_raw = ImageManagement::LoadAllInDir("C:/Users/Admin/Documents/Education/MAIA-Italia/AdvancedImageAnalysis/ProjectsSemester/projects/AIA-Retinal-Vessel-Segmentation/dataset/images",".tif",true);
		std::vector<cv::Mat> masks = ImageManagement::LoadAllInDir("C:/Users/Admin/Documents/Education/MAIA-Italia/AdvancedImageAnalysis/ProjectsSemester/projects/AIA-Retinal-Vessel-Segmentation/dataset/mask",".tif",true);
		std::vector<cv::Mat> images =  ImageManagement::ApplyMasks(images_raw, masks);
		
		cv::namedWindow( "Display window1", cv::WINDOW_AUTOSIZE );// Create a window for display.
		cv::imshow( "Display window1", images_raw[0] );                   // Show our image inside it.

		cv::waitKey(0); 

		cv::namedWindow( "Display window2", cv::WINDOW_AUTOSIZE );// Create a window for display.
		cv::imshow( "Display window2", images[0] );                   // Show our image inside it.

		cv::waitKey(0); 
		return 1;
	
	
}

