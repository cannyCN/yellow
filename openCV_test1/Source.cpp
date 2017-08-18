#include <cv.h>
#include <highgui.h>
#include <stdlib.h>
#include <stdio.h>

IplImage* image = 0;
IplImage* dst = 0;
IplImage* yellow_picture = 0;


int main(int argc, char* argv[])
{
	// Load an image
	char* filename = argc >= 2 ? argv[1] : "File1.jpeg";
	image = cvLoadImage(filename, 1);

	printf("[i] image: %s\n", filename);
	assert(image != 0);

	// Create images
	yellow_picture = cvCreateImage(cvGetSize(image), IPL_DEPTH_8U, 1);

	// Select yellow
	cvInRangeS(image, cvScalar(50, 210, 150), cvScalar(150, 255, 255), yellow_picture);

	// center of mass

	int Xc = 0;
	int Yc = 0;
	int counter = 0; // white pixel counter
	int x, y;
	float center_x, center_y;

	for (y = 0; y<yellow_picture->height; y++)
	{
		uchar* ptr = (uchar*)(yellow_picture->imageData + y * yellow_picture->widthStep);
		for (x = 0; x<yellow_picture->width; x++)
		{
			if (ptr[x]>0)
			{
				Xc += x;
				Yc += y;
				counter++;
			}
		}
	}

	if (counter != 0)
	{
		center_x = float(Xc) / counter;
		center_y = float(Yc) / counter;
	}

	// output data
	printf("yellow_picture\n");
	printf("X position = ");
	printf("%f\n", center_x);
	printf("Y position = ");
	printf("%f\n", center_y);

	printf("Width = ");
	printf("%i\n", yellow_picture->width);
	printf("Height = ");
	printf("%i\n", yellow_picture->height);

	
	// Clone image
	dst = cvCloneImage(yellow_picture);
	/*
	// Find contours
	CvMemStorage* storage = cvCreateMemStorage(0);
	CvSeq* contours = 0;

	int contoursCont = cvFindContours(dst, storage, &contours, sizeof(CvContour), CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE, cvPoint(0, 0));

	// Draw contours
	for (CvSeq* seq0 = contours; seq0 != 0; seq0 = seq0->h_next) {
		cvDrawContours(dst, seq0, CV_RGB(255, 216, 0), CV_RGB(0, 0, 250), 0, 1, 8); 
	}

	// ----------------- Center of counturs -------------------

	Xc = 0;
	Yc = 0;
	counter = 0; // white pixel counter
	x = 0;
	y = 0;

	for (y = 0; y<dst->height; y++)
	{
		uchar* ptr = (uchar*)(dst->imageData + y * dst->widthStep);
		for (x = 0; x<dst->width; x++)
		{
			if (ptr[x]>0)
			{
				Xc += x;
				Yc += y;
				counter++;
			}
		}
	}

	if (counter != 0)
	{
		center_x = float(Xc) / counter;
		center_y = float(Yc) / counter;
	}

	// output data
	printf("dst\n");
	printf("X position = ");
	printf("%f\n", center_x);
	printf("Y position = ");
	printf("%f\n", center_y);

	printf("Width = ");
	printf("%i\n", dst->width);
	printf("Height = ");
	printf("%i\n", dst->height);
	*/

	// --------------------------------- Select ROI --------------------------------
	// ROI width and height
	int roi_width = round ((dst->width) / 15);
	int roi_height = round ((dst->height) / 4);
	int roi_x = center_x - round (roi_width / 2);
	int roi_y = center_y - round (roi_height / 2);
	

	// set ROI and select ruler
	cvSetImageROI(image, cvRect(roi_x, roi_y, roi_width, roi_height));
	
	// create new image for roi
	IplImage *img_roi = cvCreateImage(cvGetSize(image),
		image->depth,
		image->nChannels);

	cvCopy(image, img_roi, NULL);

	// value for selecting area
	int add = 200;
	// add constant to select ROI
	cvAddS(image, cvScalar(add), image);

	// output data
	printf("\n");
	printf("roi\n");

	printf("roi_width = ");
	printf("%u\n", roi_width);
	printf("roi_height = ");
	printf("%u\n", roi_height);

	printf("Width = ");
	printf("%i\n", img_roi->width);
	printf("Height = ");
	printf("%i\n", img_roi->height);

	// reset ROI
	cvResetImageROI(image);

	// ---------------------------- work with small image --------------------------
	
	// result picture
	IplImage* roi_yellow = 0;
	roi_yellow = cvCreateImage(cvGetSize(img_roi), IPL_DEPTH_8U, 1);

	// Select yellow
	cvInRangeS(img_roi, cvScalar(0, 150, 180), cvScalar(255, 255, 255), roi_yellow);

	// counturs
	CvMemStorage* storage1 = cvCreateMemStorage(0);
	CvSeq* contours1 = 0;

	int contoursCont1 = cvFindContours(roi_yellow, storage1, &contours1, sizeof(CvContour), CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cvPoint(0, 0));

	// Draw contours with fill in the contour inside
	// remove numbers inside
	for (CvSeq* seq0 = contours1; seq0 != 0; seq0 = seq0->h_next) {
		cvDrawContours(roi_yellow, seq0, CV_RGB(0, 0, 255), CV_RGB(0, 0, 255), 0, -1, 0);
	}

// ---------------------------------------------
	//canny on color picture
	IplImage* roi_canny = 0;
	roi_canny = cvCreateImage(cvGetSize(img_roi), IPL_DEPTH_8U, 1);
	cvCanny(img_roi, roi_canny, 10, 100, 3);

	// delete black lines on ruller
	cvDilate(roi_yellow, roi_yellow, 0, 1);

	//canny on bit picture
	IplImage* canny_bit = 0;
	canny_bit = cvCreateImage(cvGetSize(img_roi), IPL_DEPTH_8U, 1);
	cvCanny(roi_yellow, canny_bit, 10, 100, 3);

	// find longest contour
	CvMemStorage* storage = cvCreateMemStorage(0);
	CvSeq* contours = 0;

	IplImage* longest = 0;
	longest = cvCreateImage(cvGetSize(img_roi), IPL_DEPTH_8U, 1);
	longest = cvCloneImage(roi_yellow);
	cvFindContours(longest, storage, &contours, sizeof(CvContour), CV_RETR_LIST, CV_CHAIN_APPROX_NONE, cvPoint(0, 0));

	CvSeq* seqT = 0;
	double perimT = 0;

	if (contours != 0) {
		// find longest contour
		for (CvSeq* seq0 = contours; seq0 != 0; seq0 = seq0->h_next) {
			double perim = cvContourPerimeter(seq0);
			if (perim>perimT) {
				perimT = perim;
				seqT = seq0;
			}
		}
	}
	// Draw contour 
	cvDrawContours(longest, seqT, CV_RGB(255, 255, 255), CV_RGB(255, 255, 255), 0, 5, 0); 

	// -------------------------------- contour moments test -----------------------

	// -------------------------------- pixels_per_metric --------------------------

	// --------------------------------- Diisplay windows --------------------------
	// Create a window to display results
	//cvNamedWindow("original", CV_WINDOW_AUTOSIZE);
	cvNamedWindow("img_roi", CV_WINDOW_AUTOSIZE);
	cvNamedWindow("roi_yellow", CV_WINDOW_AUTOSIZE);
	cvNamedWindow("canny", CV_WINDOW_AUTOSIZE);
	cvNamedWindow("canny_bit", CV_WINDOW_AUTOSIZE);
	cvNamedWindow("longest", CV_WINDOW_AUTOSIZE);

	// Show images
	//cvShowImage("original", image);
	cvShowImage("img_roi", img_roi);
	cvShowImage("roi_yellow", roi_yellow);
	cvShowImage("canny", roi_canny);
	cvShowImage("canny_bit", canny_bit);
	cvShowImage("longest", longest);

	// Wait until user finishes program
	cvWaitKey(0);


	// release memory
	cvReleaseImage(&image);
	cvReleaseImage(&dst);
	cvReleaseImage(&yellow_picture);
	cvReleaseImage(&roi_yellow);
	cvReleaseImage(&img_roi);
	cvReleaseImage(&roi_canny);
	cvReleaseImage(&canny_bit);
	cvReleaseImage(&longest);

	// Delete windows
	cvDestroyAllWindows();
	return 0;
}