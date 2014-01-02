////////////////////////////////////////////////////////////////////////////////
//
// AOSS Vision Module - Marco Lancini (www.marcolancini.it)
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
#include <iostream>
#include <string>
#include <iomanip>
#include <sstream>

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace std;
using namespace cv;



////////////////////////////////////////////////////////////////////////////////
// CONSTANTS ///////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
const char* WIN_UT    = "Under Test";
const char* WIN_SK    = "Skin Filter";
const char* WIN_SQ    = "Selected Countours";
const char* WIN_CT    = "Tracking";
const char* WIN_CHART = "Bar Chart";

const int threshold_value  = 45;
const int max_BINARY_value = 255;
const int erosion_size     = 3;
const int dilation_size    = 3;
const int thresh_canny     = 150;
const float thresh_area    = 500;

Scalar white = Scalar( 255, 255, 255 );
Scalar black = Scalar( 0, 0, 0 );
Scalar blue  = Scalar( 255, 0, 0 );
Scalar green = Scalar( 0, 255, 0 );
Scalar red   = Scalar( 0, 0, 255 );

Scalar peach 	  = Scalar( 185, 218, 255 );
Scalar bisque 	  = Scalar( 158, 183, 205 );
Scalar steel 	  = Scalar( 180, 130, 70 );
Scalar cadet 	  = Scalar( 160, 158, 95 );
Scalar orange     = Scalar( 0, 69, 255 );
Scalar darkorange = Scalar( 0, 140, 255 );



////////////////////////////////////////////////////////////////////////////////
// PROTOTYPES //////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void analyzeFrame( Mat *frameUnderTest, Mat *gray_image, Mat *objects, Mat *tracking, Mat *chart,
				   Mat *skin, Mat *imgHSV, Mat *planeH, Mat *planeS, Mat *planeV, Mat *imgSkin,
				   vector<Mat> *hsv_planes, Mat *el1, Mat *el2,
				   int *firstidx, int *secondidx,
				   int *p1x, int *p1y, int *p2x, int *p2y, int *distx, int *disty,
				   int *flag, float *len, float *area,
				   int *tot_width, int *tot_height );

void skinPixels( Mat *imgBGR, Mat *imgSkin, Mat *imgHSV, vector<Mat> *hsv_planes, Mat *planeH, Mat *planeS, Mat *planeV );
void selectBiggest( int *firstidx, int *secondidx, vector<Moments> *mu );
void drawBarChart(Mat *chart, int tot_width, int tot_height, int p1x, int p1y, int p2x, int p2y, int distx, int disty );



////////////////////////////////////////////////////////////////////////////////
// MAIN ////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[])
{
	// Check input /////////////////////////////////////////////////////////////
    if (argc != 2)
    {
        cout << "Not enough parameters" << endl;
        cout << "How to use: " << argv[0] << " <path of the input video>" << endl;
        return -1;
    }

    const string sourceReference = argv[1];
    char c;
    int frameNum = -1;          // Frame counter

    // Load video //////////////////////////////////////////////////////////////
    VideoCapture captUndTst(sourceReference);
    if ( !captUndTst.isOpened())
    {
        cout  << "Could not open reference " << sourceReference << endl;
        return -1;
    }

    Size refS = Size((int) captUndTst.get(CV_CAP_PROP_FRAME_WIDTH), (int) captUndTst.get(CV_CAP_PROP_FRAME_HEIGHT));

    cout << "Reference frame resolution: Width=" << refS.width << "  Height=" << refS.height
    	 << " of nr#: " << captUndTst.get(CV_CAP_PROP_FRAME_COUNT) << endl;


    // Windows /////////////////////////////////////////////////////////////////
    // Frame under test
    namedWindow( WIN_UT, CV_WINDOW_NORMAL );
    cvMoveWindow( WIN_UT, 720, 0 );

    // Skin filter
    namedWindow( WIN_SK, CV_WINDOW_NORMAL );
    cvMoveWindow( WIN_SK, refS.width, 0 );

    // Selected contours
    namedWindow( WIN_SQ, CV_WINDOW_NORMAL );
    cvMoveWindow( WIN_SQ, 720, 300 );

    // Tracking
    namedWindow( WIN_CT, CV_WINDOW_NORMAL );
    cvMoveWindow( WIN_CT, refS.width, 300 );

    // Bar chart
    namedWindow( WIN_CHART, CV_WINDOW_NORMAL );
    cvMoveWindow( WIN_CHART, 490, 0 );
    cvResizeWindow( WIN_CHART, 220, 450 );


    ////////////////////////////////////////////////////////////////////////////
    // Allocate resources //////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////
    Mat frameUnderTest, gray_image, objects, tracking, chart;
    Mat skin, imgHSV, planeH, planeS, planeV, imgSkin;
    vector<Mat> hsv_planes;
    Mat el1, el2;
    int firstidx, secondidx;
    int p1x, p1y, p2x, p2y, distx, disty;
    int flag=0;
    float len, area;


    ////////////////////////////////////////////////////////////////////////////
    // Analyze frame ///////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////
    while(true)
    {
    	// Load Frame //////////////////////////////////////////////////////////
        captUndTst >> frameUnderTest;
        if( frameUnderTest.empty() )
        {
            cout << " < < <  End of video!  > > > " << endl;
            break;
        }
        ++frameNum;
        cout << "Frame:" << " #" << frameNum << endl;

        // Analyze Frame ///////////////////////////////////////////////////////
        analyzeFrame( &frameUnderTest, &gray_image, &objects, &tracking, &chart,
        			  &skin, &imgHSV, &planeH, &planeS, &planeV, &imgSkin,
        			  &hsv_planes, &el1, &el2,
        			  &firstidx, &secondidx,
        			  &p1x, &p1y, &p2x, &p2y, &distx, &disty,
        			  &flag, &len, &area,
        			  &refS.width, &refS.height );

      	// Wait for key ////////////////////////////////////////////////////////
      	c = cvWaitKey(1);
        if (c == 27) break;
    }
    return 0;
}



////////////////////////////////////////////////////////////////////////////////
// ANALYZE FRAME ///////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void analyzeFrame( Mat *frameUnderTest, Mat *gray_image, Mat *objects, Mat *tracking, Mat *chart,
				   Mat *skin, Mat *imgHSV, Mat *planeH, Mat *planeS, Mat *planeV, Mat *imgSkin,
				   vector<Mat> *hsv_planes, Mat *el1, Mat *el2,
				   int *firstidx, int *secondidx,
				   int *p1x, int *p1y, int *p2x, int *p2y, int *distx, int *disty,
				   int *flag, float *len, float *area,
				   int *tot_width, int *tot_height )
{
	// Show original image /////////////////////////////////////////////////////
	imshow( WIN_UT, *frameUnderTest );

	// Convert to gray /////////////////////////////////////////////////////////
	*gray_image = Mat::zeros( frameUnderTest->size(), CV_8UC3 );
	cvtColor( *frameUnderTest, *gray_image, CV_RGB2GRAY );

	// Blur ////////////////////////////////////////////////////////////////////
	blur( *gray_image, *gray_image, Size(3,3) );

	// Threshold ///////////////////////////////////////////////////////////////
	threshold( *gray_image, *gray_image, threshold_value, max_BINARY_value, THRESH_BINARY_INV );

	// Skin Filter (detection and subtraction) /////////////////////////////////
	skinPixels( frameUnderTest, imgSkin, imgHSV, hsv_planes, planeH, planeS, planeV );
	subtract( *gray_image, *imgSkin, *gray_image );

	// Show skin filter ////////////////////////////////////////////////////////
	imshow( WIN_SK, *gray_image );

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
		*area = contourArea( contours[i] );
		if( *area > thresh_area )
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
	selectBiggest( firstidx, secondidx, &mu );



	////////////////////////////////////////////////////////////////////////////
	// Draw selected contours //////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////
	*objects = Mat::zeros( gray_image->size(), CV_8UC3 );

	// First object
	drawContours( *objects, contours_poly, *firstidx, green, 2, 8, hierarchy, 0, Point() );
	rectangle( *objects, boundRect[*firstidx].tl(), boundRect[*firstidx].br(), green, 2, 8, 0 );
	circle( *objects, mc[*firstidx], 5, green, -1, 8, 0 );

	// Second object
	drawContours( *objects, contours_poly, *secondidx, green, 2, 8, hierarchy, 0, Point() );
	rectangle( *objects, boundRect[*secondidx].tl(), boundRect[*secondidx].br(), green, 2, 8, 0 );
	circle( *objects, mc[*secondidx], 5, green, -1, 8, 0 );

	// Show selected contours //////////////////////////////////////////////////
	imshow( WIN_SQ, *objects );



	////////////////////////////////////////////////////////////////////////////
	// Draw tracking ///////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////
	if( *flag == 0 )
	{
		// If it's the first iteration, clear the image
		// otherwise update the tracking
		*tracking = Mat::zeros( gray_image->size(), CV_8UC3 );
		*flag = 1;
	}

	// First center
	*p1x = mu[*firstidx].m10/mu[*firstidx].m00;
	*p1y = mu[*firstidx].m01/mu[*firstidx].m00;

	circle( *tracking, mc[*firstidx], 5, green, -1, 8, 0 );
	cout << " - Center: (" << *p1x << "," << *p1y << ")" << endl;


	// Second center
	*p2x = mu[*secondidx].m10/mu[*secondidx].m00;
	*p2y = mu[*secondidx].m01/mu[*secondidx].m00;

	circle( *tracking, mc[*secondidx], 5, green, -1, 8, 0 );
	cout << " - Center: (" << *p2x << "," << *p2y << ")" << endl;
	cout << "---" << endl;

	// Show tracking ///////////////////////////////////////////////////////////
	imshow( WIN_CT, *tracking );



	////////////////////////////////////////////////////////////////////////////
	// Draw Bar Chart //////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////
	*distx = abs(*p1x - *p2x);
	*disty = abs(*p1y - *p2y);

	drawBarChart( chart, *tot_width, *tot_height, *p1x, *p1y, *p2x, *p2y, *distx, *disty );
}



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

    // Invert the resulting image
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
// DRAW BARCHART ///////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void drawBarChart(Mat *chart, int tot_width, int tot_height, int p1x, int p1y, int p2x, int p2y, int distx, int disty )
{
	// Define size of the new image ////////////////////////////////////////////
	int new_height;
	if( tot_width > tot_height ) new_height = tot_width;
	else new_height = tot_height;

	// Reset Chart /////////////////////////////////////////////////////////////
	*chart = Mat::zeros( Size(400, new_height+50), CV_8UC3 );

	// Axis ////////////////////////////////////////////////////////////////////
	line( *chart, Point(0, new_height), Point(400, new_height), red, 5, 8, 0 );

	putText( *chart, "x1", Point(5,new_height+30), FONT_HERSHEY_TRIPLEX, 1, white, 1, 8, false );
	putText( *chart, "y1", Point(55,new_height+30), FONT_HERSHEY_TRIPLEX, 1, white, 1, 8, false );

	putText( *chart, "x2", Point(155,new_height+30), FONT_HERSHEY_TRIPLEX, 1, white, 1, 8, false );
	putText( *chart, "y2", Point(205,new_height+30), FONT_HERSHEY_TRIPLEX, 1, white, 1, 8, false );

	putText( *chart, "dx", Point(305,new_height+30), FONT_HERSHEY_TRIPLEX, 1, white, 1, 8, false );
	putText( *chart, "dy", Point(355,new_height+30), FONT_HERSHEY_TRIPLEX, 1, white, 1, 8, false );


	// Define object on left and right /////////////////////////////////////////
	int x1,x2,y1,y2;
	if( p1x < p2x)
	{
		x1 = p1x;
		y1 = p1y;

		x2 = p2x;
		y2 = p2y;
	}
	else
	{
		x1 = p2x;
		y1 = p2y;

		x2 = p1x;
		y2 = p1y;
	}

	// Draw bars of P1 (left object) ///////////////////////////////////////////
	rectangle( *chart, Point(5,new_height), Point(5+40,new_height-x1), peach, CV_FILLED, 8, 0 );
	rectangle( *chart, Point(55,new_height), Point(55+40,new_height-y1), bisque, CV_FILLED, 8, 0 );

	// Draw bars of P2 (right object) //////////////////////////////////////////
	rectangle( *chart, Point(155,new_height), Point(155+40,new_height-x2), steel, CV_FILLED, 8, 0 );
	rectangle( *chart, Point(205,new_height), Point(205+40,new_height-y2), cadet, CV_FILLED, 8, 0 );

	// Draw bars of distance ///////////////////////////////////////////////////
	rectangle( *chart, Point(305,new_height), Point(305+40,new_height-distx), orange, CV_FILLED, 8, 0 );
	rectangle( *chart, Point(355,new_height), Point(355+40,new_height-disty), darkorange, CV_FILLED, 8, 0 );

	// Show the bar chart
	imshow( WIN_CHART, *chart );
}
