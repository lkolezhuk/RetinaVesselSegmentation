// include aia and ucas utility functions
#include "aia/aiaConfig.h"
#include "image_management.h"
#include <iostream>

#include <opencv2\legacy\legacy.hpp>
#include <opencv2\opencv.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <opencv2/imgproc/imgproc.hpp>
#include "functions.h"

#include "cl_Texture.h"


using namespace std;
int main() 
{
		std::vector<cv::Mat> images_raw = ImageManagement::LoadAllInDir("C:/Users/Admin/Documents/Education/MAIA-Italia/AdvancedImageAnalysis/ProjectsSemester/projects/AIA-Retinal-Vessel-Segmentation/dataset/images",".tif",false);
		std::vector<cv::Mat> masks = ImageManagement::LoadAllInDir("C:/Users/Admin/Documents/Education/MAIA-Italia/AdvancedImageAnalysis/ProjectsSemester/projects/AIA-Retinal-Vessel-Segmentation/dataset/mask",".tif",true);
		std::vector<cv::Mat> imagesRGB =  ImageManagement::ApplyMasks(images_raw, masks);
		std::vector<cv::Mat> images = ImageManagement::ExtractChanell(imagesRGB, 1);

		cv::namedWindow( "Display window1", cv::WINDOW_AUTOSIZE );// Create a window for display.
		cv::imshow( "Display window1", images_raw[0] );                   // Show our image inside it.

		//cv::waitKey(0); 
		//cv::namedWindow( "Display window2", cv::WINDOW_AUTOSIZE );// Create a window for display.
		//cv::imshow( "Display window2", images[0] );                   // Show our image inside it.

		

		/*cv::waitKey(0); 
		cv::GaussianBlur( images[0], images[0], cv::Size(5,5), 10, 10, cv::BORDER_DEFAULT );
		cv::imshow( "Display window2", images[0] );  */

		cv::waitKey(0); 
		cv::fastNlMeansDenoising(images[0], images[0], 3.0, 7, 21);
		cv::imshow( "Display window2", images[0] );  
		
		cv::waitKey(0); 
		
		/// Convert it to gray
		cv::Mat image_gray = images[0];
		//cvtColor( images[0], image_gray, CV_2GRAY );
		
		//cv::adaptiveBilateralFilter(image_gray, image_gray, cv::Size(5,5), 30);
		//cv::imshow( "Display window2", images[0] );                   // Show our image inside it.
		//
		//	image_gray.convertTo(image_gray,CV_32F);
		//cv::equalizeHist(image_gray, image_gray );


		cv::Ptr<cv::CLAHE> clahe =  cv::createCLAHE(4);
		clahe->apply(image_gray,image_gray);

		cv::imshow( "Display window2", image_gray );                   // Show our image inside it.
		cv::waitKey(0); 
		
		cv::vector<cv::Mat> image_gabor;
		for (unsigned int i = 0; i < 8; i++)
		{
			cv::Mat img;
			cv::Mat kernel = cv::getGaborKernel(cv::Size(9,7), 3.95, i*3.14/8 , 7.2, 4, 0);
			cv::filter2D(image_gray, img, CV_32F, kernel);
			image_gabor.push_back(img);
		}

		cv::Mat viz;
		cv::Mat blended_gabor = ImageManagement::BlendImages(image_gabor);
		blended_gabor.convertTo(viz, CV_8U, 10.0/255.0);  
		cv::imshow("Display window1", image_gray);

		double min;
		double max;
		cv::minMaxIdx(blended_gabor, &min, &max);
		cv::Mat adjMap;
		cv::convertScaleAbs(blended_gabor, blended_gabor, 255 / max);
		//clahe->apply(adjMap,blended_gabor);
		cv::imshow("Display window2", blended_gabor);
		cv::waitKey(0);
		/*cv::Mat res;
		cv::threshold(blended_gabor, res, 150, 255, CV_THRESH_BINARY );
		cv::imshow("OTSU", res);

			*/
		
		int optimal_threshold = 0;
		double max_entropy = 0;
		//cv::GaussianBlur( blended_gabor, blended_gabor, cv::Size(3,3), 0, 0, cv::BORDER_DEFAULT );
		cv::imshow("Display window2", blended_gabor);
		cv::waitKey(0);

		for (int threshold = 0; threshold < 255; threshold += 5)
		{
			cv::Mat res;

			cv::threshold(blended_gabor, res, threshold, 255, CV_THRESH_BINARY );

			const IplImage * pGray = new IplImage(res);
			//cout<<"ad"<<endl;
		
		
			cl_Texture* texture=new cl_Texture();
			cl_Texture::GLCM* glcm;
			int count_steps=2;
			const int StepDirections[] = { 0,1,  -1,0};

			double d0, d1, d2, d3;

			// store the features in this array
			double * features = new double[4*count_steps]; 

			// call function
			glcm = texture->CreateGLCM(pGray, 1,StepDirections,count_steps,CV_GLCM_OPTIMIZATION_LUT); 
					
			// get the features from GLCM
			texture->CreateGLCMDescriptors(glcm, CV_GLCMDESC_OPTIMIZATION_ALLOWDOUBLENEST); 
			double avg_entropy = 0;
			for(int i = 0; i < count_steps;i++)
			{
				
				d0 = texture->GetGLCMDescriptor(glcm,i,CV_GLCMDESC_ENTROPY);
				avg_entropy += d0;
				features[i*count_steps] = d0;
				cout << threshold << ": "<< d0 << " ";
			}
			avg_entropy = avg_entropy / count_steps;
			if(avg_entropy > max_entropy) {
				max_entropy = avg_entropy;
				optimal_threshold  = threshold;
			}
			cout<<endl;
		}
		cout<<"Max entropy is "<<max_entropy << " when threshold is " << optimal_threshold <<endl;
		cv::Mat opt;
		cv::threshold(blended_gabor, opt, 50, 255, CV_THRESH_BINARY );
		cv::imshow("Optimal", opt);

		
		cv::waitKey(0);
		//cv::Mat grad_x, grad_y, grad;
	 //   cv::Mat abs_grad_x, abs_grad_y;
		//int scale = 3;
		//int delta = 0;
		//int ddepth = CV_16S;

		//  /// Gradient X
		//  //Scharr( src_gray, grad_x, ddepth, 1, 0, scale, delta, BORDER_DEFAULT );
		//  Sobel( image_gray, grad_x, ddepth, 1, 0, 3, scale, delta, cv::BORDER_DEFAULT );
		//  convertScaleAbs( grad_x, abs_grad_x );

		//  /// Gradient Y
		//  //Scharr( src_gray, grad_y, ddepth, 0, 1, scale, delta, BORDER_DEFAULT );
		//  Sobel( image_gray, grad_y, ddepth, 0, 1, 3, scale, delta, cv::BORDER_DEFAULT );
		//  convertScaleAbs( grad_y, abs_grad_y );

		//  /// Total Gradient (approximate)
		//  addWeighted( abs_grad_x, 0.5, abs_grad_y, 0.5, 0, grad );
		//  /*threshold( grad, grad, 50, 0,3);*/
		//  cv::imshow("Display window2", grad );

		//  cv::waitKey(0);
		
		return 1;
}

