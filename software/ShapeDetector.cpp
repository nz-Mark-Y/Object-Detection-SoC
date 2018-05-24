//============================================================================

/*
 * Simple example of object detection which detects geometric shapes by use of
 * OpenCV libraries. The source image is initially converted to gray-scale image
 * and is blurred to remove the noise. Two different options of Gaussian blure
 * and Box blur are examined. Using findContours and approxPolyDP, contours within
 * the image are detected and approximated. Threshold image or edge image should be
 * input to findcontours function. Threshold image, Sobel edge image followed by
 * thresholding, and canny edge image are used as the input to findcontours function.
 *
 */



#include <iostream>
#include <stdlib.h>
#include <opencv2\imgproc\types_c.h>
#include <opencv2\highgui\highgui.hpp>
#include <opencv2\imgproc\imgproc.hpp>


using namespace cv;
using namespace std;

Mat src, src_gray, src_blur, src_gaussian, src_thresh;
Mat src_sobel, sobel_thresh, src_canny, dst;
Mat grad_x, grad_y;
Mat abs_grad_x, abs_grad_y;

int thresh = 110;
int max_thresh = 255;
int scale = 1;
int delta = 0;
int ddepth = CV_16S;

std::vector<cv::Point> approx;
int vtc;


/** Helper function to display text in the center of a contour **/
void setLabel(cv::Mat& im, const std::string label, std::vector<cv::Point>& contour)
{
	int fontface = cv::FONT_HERSHEY_SIMPLEX;
	double scale = 0.4;
	int thickness = 1;
	int baseline = 0;

	cv::Size text = cv::getTextSize(label, fontface, scale, thickness, &baseline);
	cv::Rect r = cv::boundingRect(contour);

	cv::Point pt(r.x + ((r.width - text.width) / 2), r.y + ((r.height + text.height) / 2));
	cv::rectangle(im, pt + cv::Point(0, baseline), pt + cv::Point(text.width, -text.height), CV_RGB(255,255,255), -1);
	cv::putText(im, label, pt, fontface, scale, CV_RGB(0,0,0), thickness, 8);
}

/** @function main */
int main( int argc, char* argv[] )
{

	if(argc<3)
	{
		printf( "Usage: main <image-file-name> <option>\n" );
		printf( "option:\n"
				"\t t -Threshold\n"
				"\t s -Sobel\n"
				"\t c -Canny\n" );
		exit(0);
	}

	string option = argv[2];

	/** Load source image **/
    printf( "Reading image\n" );
    src = imread( argv[1], 1 );
    if (src.empty())
    {
    	printf( "Could not load image file: %s\n",argv[1] );
    	exit(0);
    }

    /** Convert image to gray and blur it **/
    cvtColor( src, src_gray, CV_BGR2GRAY );
    GaussianBlur( src_gray, src_gaussian, Size(3,3), 0, 0, BORDER_DEFAULT );
    blur( src_gray, src_blur, Size(3,3) );


    /** Generate threshold result for findcontours where option argument is t **/
    threshold(src_gaussian, src_thresh, thresh, max_thresh, 1);


    /** Generate Sobel result for findcontours where option argument is s **/
    /** Gradient X **/
    Sobel( src_gaussian, grad_x, ddepth, 1, 0, 3, scale, delta, BORDER_REPLICATE );
    convertScaleAbs( grad_x, abs_grad_x );
    /** Gradient Y **/
    Sobel( src_gaussian, grad_y, ddepth, 0, 1, 3, scale, delta, BORDER_REPLICATE );
    convertScaleAbs( grad_y, abs_grad_y );
    /** Total Gradient (approximate) **/
    addWeighted( abs_grad_x, 0.5, abs_grad_y, 0.5, 0, src_sobel );
    threshold(src_sobel, sobel_thresh, thresh, max_thresh, 0);

    /** Generate Sobel result for findcontours where option argument is c **/
    Canny(src_gaussian, src_canny, 80, 240, 3);

    std::vector<std::vector<cv::Point> > contours;
    if (option == "t")
    	cv::findContours(src_thresh.clone(), contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
    else if (option == "s")
        cv::findContours(sobel_thresh.clone(), contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
    else
        cv::findContours(src_canny.clone(), contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

    printf("%d contours detected in the image!\n", contours.size());




	dst = src.clone();

	for (unsigned int i = 0; i < contours.size(); i++)
	{
		/** Approximate contour with accuracy proportional to the contour perimeter **/
		cv::approxPolyDP(cv::Mat(contours[i]), approx, cv::arcLength(cv::Mat(contours[i]), true)*0.02, true);
		/** Skip small or non-convex objects **/
		if (std::fabs(cv::contourArea(contours[i])) < 100 || !cv::isContourConvex(approx))
			continue;

		/** Number of vertices of polygonal curve **/
		vtc = approx.size();
		if (vtc == 3)
			setLabel(dst, "TRI", contours[i]);    	// Triangles
		else if (vtc == 4)
			setLabel(dst, "RECT", contours[i]);		// Rectangles
		else if (vtc == 5 )
			setLabel(dst, "PENTA", contours[i]);	// Pentagons
		else if (vtc == 6)
			setLabel(dst, "HEXA", contours[i]);		// Hexagons
		else
		{
			/** Detect and label circles **/
			double area = cv::contourArea(contours[i]);
			cv::Rect r = cv::boundingRect(contours[i]);
			int radius = r.width / 2;

			if (std::abs(1 - ((double)r.width / r.height)) <= 0.2 &&
				std::abs(1 - (area / (CV_PI * std::pow(radius, 2)))) <= 0.2)
				setLabel(dst, "CIR", contours[i]);
		}
	}


	printf( "Threshold results saved in thresh.bmp\n" );
	imwrite( "sobel.bmp", src_thresh );

	printf( "Sobel edge detection results saved in sobel.bmp\n" );
	imwrite( "sobel.bmp", src_sobel );

	printf( "Thresholded Sobel edge detection results saved in sobelthresh.bmp\n" );
	imwrite( "sobelthresh.bmp", sobel_thresh );

	printf( "Canny edge detection results saved in canny.bmp\n" );
	imwrite( "canny.bmp", src_canny );

	printf( "Shape detection results saved in result.bmp\n" );
	imwrite( "result.bmp", dst );

	return(0);
}

