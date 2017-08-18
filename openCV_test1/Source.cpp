#include <cv.h>
#include <highgui.h>
#include <stdlib.h>
#include <stdio.h>

IplImage* image = 0;
IplImage* dst = 0;

// save HSV channels
IplImage* hsv = 0;
IplImage* h_plane = 0;
IplImage* s_plane = 0;
IplImage* v_plane = 0;

IplImage* yellow_picture = 0;

// result picture
IplImage* hue_yellow = 0;

int main(int argc, char* argv[])
{
	// Load an image
	char* filename = argc >= 2 ? argv[1] : "File.jpeg";
	image = cvLoadImage(filename, 1);

	printf("[i] image: %s\n", filename);
	assert(image != 0);

	// Create images
	//hsv = cvCreateImage(cvGetSize(image), IPL_DEPTH_8U, 3);
	yellow_picture = cvCreateImage(cvGetSize(image), IPL_DEPTH_8U, 1);

	// Convert to HUE
	//cvCvtColor(image, hsv, CV_BGR2HSV);

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
	
	// Фильтрация шума, для избежания ложного обнаружения круга
	//GaussianBlur(src_gray, src_gray, Size(9, 9), 2, 2);

	// сглаживаем исходную картинку
	// cvSmooth(dst, dst, CV_GAUSSIAN, 3, 3);

	// Find contours
	CvMemStorage* storage = cvCreateMemStorage(0);
	CvSeq* contours = 0;

//	int contoursCont = cvFindContours(bin, storage, &contours, sizeof(CvContour), 0, CV_CHAIN_APPROX_SIMPLE, cvPoint(0, 0));
	int contoursCont = cvFindContours(dst, storage, &contours, sizeof(CvContour), CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE, cvPoint(0, 0));

	// Draw contours
	for (CvSeq* seq0 = contours; seq0 != 0; seq0 = seq0->h_next) {
		cvDrawContours(dst, seq0, CV_RGB(255, 216, 0), CV_RGB(0, 0, 250), 0, 1, 8); // рисуем контур
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


	// --------------------------------- Select ROI --------------------------------
	// ROI width and height
	int roi_width = round ((dst->width) / 15);
	int roi_height = round ((dst->height) / 3);
	int roi_x = center_x - round (roi_width / 2);
	int roi_y = center_y - round (roi_height / 3);
	

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

	// reset ROI
	cvResetImageROI(image);
	
	// ---------------------------- work with small image --------------------------

	// Convert to HUE
	cvCvtColor(img_roi, img_roi, CV_BGR2HSV);

	// Select yellow
	//cvInRangeS(img_roi, cvScalar(50, 210, 150), cvScalar(150, 255, 255), img_roi);

	// -------------------------------- pixels_per_metric --------------------------


	// --------------------------------- Diisplay windows --------------------------
	// Create a window to display results
	cvNamedWindow("original", CV_WINDOW_AUTOSIZE);
	cvNamedWindow("contours", 1);
	cvNamedWindow("yellow", 1);
	cvNamedWindow("roi", 1);

	// Show images
	cvShowImage("original", image);
	cvShowImage("contours", dst);
	cvShowImage("yellow", yellow_picture);
	cvShowImage("roi", img_roi);

	// Wait until user finishes program
	cvWaitKey(0);

	// reset ROI
	cvResetImageROI(image);

	// release memory
	cvReleaseImage(&image);
	cvReleaseImage(&dst);
	cvReleaseImage(&yellow_picture);
	cvReleaseImage(&img_roi);

	// Delete windows
	cvDestroyAllWindows();
	return 0;
}