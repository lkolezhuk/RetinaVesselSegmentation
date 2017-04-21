#include "ucasImageUtils.h"
#include "ucasStringUtils.h"
#include "ucasFileUtils.h"
#include "ucasLog.h"
#include "ucasTypes.h"

#ifdef WITH_GDCM
#include "gdcmImage.h"
#include "gdcmImageReader.h"
#endif

namespace
{
	// image histogram statistics
	inline double A(const std::vector<int> &y, int j=-1) 
	{
		if(j < 0 || j >= y.size())
			j = int(y.size())-1;
		double x = 0;
		for (int i=0;i<=j;i++)
			x+=y[i];
		return x;
	}
	inline double B(const std::vector<int> &y, int j=-1) 
	{
		if(j < 0 || j >= y.size())
			j = int(y.size())-1;
		double x = 0;
		for (int i=0;i<=j;i++)
			x+=i*y[i];
		return x;
	}
	inline double C(const std::vector<int> &y, int j=-1) 
	{
		if(j < 0 || j >= y.size())
			j = int(y.size())-1;
		double x = 0;
		for (int i=0;i<=j;i++)
			x+=i*i*y[i];
		return x;
	}
}

// converts the OpenCV depth flag into the corresponding bitdepth
int ucas::imdepth(int ocv_depth)
{
	switch(ocv_depth)
	{
		case CV_8U:  return 8;
		case CV_8S:  return 8;
		case CV_16U: return 16;
		case CV_16S: return 16;
		case CV_32S: return 32;
		case CV_32F: return 32;
		case CV_64F: return 64;
		default:     return -1;
	}
}

// extends the OpenCV "imshow" function with the addition of a scale factor
void ucas::imshow(const std::string winname, const cv::InputArray arr, bool wait, float scale)
{
	// create window
	cv::namedWindow(winname, CV_WINDOW_FREERATIO);

	// resize window so as to fit screen size while maintaining image aspect ratio
	int win_height = arr.size().height, win_width = arr.size().width;

	cv::resizeWindow(winname, ucas::round(win_width*scale), ucas::round(win_height*scale));

	// display image
	cv::imshow(winname, arr);

	// wait for key pressed
	if(wait)
		cv::waitKey(0);
}

cv::Mat ucas::imread(const std::string & path, int flags, int *bits_used) throw (ucas::Error)
{
	// check file exists first
	if(!ucas::isFile(path))
		throw ucas::FileNotExistsError(path);

	// check for DICOM extension and try to load with GDCM library, if present
	if(ucas::getFileExtension(path) == "dcm" || ucas::getFileExtension(path) == "DCM")
	{

#ifdef WITH_GDCM

		// read file
		gdcm::ImageReader imreader;
		imreader.SetFileName( path.c_str() );
		if(!imreader.Read())
			throw ucas::Error("GDCM cannot read DICOM image");

		// get image
		gdcm::Image &image = imreader.GetImage();

		// checks preconditions
		if(image.GetPixelFormat().GetBitsAllocated() != 8 && image.GetPixelFormat().GetBitsAllocated() != 16)
			throw ucas::Error(ucas::strprintf("Unsupported DICOM image: bits allocated are %d, but we currently support only 8 and 16", image.GetPixelFormat().GetBitsAllocated()));
		if(image.GetPixelFormat() != gdcm::PixelFormat::UINT8 && image.GetPixelFormat() != gdcm::PixelFormat::UINT16)
			throw ucas::Error(ucas::strprintf("Unsupported DICOM image: pixel format is %s, but we currently support only UINT8 and UINT16", image.GetPixelFormat().GetScalarTypeAsString()));
		if(image.GetPixelFormat().GetSamplesPerPixel() != 1)
			throw ucas::Error(ucas::strprintf("Unsupported DICOM image: samples per pixel are %d, but we currently support only 1", image.GetPixelFormat().GetSamplesPerPixel()));
		
		// create data
		cv::Mat mat(image.GetRows(), image.GetColumns(), image.GetPixelFormat().GetBitsAllocated() == 8 ? CV_8U : CV_16U);

		// read data
		if(!image.GetBuffer(reinterpret_cast<char*>(mat.data)))
			throw ucas::Error("GDCM cannot read DICOM pixel data");

		if(bits_used)
			*bits_used = image.GetPixelFormat().GetBitsStored();

		return mat;
#else
		throw ucas::Error("Cannot read DICOM file: GDCM library not found. Please re-configure the build from source and enable GDCM.");
#endif
	}

	cv::Mat mat = cv::imread(path, flags);
	if(bits_used)
		*bits_used = ucas::imdepth(mat.depth());
	return mat;
}

// rescale the image from 'bits_in' to 'bits_out'
void ucas::imrescale(cv::Mat & image, int bits_in, int bits_out) throw (ucas::Error)
{
	if(image.depth() == CV_8U)
	{
		if(bits_out > 8)
			throw ucas::Error(ucas::strprintf("Cannot rescale image: bits_out (%d) > stored bits (8)", bits_out));
		float f = (std::pow(2.0f, bits_out) - 1.0f) / ( std::pow(2.0f, bits_in) -1.0f);
		int max = int(std::pow(2.0f, bits_out) - 1);
		for(int y=0; y<image.rows; y++)
		{
			unsigned char* data = image.ptr<unsigned char>(y);
			for(int x=0; x<image.cols; x++)
				data[x] = (unsigned char)(std::min ( ucas::round( data[x] * f ), max ) );
		}
	}
	if(image.depth() == CV_16U)
	{
		if(bits_out > 16)
			throw ucas::Error(ucas::strprintf("Cannot rescale image: bits_out (%d) > stored bits (16)", bits_out));
		float f = (std::pow(2.0f, bits_out) - 1) /  ( std::pow(2.0f, bits_in)-1);
		int max = int(std::pow(2.0f, bits_out) - 1);
		for(int y=0; y<image.rows; y++)
		{
			unsigned short* data = image.ptr<unsigned short>(y);
			for(int x=0; x<image.cols; x++)
				data[x] = (unsigned short)(std::min ( ucas::round( data[x] * f ), max ) );
		}
	}
}

std::string ucas::binarizationMethod_toString(ucas::binarizationMethod code)
{
	if     (code == otsuopencv)		return "otsuopencv";
	else if(code == otsu)			return "otsu";
	else if(code == isodata)		return "isodata";
	else if(code == triangle)		return "triangle";
	else if(code == mean)			return "mean";
	else if(code == minerror)		return "minerror";
	else if(code == maxentropy)		return "maxentropy";
	else if(code == renyientropy)	return "renyientropy";
	else if(code == yen)			return "yen";
	else							return "all";
}
ucas::binarizationMethod ucas::binarizationMethod_toInt(const std::string & code)
{
	for(int i=0; i<9; i++)
		if(binarizationMethod_toString(ucas::binarizationMethod(i)).compare(code) == 0)
			return ucas::binarizationMethod(i);
	return ucas::binarizationMethod(all);
}
std::string ucas::binarizationMethods()
{
	std::string res = "{";
	for(int i=0; i<9; i++)
		res += "\"" + std::string(binarizationMethod_toString(binarizationMethod(i))) + "\"" + (i == 8 ? "}" : ", ");
	return res;
}

int ucas::getMeanThreshold(const std::vector<int> & data) throw (ucas::Error)
{
	// C. A. Glasbey, "An analysis of histogram-based thresholding algorithms,"
	// CVGIP: Graphical Models and Image Processing, vol. 55, pp. 532-537, 1993.
	//
	// The threshold is the mean of the greyscale data
	return static_cast<int> ( std::floor(B(data)/A(data)) );
}

// Otsu's threshold algorithm
int ucas::getOtsuAutoThreshold(const std::vector<int> & data) throw (ucas::Error)
{
	// Otsu's threshold algorithm
	// C++ code by Jordan Bevik <Jordan.Bevic@qtiworld.com>
	// ported to ImageJ plugin by G.Landini
	int k,kStar;  // k = the current threshold; kStar = optimal threshold
	int N1, N;    // N1 = # points with intensity <=k; N = total number of points
	double BCV, BCVmax; // The current Between Class Variance and maximum BCV
	double num, denom;  // temporary bookeeping
	int Sk;  // The total intensity for all histogram points <=k
	int S, L=int(data.size()); // The total intensity of the image

	// Initialize values:
	S = N = 0;
	for (k=0; k<L; k++){
		S += k * data[k];	// Total histogram intensity
		N += data[k];		// Total number of data points
	}

	Sk = 0;
	N1 = data[0]; // The entry for zero intensity
	BCV = 0;
	BCVmax=0;
	kStar = 0;

	// Look at each possible threshold value,
	// calculate the between-class variance, and decide if it's a max
	for (k=1; k<L-1; k++) { // No need to check endpoints k = 0 or k = L-1
		Sk += k * data[k];
		N1 += data[k];

		// The float casting here is to avoid compiler warning about loss of precision and
		// will prevent overflow in the case of large saturated images
		denom = (double)( N1) * (N - N1); // Maximum value of denom is (N^2)/4 =  approx. 3E10

		if (denom != 0 ){
			// Float here is to avoid loss of precision when dividing
			num = ( (double)N1 / N ) * S - Sk; 	// Maximum value of num =  255*N = approx 8E7
			BCV = (num * num) / denom;
		}
		else
			BCV = 0;

		if (BCV >= BCVmax){ // Assign the best threshold found so far
			BCVmax = BCV;
			kStar = k;
		}
	}
	// kStar += 1;	// Use QTI convention that intensity -> 1 if intensity >= k
	// (the algorithm was developed for I-> 1 if I <= k.)
	return kStar;
}

//Yen J.C., Chang F.J., and Chang S. (1995) 
// "A New Criterion for Automatic Multilevel Thresholding" IEEE Trans. on Image Processing, 4(3): 370-378
int ucas::getYenyAutoThreshold(const std::vector<int> & data) throw (ucas::Error)
{
	int threshold;
	int ih, it;
	double crit;
	double max_crit;
	double * norm_histo = new double[data.size()]; /* normalized histogram */
	double * P1 = new double[data.size()]; /* cumulative normalized histogram */
	double * P1_sq = new double[data.size()];
	double * P2_sq = new double[data.size()];

	int total =0;
	for (ih = 0; ih < data.size(); ih++ )
		total+=data[ih];

	for (ih = 0; ih < data.size(); ih++ )
		norm_histo[ih] = (double)data[ih]/total;

	P1[0]=norm_histo[0];
	for (ih = 1; ih < data.size(); ih++ )
		P1[ih]= P1[ih-1] + norm_histo[ih];

	P1_sq[0]=norm_histo[0]*norm_histo[0];
	for (ih = 1; ih < data.size(); ih++ )
		P1_sq[ih]= P1_sq[ih-1] + norm_histo[ih] * norm_histo[ih];

	P2_sq[data.size() - 1] = 0.0;
	for ( ih = int(data.size())-2; ih >= 0; ih-- )
		P2_sq[ih] = P2_sq[ih + 1] + norm_histo[ih + 1] * norm_histo[ih + 1];

	/* Find the threshold that maximizes the criterion */
	threshold = -1;
	max_crit = -std::numeric_limits<double>::max();
	for ( it = 0; it < data.size(); it++ ) {
		crit = -1.0 * (( P1_sq[it] * P2_sq[it] )> 0.0? std::log( P1_sq[it] * P2_sq[it]):0.0) +  2 * ( ( P1[it] * ( 1.0 - P1[it] ) )>0.0? std::log(  P1[it] * ( 1.0 - P1[it] ) ): 0.0);
		if ( crit > max_crit ) {
			max_crit = crit;
			threshold = it;
		}
	}

	delete[] norm_histo;
	delete[] P1;
	delete[] P1_sq;
	delete[] P2_sq;

	return threshold;
}

// Kapur J.N., Sahoo P.K., and Wong A.K.C. (1985) 
// "A New Method for Gray-Level Picture Thresholding Using the Entropy of the Histogram" Graphical Models and Image Processing, 29(3): 273-285
int ucas::getRenyiEntropyAutoThreshold(const std::vector<int> & data) throw (ucas::Error)
{
	int threshold; 
	int opt_threshold;

	int ih, it;
	int first_bin;
	int last_bin;
	int tmp_var;
	int t_star1, t_star2, t_star3;
	int beta1, beta2, beta3;
	double alpha;/* alpha parameter of the method */
	double term;
	double tot_ent;  /* total entropy */
	double max_ent;  /* max entropy */
	double ent_back; /* entropy of the background pixels at a given threshold */
	double ent_obj;  /* entropy of the object pixels at a given threshold */
	double omega;
	double * norm_histo = new double[data.size()]; /* normalized histogram */
	double * P1 = new double[data.size()]; /* cumulative normalized histogram */
	double * P2 = new double[data.size()];

	int total =0;
	for (ih = 0; ih < data.size(); ih++ )
		total+=data[ih];

	for (ih = 0; ih < data.size(); ih++ )
		norm_histo[ih] = (double)data[ih]/total;

	P1[0]=norm_histo[0];
	P2[0]=1.0-P1[0];
	for (ih = 1; ih < data.size(); ih++ ){
		P1[ih]= P1[ih-1] + norm_histo[ih];
		P2[ih]= 1.0 - P1[ih];
	}

	/* Determine the first non-zero bin */
	first_bin=0;
	for (ih = 0; ih < data.size(); ih++ ) {
		if ( !(ucas::abs(P1[ih])<2.220446049250313E-16)) {
			first_bin = ih;
			break;
		}
	}

	/* Determine the last non-zero bin */
	last_bin=int(data.size()) - 1;
	for (ih = int(data.size()) - 1; ih >= first_bin; ih-- ) {
		if ( !(ucas::abs(P2[ih])<2.220446049250313E-16)) {
			last_bin = ih;
			break;
		}
	}

	/* Maximum Entropy Thresholding - BEGIN */
	/* ALPHA = 1.0 */
	/* Calculate the total entropy each gray-level
	and find the threshold that maximizes it 
	*/
	threshold =0; // was MIN_INT in original code, but if an empty image is processed it gives an error later on.
	max_ent = 0.0;

	for ( it = first_bin; it <= last_bin; it++ ) {
		/* Entropy of the background pixels */
		ent_back = 0.0;
		for ( ih = 0; ih <= it; ih++ )  {
			if ( data[ih] !=0 ) {
				ent_back -= ( norm_histo[ih] / P1[it] ) * std::log ( norm_histo[ih] / P1[it] );
			}
		}

		/* Entropy of the object pixels */
		ent_obj = 0.0;
		for ( ih = it + 1; ih < data.size(); ih++ ){
			if (data[ih]!=0){
			ent_obj -= ( norm_histo[ih] / P2[it] ) * std::log ( norm_histo[ih] / P2[it] );
			}
		}

		/* Total entropy */
		tot_ent = ent_back + ent_obj;

		// IJ.log(""+max_ent+"  "+tot_ent);

		if ( max_ent < tot_ent ) {
			max_ent = tot_ent;
			threshold = it;
		}
	}
	t_star2 = threshold;

	/* Maximum Entropy Thresholding - END */
	threshold =0; //was MIN_INT in original code, but if an empty image is processed it gives an error later on.
	max_ent = 0.0;
	alpha = 0.5;
	term = 1.0 / ( 1.0 - alpha );
	for ( it = first_bin; it <= last_bin; it++ ) {
		/* Entropy of the background pixels */
		ent_back = 0.0;
		for ( ih = 0; ih <= it; ih++ )
			ent_back += std::sqrt ( norm_histo[ih] / P1[it] );

		/* Entropy of the object pixels */
		ent_obj = 0.0;
		for ( ih = it + 1; ih < data.size(); ih++ )
			ent_obj += std::sqrt ( norm_histo[ih] / P2[it] );

		/* Total entropy */
		tot_ent = term * ( ( ent_back * ent_obj ) > 0.0 ? std::log ( ent_back * ent_obj ) : 0.0);

		if ( tot_ent > max_ent ){
			max_ent = tot_ent;
			threshold = it;
		}
	}

	t_star1 = threshold;

	threshold = 0; //was MIN_INT in original code, but if an empty image is processed it gives an error later on.
	max_ent = 0.0;
	alpha = 2.0;
	term = 1.0 / ( 1.0 - alpha );
	for ( it = first_bin; it <= last_bin; it++ ) {
		/* Entropy of the background pixels */
		ent_back = 0.0;
		for ( ih = 0; ih <= it; ih++ )
			ent_back += ( norm_histo[ih] * norm_histo[ih] ) / ( P1[it] * P1[it] );

		/* Entropy of the object pixels */
		ent_obj = 0.0;
		for ( ih = it + 1; ih < data.size(); ih++ )
			ent_obj += ( norm_histo[ih] * norm_histo[ih] ) / ( P2[it] * P2[it] );

		/* Total entropy */
		tot_ent = term *( ( ent_back * ent_obj ) > 0.0 ? std::log(ent_back * ent_obj ): 0.0 );

		if ( tot_ent > max_ent ){
			max_ent = tot_ent;
			threshold = it;
		}
	}

	t_star3 = threshold;

	/* Sort t_star values */
	if ( t_star2 < t_star1 ){
		tmp_var = t_star1;
		t_star1 = t_star2;
		t_star2 = tmp_var;
	}
	if ( t_star3 < t_star2 ){
		tmp_var = t_star2;
		t_star2 = t_star3;
		t_star3 = tmp_var;
	}
	if ( t_star2 < t_star1 ) {
		tmp_var = t_star1;
		t_star1 = t_star2;
		t_star2 = tmp_var;
	}

	/* Adjust beta values */
	if ( std::abs ( t_star1 - t_star2 ) <= 5 )  {
		if ( std::abs ( t_star2 - t_star3 ) <= 5 ) {
			beta1 = 1;
			beta2 = 2;
			beta3 = 1;
		}
		else {
			beta1 = 0;
			beta2 = 1;
			beta3 = 3;
		}
	}
	else {
		if ( std::abs ( t_star2 - t_star3 ) <= 5 ) {
			beta1 = 3;
			beta2 = 1;
			beta3 = 0;
		}
		else {
			beta1 = 1;
			beta2 = 2;
			beta3 = 1;
		}
	}
	//IJ.log(""+t_star1+" "+t_star2+" "+t_star3);
	/* Determine the optimal threshold value */
	omega = P1[t_star3] - P1[t_star1];
	opt_threshold = (int) (t_star1 * ( P1[t_star1] + 0.25 * omega * beta1 ) + 0.25 * t_star2 * omega * beta2  + t_star3 * ( P2[t_star3] + 0.25 * omega * beta3 ));

	delete[] norm_histo;
	delete[] P1;
	delete[] P2;

	return opt_threshold;
}

// Kapur J.N., Sahoo P.K., and Wong A.K.C. (1985) 
// "A New Method for Gray-Level Picture Thresholding Using the Entropy of the Histogram" Graphical Models and Image Processing, 29(3): 273-285
int ucas::getMaxEntropyAutoThreshold(const std::vector<int> & data) throw (ucas::Error)
{
	int threshold=-1;
	int ih, it;
	int first_bin;
	int last_bin;
	double tot_ent;  /* total entropy */
	double max_ent;  /* max entropy */
	double ent_back; /* entropy of the background pixels at a given threshold */
	double ent_obj;  /* entropy of the object pixels at a given threshold */
	double * norm_histo = new double[data.size()]; /* normalized histogram */
	double * P1 = new double[data.size()]; /* cumulative normalized histogram */
	double * P2 = new double[data.size()];

	int total =0;
	for (ih = 0; ih < data.size(); ih++ )
		total+=data[ih];

	for (ih = 0; ih < data.size(); ih++ )
		norm_histo[ih] = (double)data[ih]/total;

	P1[0]=norm_histo[0];
	P2[0]=1.0-P1[0];
	for (ih = 1; ih < data.size(); ih++ ){
		P1[ih]= P1[ih-1] + norm_histo[ih];
		P2[ih]= 1.0 - P1[ih];
	}

	/* Determine the first non-zero bin */
	first_bin=0;
	for (ih = 0; ih < data.size(); ih++ ) {
		if ( !(ucas::abs(P1[ih])<2.220446049250313E-16)) {
			first_bin = ih;
			break;
		}
	}

	/* Determine the last non-zero bin */
	last_bin=int(data.size()) - 1;
	for (ih = int(data.size()) - 1; ih >= first_bin; ih-- ) {
		if ( !(ucas::abs(P2[ih])<2.220446049250313E-16)) {
			last_bin = ih;
			break;
		}
	}

	// Calculate the total entropy each gray-level
	// and find the threshold that maximizes it 
	max_ent = -std::numeric_limits<double>::max();

	for ( it = first_bin; it <= last_bin; it++ ) {
		/* Entropy of the background pixels */
		ent_back = 0.0;
		for ( ih = 0; ih <= it; ih++ )  {
			if ( data[ih] !=0 ) {
				ent_back -= ( norm_histo[ih] / P1[it] ) * std::log ( norm_histo[ih] / P1[it] );
			}
		}

		/* Entropy of the object pixels */
		ent_obj = 0.0;
		for ( ih = it + 1; ih < data.size(); ih++ ){
			if (data[ih]!=0){
				ent_obj -= ( norm_histo[ih] / P2[it] ) * std::log ( norm_histo[ih] / P2[it] );
			}
		}

		/* Total entropy */
		tot_ent = ent_back + ent_obj;

		// IJ.log(""+max_ent+"  "+tot_ent);
		if ( max_ent < tot_ent ) {
			max_ent = tot_ent;
			threshold = it;
		}
	}

	delete[] norm_histo;
	delete[] P1;
	delete[] P2;

	return threshold;
}


// Kittler and J. Illingworth, "Minimum error thresholding," Pattern Recognition, vol. 19, pp. 41-47, 1986.
// C. A. Glasbey, "An analysis of histogram-based thresholding algorithms," CVGIP: Graphical Models and Image Processing, vol. 55, pp. 532-537, 1993.
int ucas::getMinErrorIThreshold(const std::vector<int> & data) throw (ucas::Error)
{
	//Initial estimate for the threshold is found with the MEAN algorithm.
	int threshold =  getMeanThreshold(data); 
	int Tprev =-2;
	double mu, nu, p, q, sigma2, tau2, w0, w1, w2, sqterm, temp;
	while (threshold!=Tprev)
	{
		//Calculate some statistics.
		mu = B(data, threshold)/A(data, threshold);
		nu = (B(data)-B(data, threshold))/(A(data)-A(data, threshold));
		p = A(data, threshold)/A(data);
		q = (A(data)-A(data, threshold)) / A(data);
		sigma2 = C(data, threshold)/A(data, threshold)-(mu*mu);
		tau2 = (C(data)-C(data, threshold)) / (A(data)-A(data, threshold)) - (nu*nu);


		//The terms of the quadratic equation to be solved.
		w0 = 1.0/sigma2-1.0/tau2;
		w1 = mu/sigma2-nu/tau2;
		w2 = (mu*mu)/sigma2 - (nu*nu)/tau2 + std::log10((sigma2*(q*q))/(tau2*(p*p)));

		//If the next threshold would be imaginary, return with the current one.
		sqterm = (w1*w1)-w0*w2;
		if (sqterm < 0) {
			ucas::warning("MinError(I): not converging. Try \'Ignore black/white\' options");
			return threshold;
		}

		//The updated threshold is the integer part of the solution of the quadratic equation.
		Tprev = threshold;
		temp = (w1+std::sqrt(sqterm))/w0;

		if ( ucas::is_nan(temp)) {
			ucas::warning("MinError(I): NaN, not converging. Try \'Ignore black/white\' options");
			threshold = Tprev;
		}
		else
			threshold =(int) std::floor(temp);
	}
	return threshold;
}

// Iterative procedure based on the isodata algorithm [T.W. Ridler, S. Calvard, Picture 
// thresholding using an iterative selection method, IEEE Trans. System, Man and Cybernetics, SMC-8 (1978) 630-632.] 
int ucas::getIsoDataAutoThreshold(const std::vector<int> & data) throw (ucas::Error)
{
	// IMPLEMENTATION: taken from ImageJ
	int i, l, toth, totl, h, g=0;
	for (i = 1; i < data.size(); i++){
		if (data[i] > 0){
			g = i + 1;
			break;
		}
	}
	while (true){
		l = 0;
		totl = 0;
		for (i = 0; i < g; i++) {
			totl = totl + data[i];
			l = l + (data[i] * i);
		}
		h = 0;
		toth = 0;
		for (i = g + 1; i < data.size(); i++){
			toth += data[i];
			h += (data[i]*i);
		}
		if (totl > 0 && toth > 0){
			l /= totl;
			h /= toth;
			if (g == (int) round((l + h) / 2.0))
				break;
		}
		g++;
		if (g >data.size()-2){
			ucas::warning("IsoData Threshold not found.");
			return -1;
		}
	}
	return g;
}

int ucas::getTriangleAutoThreshold(const std::vector<int> & data2) throw (ucas::Error)
{   
	std::vector<int> data = data2;

	// find min and max
	int min = 0, dmax=0, max = 0, min2=0;
	for (int i = 0; i < data.size(); i++) {
		if (data[i]>0){
			min=i;
			break;
		}
	}
	if (min>0) min--; // line to the (p==0) point, not to data[min]

	// The Triangle algorithm cannot tell whether the data is skewed to one side or another.
	// This causes a problem as there are 2 possible thresholds between the max and the 2 extremes
	// of the histogram.
	// Here I propose to find out to which side of the max point the data is furthest, and use that as
	//  the other extreme.
	for (int i = int(data.size()) - 1; i >0; i-- ) {
		if (data[i]>0){
			min2=i;
			break;
		}
	}
	if (min2<data.size() - 1) min2++; // line to the (p==0) point, not to data[min]

	for (int i =0; i < data.size(); i++) {
		if (data[i] >dmax) {
			max=i;
			dmax=data[i];
		}
	}
	// find which is the furthest side
	//IJ.log(""+min+" "+max+" "+min2);
	bool inverted = false;
	if ((max-min)<(min2-max)){
		// reverse the histogram
		//IJ.log("Reversing histogram.");
		inverted = true;
		int left  = 0;          // index of leftmost element
		int right = int(data.size()) - 1; // index of rightmost element
		while (left < right) {
			// exchange the left and right elements
			int temp = data[left]; 
			data[left]  = data[right]; 
			data[right] = temp;
			// move the bounds toward the center
			left++;
			right--;
		}
		min=int(data.size()) - 1-min2;
		max=int(data.size()) - 1-max;
	}

	if (min == max){
		//IJ.log("Triangle:  min == max.");
		return min;
	}

	// describe line by nx * x + ny * y - d = 0
	double nx, ny, d;
	// nx is just the max frequency as the other point has freq=0
	nx = data[max];   //-min; // data[min]; //  lowest value bmin = (p=0)% in the image
	ny = min - max;
	d = std::sqrt(nx * nx + ny * ny);
	nx /= d;
	ny /= d;
	d = nx * min + ny * data[min];

	// find split point
	int split = min;
	double splitDistance = 0;
	for (int i = min + 1; i <= max; i++) {
		double newDistance = nx * i + ny * data[i] - d;
		if (newDistance > splitDistance) {
			split = i;
			splitDistance = newDistance;
		}
	}
	split--;

	if (inverted) {
		// The histogram might be used for something else, so let's reverse it back
		int left  = 0; 
		int right = int(data.size()) - 1;
		while (left < right) {
			int temp = data[left]; 
			data[left]  = data[right]; 
			data[right] = temp;
			left++;
			right--;
		}
		return (int(data.size()) - 1-split);
	}
	else
		return split;
}

// output(x,y) = maxval if input(x,y) > threshold, 0 otherwise
// *** WARNING *** : binarization is done in place (8-bit output)
cv::Mat ucas::binarize(cv::Mat & image, int threshold) throw (ucas::Error)
{
	// checks
	if(!image.data)
		throw ucas::Error("in binarize(): invalid image");
	if(image.channels() != 1)
		throw ucas::Error("in binarize(): unsupported number of channels");
	if(image.depth() != CV_8U && image.depth() != CV_16U)
		throw ucas::Error("in binarize(): unsupported bitdepth: only 8- and 16-bit grayscale image are supported");

	// binarization
	if(image.depth() == CV_8U)
	{
		for(int i=0; i<image.rows; i++)
		{
			ucas::uint8* punt = image.ptr<ucas::uint8>(i);
			for(int j=0; j<image.cols; j++)
				punt[j] = punt[j] > threshold ? 255 : 0;
		}
	}
	else if(image.depth() == CV_16U)
	{
		for(int i=0; i<image.rows; i++)
		{
			ucas::uint16* punt = image.ptr<ucas::uint16>(i);
			for(int j=0; j<image.cols; j++)
				punt[j] = punt[j] > threshold ? 255 : 0;
		}
	}

	// conversion
	if(image.depth() != CV_8U)
		image.convertTo(image, CV_8U);

	return image;
}

//return image histogram (the number of bins is automatically set to 2^imgdepth if not provided)
std::vector<int> ucas::histogram(const cv::Mat & image, int bins /*= -1 */) throw (ucas::Error)
{
	// checks
	if(!image.data)
		throw ucas::Error("in histogram(): invalid image");
	if(image.channels() != 1)
		throw ucas::Error("in histogram(): unsupported number of channels");

	// the number of gray levels
	int grayLevels  = static_cast<int>( std::pow(2, ucas::imdepth(image.depth())) );

	// computing the number of bins
	bins = bins == -1 ? grayLevels : bins;

	// input-output parameters of cv::calcHist function
	int histSize[1]  = {bins};				// number of bins
	int channels[1]  = {0};					// only 1 channel used here
	float hranges[2] = {0.0f, static_cast<float>(grayLevels)};	// [min, max) pixel levels to take into account
	const float* ranges[1] = {hranges};		// [min, max) pixel levels for all the images (here only 1 image)
	cv::MatND histo;						// where the output histogram is stored

	// histogram computation
	cv::calcHist(&image, 1, channels, cv::Mat(), histo, 1, histSize, ranges);

	// conversion from MatND to vector<int>
	std::vector<int> hist;
	for(int i=0; i<bins; i++)
		hist.push_back(static_cast<int>(histo.at<float>(i)));

	return hist;
}

// bracket the histogram to the range that holds data
std::vector<int> ucas::compressHistogram(std::vector<int> &histo, int & minbin)
{
	int maxbin=-1;
	minbin = -1;
	for (int i=0; i<histo.size(); i++)
		if (histo[i]>0) 
			maxbin = i;
	for (int i=int(histo.size())-1; i>=0; i--)
		if (histo[i]>0) 
			minbin = i;

	std::vector<int> data2(maxbin-minbin+1);
	for (int i=minbin; i<=maxbin; i++)
		data2[i-minbin]= histo[i];

	return data2;
}