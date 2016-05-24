#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sstream>
#include <vector>

using namespace cv;

int edgeThresh = 1;
int thresh = 100;
int max_thresh = 255;
int ratio = 2;
int kernel_size = 3;

RNG rng(12345);


/*Rect deleteBorder(InputArray _src,int size){
    Mat src = _src.getMat();
    Mat sbl_x, sbl_y;
    int ksize = 2 * size + 1;
    Sobel(src, sbl_x, CV_32FC1, 2, 0, ksize);
    Sobel(src, sbl_y, CV_32FC1, 0, 2, ksize);
    Mat sum_img = sbl_x + sbl_y;

    Mat gray;
    normalize(sum_img, gray, 0, 255, CV_MINMAX, CV_8UC1);

    Mat row_proj, col_proj;
    reduce(gray, row_proj, 1, CV_REDUCE_AVG, CV_8UC1);
    reduce(gray, col_proj, 0, CV_REDUCE_AVG, CV_8UC1);
    Sobel(row_proj, row_proj, CV_8UC1, 0, 2, 3);
    Sobel(col_proj, col_proj, CV_8UC1, 2, 0, 3);


    Point peak_pos;
    int half_pos = row_proj.total() / 2;
    Rect result;

    minMaxLoc(row_proj(Range(0,half_pos), Range(0, 1)), 0, 0, 0, &peak_pos);
    result.y = peak_pos.y;
    minMaxLoc(row_proj(Range(half_pos, row_proj.total()), Range(0, 1)), 0, 0, 0, &peak_pos);
    result.height = (peak_pos.y + half_pos - result.y);

    half_pos = col_proj.total() / 2;
    minMaxLoc(col_proj(Range(0, 1), Range(0, half_pos)), 0, 0, 0, &peak_pos);
    result.x = peak_pos.x;
    minMaxLoc(col_proj(Range(0, 1), Range(half_pos, col_proj.total())), 0, 0, 0, &peak_pos);
    result.width = (peak_pos.x + half_pos - result.x);

    return result;

}*/


int main( int argc, char** argv ) {
	/// Load an image
	Mat src, greyIm, histeqIm;

	src = imread( argv[1] );

	if( !src.data ) {
		printf("Input file? No? ouuuupsss thooooorryyyyy\n");
		return -1;
	}


	Size s = src.size();
	int rows = s.height;
	int cols = s.width;
	// Setup a rectangle to define your region of interest
	Rect myROI(0, rows/2, cols, rows/2);

	// Crop the full image to that image contained by the rectangle myROI
	// Note that this doesn't copy the data
	Mat croppedImage = src(myROI);

	imwrite("output/1_low_half.jpg", croppedImage);

	cvtColor(croppedImage, greyIm, COLOR_BGR2GRAY);

    Size crop_size = croppedImage.size();
    int crop_rows = crop_size.height;
    int crop_cols = crop_size.width;


	imwrite("output/2_grey_scale.jpg", greyIm);

	equalizeHist( greyIm, histeqIm );

	imwrite("output/3_hist_eq.jpg", histeqIm);	


	std::vector<std::vector<Point> > contours;
    std::vector<Vec4i> hierarchy;

    // Reduce noise with kernel 3x3
    Mat blurIm;
    blur(histeqIm, blurIm, Size(3,3));

	imwrite("output/4_blur.jpg", blurIm);

    // Canny detector
    Mat edgesIm;
    Canny(blurIm, edgesIm, thresh, thresh*ratio, kernel_size);

    imwrite("output/5_edge.jpg", edgesIm);
    
    // Find contours
    findContours(edgesIm, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0,0));

    // Approximate contours to polygons + get bounding rects and circles
    std::vector<std::vector<Point> > contours_poly(contours.size());
    std::vector<Rect> boundRect(contours.size());
    std::vector<Point2f>center(contours.size());
    std::vector<float>radius(contours.size());

    for (int i = 0; i < contours.size(); i++) {
        approxPolyDP(Mat(contours[i]), contours_poly[i], 3, true);
        boundRect[i] = boundingRect(Mat(contours_poly[i]));
        minEnclosingCircle((Mat)contours_poly[i], center[i], radius[i]);
    }

    // Draw contours
    int j=0;
    Mat drawing = Mat::zeros(edgesIm.size(), CV_8UC3);
    Mat piece[5], hsvIm[5];
    for (int i = 0; i < contours.size(); i++) {
        if (!((boundRect[i].height >= boundRect[i].width/5) && (boundRect[i].height <= boundRect[i].width/2) 
            && boundRect[i].height<=crop_rows/4 && boundRect[i].width<=crop_cols/2   
            && boundRect[i].height>=crop_rows/10 && boundRect[i].width>=crop_cols/6)) 
            continue;



        Rect roi = boundRect[i];
        piece[j] = croppedImage(roi);
        imwrite("output/contour"+std::to_string(j)+".jpg", piece[j]);
        j++;

        /*Mat tmp_gray;
        //cvtColor(piece[j], tmp_gray, CV_BGR2GRAY);
        Rect r = deleteBorder(piece[j], 2);
        rectangle(piece[j], r, CV_RGB(0, 255, 0), 2);
        imwrite("output/deleted_borders"+std::to_string(j)+".jpg", piece[j]);*/

        Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255));
        drawContours(drawing, contours, i, color, 2, 8, hierarchy, 0, Point());
        rectangle(drawing, boundRect[i].tl(), boundRect[i].br(), color, 2, 8, 0);
        //circle(drawing, center[i], (int)radius[i], color, 2, 8, 0);  
    }

    imwrite("output/6_contours.jpg", drawing);


	return 0;
}