#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sstream>
#include <vector>


int edgeThresh = 1;
int thresh = 100;
int max_thresh = 255;
int ratio = 2;
int kernel_size = 3;

cv::RNG rng(12345);

int main( int argc, char** argv ) {
	/// Load an image
	cv::Mat src, greyIm, histeqIm;

	src = cv::imread( argv[1] );

	if( !src.data ) {
		printf("Input file? No? ouuuupsss thooooorryyyyy\n");
		return -1;
	}


	cv::Size s = src.size();
	int rows = s.height;
	int cols = s.width;
	// Setup a rectangle to define your region of interest
	cv::Rect myROI(0, rows/2, cols, rows/2);

	// Crop the full image to that image contained by the rectangle myROI
	// Note that this doesn't copy the data
	cv::Mat croppedImage = src(myROI);

	cv::imwrite("output/1_low_half.jpg", croppedImage);

	cv::cvtColor(croppedImage, greyIm, cv::COLOR_BGR2GRAY);

    cv::Size crop_size = croppedImage.size();
    int crop_rows = crop_size.height;
    int crop_cols = crop_size.width;


	cv::imwrite("output/2_grey_scale.jpg", greyIm);

	cv::equalizeHist( greyIm, histeqIm );

	cv::imwrite("output/3_hist_eq.jpg", histeqIm);	




	std::vector<std::vector<cv::Point> > contours;
    std::vector<cv::Vec4i> hierarchy;

    // Reduce noise with kernel 3x3
    cv::Mat blurIm;
    blur(histeqIm, blurIm, cv::Size(3,3));

	cv::imwrite("output/4_blur.jpg", blurIm);

    // Canny detector
    cv::Mat edgesIm;
    Canny(blurIm, edgesIm, thresh, thresh*ratio, kernel_size);

    cv::imwrite("output/5_edge.jpg", edgesIm);
    
    // Find contours
    cv::findContours(edgesIm, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE, cv::Point(0,0));

    // Approximate contours to polygons + get bounding rects and circles
    std::vector<std::vector<cv::Point> > contours_poly(contours.size());
    std::vector<cv::Rect> boundRect(contours.size());
    std::vector<cv::Point2f>center(contours.size());
    std::vector<float>radius(contours.size());

    for (int i = 0; i < contours.size(); i++) {
        cv::approxPolyDP(cv::Mat(contours[i]), contours_poly[i], 3, true);
        boundRect[i] = cv::boundingRect(cv::Mat(contours_poly[i]));
        cv::minEnclosingCircle((cv::Mat)contours_poly[i], center[i], radius[i]);
    }

    // Draw contours
    int j=0;
    cv::Mat drawing = cv::Mat::zeros(edgesIm.size(), CV_8UC3);
    cv::Mat piece[5], hsvIm[5];
    for (int i = 0; i < contours.size(); i++) {
        if (!((boundRect[i].height >= boundRect[i].width/5) && (boundRect[i].height <= boundRect[i].width/2) 
            && boundRect[i].height<=crop_rows/4 && boundRect[i].width<=crop_cols/2   
            && boundRect[i].height>=crop_rows/10 && boundRect[i].width>=crop_cols/6)) 
        continue;

        cv::Rect roi = boundRect[i];
        piece[j] = croppedImage(roi);
        imwrite("output/contour"+std::to_string(j)+".jpg", piece[j]);
        j++;

        cv::Scalar color = cv::Scalar(rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255));
        cv::drawContours(drawing, contours, i, color, 2, 8, hierarchy, 0, cv::Point());
        cv::rectangle(drawing, boundRect[i].tl(), boundRect[i].br(), color, 2, 8, 0);
        //circle(drawing, center[i], (int)radius[i], color, 2, 8, 0);  
    }

    imwrite("output/6_contours.jpg", drawing);



    int h_bins = 50; int s_bins = 60;
    int histSize[] = { h_bins, s_bins };

    float h_ranges[] = { 0, 180 };
    float s_ranges[] = { 0, 256 };

    const float* ranges[] = { h_ranges, s_ranges };

    int channels[] = { 0, 1 };

    cv::Mat hist[5];

    for (int i=0; i<j; i++){
        cvtColor(piece[i], hsvIm[i], cv::COLOR_BGR2HSV);
        imwrite("output/hsvIm"+std::to_string(i)+".jpg", hsvIm[i]);

        calcHist( &hsvIm[i], 1, channels, cv::Mat(), hist[i], 2, histSize, ranges, true, false );
        //normalize( hsvIm[i], hsvIm[i], 0, 1, cv::NORM_MINMAX, -1, cv::Mat() );
    }












	return 0;
}