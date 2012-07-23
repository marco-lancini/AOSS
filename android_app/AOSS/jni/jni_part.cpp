////////////////////////////////////////////////////////////////////////////////
//
// AOSS - Marco Lancini (www.marcolancini.it)
//
//
// Copyright (C) 2012 Marco Lancini
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
////////////////////////////////////////////////////////////////////////////////
#include <jni.h>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <vector>

using namespace std;
using namespace cv;


////////////////////////////////////////////////////////////////////////////////
// CONSTANTS ///////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
const int threshold_value  = 45;
const int max_BINARY_value = 255;
const int erosion_size = 3;
const int dilation_size = 3;
const float thresh_area = 1;

Scalar green = Scalar( 0, 255, 0 );


////////////////////////////////////////////////////////////////////////////////
// SKIN DETECTION //////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void skinPixels( Mat *imgBGR, Mat *imgSkin, Mat *imgHSV, vector<Mat> *hsv_planes, Mat *planeH, Mat *planeS, Mat *planeV )
{
	// Returns an image that is white in corrispondence of skin pixels, black elsewhere

    // Convert the image to HSV colors
	*imgHSV = Mat::zeros( imgBGR->size(), CV_8UC3 );
	cvtColor( *imgBGR, *imgHSV, CV_BGR2HSV );

	// Get the separate HSV color components of the color input image
	split( *imgHSV, *hsv_planes );

	*planeH = hsv_planes->at(0);      // Hue component
	*planeS = hsv_planes->at(1);      // Saturation component
	*planeV = hsv_planes->at(2);      // Brightness component

    // Detect which pixels in each of the H, S and V channels are probably skin pixels
    // Assume that skin has a Hue between 0 to 18 (out of 180), and Saturation above 50, and Brightness above 80
    threshold( *planeH, *planeH, 18, 255, THRESH_BINARY_INV );
    threshold( *planeS, *planeS, 50, 255, THRESH_BINARY_INV );
    threshold( *planeV, *planeV, 80, 255, THRESH_BINARY_INV );

    // Combine all 3 thresholded color components
    // so that an output pixel will only be white if the H, S and V pixels were also white
    *imgSkin = Mat::zeros( imgHSV->size(), CV_8UC1 );  // Greyscale output image
    bitwise_and( *planeH, *planeS, *imgSkin );		   // imageSkin = H {BITWISE_AND} S
    bitwise_and( *imgSkin, *planeV, *imgSkin );		   // imageSkin = H {BITWISE_AND} S {BITWISE_AND} V

    bitwise_not( *imgSkin, *imgSkin );
}



////////////////////////////////////////////////////////////////////////////////
// SELECT BIGGEST //////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void selectBiggest( int *firstidx, int *secondidx, vector<Moments> *mu )
{
	// Returns the indexes of the 2 biggest contours of the image

	*firstidx  = -1;
	*secondidx = -1;

	float firstdim  = -1;
	float seconddim = -1;
	float temparea;

	// Init ////////////////////////////////////////////////////////////////////
	if( mu->at(0).m00 > mu->at(1).m00 )
	{
		*firstidx = 0;
		firstdim = mu->at(0).m00;

		*secondidx = 1;
		seconddim = mu->at(1).m00;;
	}
	else
	{
		*firstidx = 1;
		firstdim = mu->at(1).m00;

		*secondidx = 0;
		seconddim = mu->at(0).m00;
	}

	// Iterate /////////////////////////////////////////////////////////////////
	for( int i = 2; i < mu->size(); i++ )
	{
		temparea = mu->at(i).m00;

		if( temparea > firstdim ) {
			*secondidx = *firstidx;
			seconddim = firstdim;

			*firstidx = i;
			firstdim = temparea;
		}
		else if( temparea > seconddim ) {
			*secondidx = i;
			seconddim = temparea;
		}
	}
}




////////////////////////////////////////////////////////////////////////////////
// JNI /////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
extern "C" {
JNIEXPORT double JNICALL Java_org_opencv_aoss_AOSSView_analyzeFrame( JNIEnv* env, jobject thiz, jlong addrGray, jlong addrRgba, jlong addrSkin, jlong addrImgHSV, jlong addrPlaneH, jlong addrPlaneS, jlong addrPlaneV, jlong addrImgSkin, jlong addrEl1, jlong addrEl2 )
{	
	// Reference ///////////////////////////////////////////////////////////////
	Mat* gray_image = (Mat*)addrGray;
	Mat* tracking   = (Mat*)addrRgba;
	Mat* skin       = (Mat*)addrSkin;
	Mat* imgHSV     = (Mat*)addrImgHSV;
	Mat* planeH		= (Mat*)addrPlaneH;
	Mat* planeS 	= (Mat*)addrPlaneS;
	Mat* planeV 	= (Mat*)addrPlaneV;
	Mat* imgSkin 	= (Mat*)addrImgSkin;
	Mat* el1 		= (Mat*)addrEl1;
	Mat* el2 		= (Mat*)addrEl2;
	
	int firstidx, secondidx;
	float area;
	vector<Mat> hsv_planes;

	// Blur ////////////////////////////////////////////////////////////////////
	blur( *gray_image, *gray_image, Size(3,3) );

	// Threshold ///////////////////////////////////////////////////////////////
	threshold( *gray_image, *gray_image, threshold_value, max_BINARY_value, THRESH_BINARY_INV );

	// Skin detection and subtraction //////////////////////////////////////////
	skinPixels( tracking, imgSkin, imgHSV, &hsv_planes, planeH, planeS, planeV );
	subtract( *gray_image, *imgSkin, *gray_image );

	// Erode ///////////////////////////////////////////////////////////////////
	*el1 = getStructuringElement( MORPH_RECT, Size( 2*erosion_size + 1, 2*erosion_size+1 ), Point( erosion_size, erosion_size ) );
	erode( *gray_image, *gray_image, *el1 );

	// Dilate //////////////////////////////////////////////////////////////////
	*el2 = getStructuringElement( MORPH_RECT, Size( 2*dilation_size + 1, 2*dilation_size+1 ), Point( dilation_size, dilation_size ) );
	dilate( *gray_image, *gray_image, *el2 );

	// Find contours ///////////////////////////////////////////////////////////
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;
	findContours( *gray_image, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );

	// ApproxPoly + BoundingRect + Moments + Mass Centers //////////////////////
	vector<vector<Point> > contours_poly( contours.size() );
	vector<Rect> boundRect( contours.size() );
	vector<Moments> mu(contours.size() );
	vector<Point2f> mc( contours.size() );

	for( int i = 0; i < contours.size(); i++ )
	{
		// Discard contours with area < threshold
		area = contourArea( contours[i] );
		if( area > thresh_area )
		{
			// Approximate contours to polygons
			approxPolyDP( Mat(contours[i]), contours_poly[i], 3, true );

			// Get bounding rects
			boundRect[i] = boundingRect( Mat(contours_poly[i]) );

			// Get the moments
			mu[i] = moments( contours_poly[i], false );

			// Get the mass centers
			mc[i] = Point2f( mu[i].m10/mu[i].m00 , mu[i].m01/mu[i].m00 );
		}
	}

	// Select 2 contours, whose moments have the biggest area //////////////////
	selectBiggest( &firstidx, &secondidx, &mu );

	////////////////////////////////////////////////////////////////////////////
	// Draw tracking ///////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////
	circle( *tracking, mc[firstidx], 5, green, -1, 8, 0 );
	circle( *tracking, mc[secondidx], 5, green, -1, 8, 0 );

	int p1x = mu[firstidx].m10/mu[firstidx].m00;
	int p1y = mu[firstidx].m01/mu[firstidx].m00;
	int p2x = mu[secondidx].m10/mu[secondidx].m00;
	int p2y = mu[secondidx].m01/mu[secondidx].m00;
	
	return sqrt( pow( (p1x - p2x), 2) + pow( (p1y - p2y), 2) );	
}

}
