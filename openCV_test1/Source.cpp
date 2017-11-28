#include <cv.h>
#include <highgui.h>
#include <stdlib.h>
#include <stdio.h>

// define matrix for picture
IplImage* image = 0;
IplImage* yellow_picture = 0;
IplImage* gray_picture = 0;

// shirt tamplate
IplImage* shirt = 0;

// variables
double unit = 0.;

// Object size in mm 90*90
double square_width = 90.;
double square_high = 90.;

// define colors
// [R, G, B]
const CvScalar WHITE = CV_RGB(255, 255, 255);
const CvScalar BLACK = CV_RGB(0, 0, 0);
const CvScalar YELLOW1 = CV_RGB(80, 50, 50);
const CvScalar YELLOW2 = CV_RGB(100, 255, 255);
const CvScalar RED = CV_RGB(255, 0, 0);

int main(int argc, char* argv[])
{

  // Load an image
  char* filename = argc >= 2 ? argv[1] : "test.jpg";
  image = cvLoadImage(filename, 1);
  printf("[i] image: %s\n", filename);
  assert(image != 0);

  // Load an shirt template
  filename = argc >= 2 ? argv[1] : "contour.jpg";
  shirt = cvLoadImage(filename, 1);
  printf("[i] template: %s\n", filename);
  assert(shirt != 0);


  // Create images 
  // 3 chanells for HSV
  yellow_picture = cvCreateImage(cvGetSize(image), IPL_DEPTH_8U, 3);

  // 1 channel for GrayScale 
  gray_picture = cvCreateImage(cvGetSize(image), IPL_DEPTH_8U, 1);


  /// ---------------------------------- Select yellow ---------------------------
  // confert to HSB and select yellow color
  cvCvtColor(image,
    yellow_picture,
    CV_RGB2HSV);

  // select color
  // H 30, S 38, V 71
  // Range +-H, 50-255 S, 50-255 V
  cvInRangeS(yellow_picture,
    YELLOW1,
    YELLOW2,
    gray_picture);
  /// ---------------------------------- End Select yellow -----------------------

  /// ---------------------------------- Canny -----------------------------------
  // Smoothing with Gaussian filter
  cvSmooth(gray_picture,
    gray_picture,
    2, 3,
    0, 0, 0);

  //canny on gray picture
  cvCanny(gray_picture,
    gray_picture,
    10,
    100,
    3);

  cvDilate(gray_picture,
    gray_picture,
    0, 1);

  cvErode(gray_picture,
    gray_picture,
    0, 1);
  /// ---------------------------------- End Canny -------------------------------

  /// --------------------------- yellow Rectangle -------------------------------
  // Find longest contour 
  CvMemStorage* storageContour = cvCreateMemStorage(0);
  CvSeq* contours = 0;

  cvFindContours(gray_picture,
    storageContour,
    &contours,
    sizeof(CvContour),
    CV_RETR_LIST,
    CV_CHAIN_APPROX_NONE,
    cvPoint(0, 0));

  CvSeq* seqT = 0;
  double perimT = 0;

  if (contours != 0) {
    // find longest contour
    for (CvSeq* seq0 = contours; seq0 != 0; seq0 = seq0->h_next) {
      double perim = cvContourPerimeter(seq0);
      //      printf("%f\n", perim);
      if (perim>perimT) {
        perimT = perim;
        seqT = seq0;
      }
    }
  }
  // Draw contour 
  cvDrawContours(gray_picture,
    seqT,
    WHITE,
    WHITE,
    0, 1, 0);

  // Draw rectagle
  CvBox2D rect;
  CvPoint2D32f rect_vtx[4];
  CvPoint pt, pt0;

  rect = cvMinAreaRect2(seqT, 0);
  cvBoxPoints(rect, rect_vtx);

  pt0.x = cvRound(rect_vtx[3].x);
  pt0.y = cvRound(rect_vtx[3].y);
  int i = 0;
  for (i = 0; i < 4; i++)
  {

    printf("rect.x = ");
    printf("%d", pt0.x);

    printf("    rect.y = ");
    printf("%d\n", pt0.y);

    pt.x = cvRound(rect_vtx[i].x);
    pt.y = cvRound(rect_vtx[i].y);
    cvLine(image,
      pt0,
      pt,
      WHITE,
      1, CV_AA, 0);
    pt0 = pt;
  }

  /// --------------------------- End yellow rectangle ---------------------------

  /// ------------------------------- pixels_per_metric --------------------------
  // Calculate points of rectagle
  double length = 0.;
  double length_tmp = 0.;

  for (i = 0; i < 4; i++)
  {
    if (i < 3)
    {
      length_tmp = sqrt(pow((rect_vtx[i + 1].x - rect_vtx[i].x), 2) +
        pow((rect_vtx[i + 1].y - rect_vtx[i].y), 2));

      printf("length = ");
      printf("%f\n", length_tmp);

      length = length + length_tmp;
    }
    else
    {
      length_tmp = sqrt(pow((rect_vtx[0].x - rect_vtx[3].x), 2) +
        pow((rect_vtx[0].y - rect_vtx[3].y), 2));

      printf("length = ");
      printf("%f\n", length_tmp);

      length = length + length_tmp;
    }
  }

  length = length / 4.;

  printf("length_ = ");
  printf("%f\n", length);

  unit = length / square_high;

  printf("units = ");
  printf("%f\n", unit);
  /// --------------------------- end pixels_per_metric --------------------------

  /// --------------------------- change picture contrast ------------------------
  // increase contrast of gray picture
  // define matrix for picture
  IplImage* contrast_picture = 0;
  contrast_picture = cvCreateImage(cvGetSize(image), IPL_DEPTH_8U, 3);
  cvCopy(image, contrast_picture, NULL);

  double alpha = 1.2;
  double beta = 15;

  // work with each point of color picture

  /*
  // use simple linear transfer function for lightness
  // new_lightness = alpha*lightness + beta
  for (int y = 0; y<contrast_picture->height; y++)
  {
  uchar* ptr = (uchar*)(contrast_picture->imageData + y * contrast_picture->widthStep);
  for (int x = 0; x<contrast_picture->width; x++)
  {
  ptr[3 * x + 0] = cv::saturate_cast<uchar>(alpha*(ptr[3 * x + 0]) + beta); // B
  ptr[3 * x + 1] = cv::saturate_cast<uchar>(alpha*(ptr[3 * x + 1]) + beta); // G
  ptr[3 * x + 2] = cv::saturate_cast<uchar>(alpha*(ptr[3 * x + 2]) + beta); // R
  }
  }
  */

  /*
  // correct shadows
  // circular gradient

  // center of circule
  double x0 = (contrast_picture->width) * 0/6;
  double y0 = (contrast_picture->height)* 5/6;
  double param = sqrt(pow(contrast_picture->width, 2) + pow(contrast_picture->height, 2))/2;
  double intensity = 40.;

  for (int y = 0; y<contrast_picture->height; y++)
  {
  uchar* ptr = (uchar*)(contrast_picture->imageData + y * contrast_picture->widthStep);
  for (int x = 0; x<contrast_picture->width; x++)
  {
  // B
  ptr[3 * x + 0] = cv::saturate_cast<uchar>(ptr[3 * x + 0] + intensity *
  (param - sqrt(pow((x - x0), 2) + pow((y - y0), 2)))/param);
  // G
  ptr[3 * x + 1] = cv::saturate_cast<uchar>(ptr[3 * x + 1] + intensity *
  (param - sqrt(pow((x - x0), 2) + pow((y - y0), 2))) / param);
  // R
  ptr[3 * x + 2] = cv::saturate_cast<uchar>(ptr[3 * x + 2] + intensity *
  (param - sqrt(pow((x - x0), 2) + pow((y - y0), 2))) / param);
  }
  }

  // use sin-shape transfer function for lightness
  for (int y = 0; y<contrast_picture->height; y++)
  {
  uchar* ptr = (uchar*)(contrast_picture->imageData + y * contrast_picture->widthStep);
  for (int x = 0; x<contrast_picture->width; x++)
  {
  // B
  if (ptr[3 * x + 0] > 128)
  {
  ptr[3 * x + 0] = cv::saturate_cast<uchar>(ptr[3 * x + 0] - 64 * sin(ptr[3 * x + 0] * 2 * 3.14 / 255.));
  }
  else
  {
  ptr[3 * x + 0] = cv::saturate_cast<uchar>(ptr[3 * x + 0] - 64 * sin(ptr[3 * x + 0] * 2 * 3.14 / 255.));
  }
  // G
  if (ptr[3 * x + 1] > 128)
  {
  ptr[3 * x + 1] = cv::saturate_cast<uchar>(ptr[3 * x + 1] - 64 * sin(ptr[3 * x + 1] * 2 * 3.14 / 255.));
  }
  else
  {
  ptr[3 * x + 1] = cv::saturate_cast<uchar>(ptr[3 * x + 1] - 64 * sin(ptr[3 * x + 1] * 2 * 3.14 / 255.));
  }
  // R
  if (ptr[3 * x + 2] > 128)
  {
  ptr[3 * x + 2] = cv::saturate_cast<uchar>(ptr[3 * x + 2] - 64 * sin(ptr[3 * x + 2] * 2 * 3.14 / 255.));
  }
  else
  {
  ptr[3 * x + 2] = cv::saturate_cast<uchar>(ptr[3 * x + 2] - 64 * sin(ptr[3 * x + 2] * 2 * 3.14 / 255.));
  }
  }
  }
  */
  /// ----------------------- end change picture contrast ------------------------

  /// ---------------------------------- Canny -----------------------------------
  // threshold for transfer color picture into gray
  cvCvtColor(contrast_picture, gray_picture, CV_RGB2GRAY);
  cvThreshold(gray_picture,
    gray_picture,
    200,
    255,
    3);

  cvDilate(gray_picture,
    gray_picture,
    0, 2);

  cvErode(gray_picture,
    gray_picture,
    0, 2);

  // Smoothing with Gaussian filter
  cvSmooth(gray_picture,
    gray_picture,
    2, 3,
    0, 0, 0);

  //canny on gray picture
  cvCanny(gray_picture,
    gray_picture,
    10,
    100,
    3);

  cvDilate(gray_picture,
    gray_picture,
    0, 2);

  cvErode(gray_picture,
    gray_picture,
    0, 2);

  cvDilate(gray_picture,
    gray_picture,
    0, 2);

  cvErode(gray_picture,
    gray_picture,
    0, 2);
  /// ---------------------------------- End Canny -------------------------------

  /// ------------------------- finding shirt contour ----------------------------
  // Find longest contour 
  CvMemStorage* storageContourT = cvCreateMemStorage(0);
  contours = 0;

  cvFindContours(gray_picture,
    storageContourT,
    &contours,
    sizeof(CvContour),
    CV_RETR_LIST,
    CV_CHAIN_APPROX_NONE,
    cvPoint(0, 0));

  seqT = 0;
  perimT = 0;

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
  cvDrawContours(gray_picture,
    seqT,
    WHITE,
    WHITE,
    0, 1, 8, 0);



  /// --------------------- end finding shirt contour ----------------------------

  /// --------------------- approximate shirt ------------------------------------
  CvSeq* result = 0;
  result = cvApproxPoly(seqT,
    sizeof(CvContour),
    storageContourT,
    CV_POLY_APPROX_DP,
    20, 0);

  printf("[d] %d %d\n", seqT->total, result->total);

  // change line thickness to -1 for contour filling 
  // Draw contour 
  cvDrawContours(image,
    result,
    RED,
    RED,
    0, 10, 8, 0);

  int maxCorners = 10;
  double qualityLevel = 0.01;
  double minDistance = 20.;
  int blockSize = 3;
  double k = 0.04;

  printf("[d] %d %d\n", seqT->total, result->total);

  // convexHull
  CvSeq* hull2 = 0;
  hull2 = cvConvexHull2(result, 0, CV_CLOCKWISE, 1);

  printf("[d] %d %d\n", result->total, hull2->total);

  // Draw contour 
  cvDrawContours(image,
    hull2,
    BLACK,
    BLACK,
    1, 5, 8, 0);

  CvPoint pt10;

  CvMemStorage* storage1 = cvCreateMemStorage(0);
  CvMemStorage* storage2 = cvCreateMemStorage(0);
  CvSeq* ptseq = cvCreateSeq(CV_SEQ_KIND_GENERIC | CV_32SC2, sizeof(CvContour),
    sizeof(CvPoint), storage1);
  CvSeq* hull;
  CvSeq* defects;

  for (int i = 0; i < result->total; i++)
  {
    CvPoint* p = CV_GET_SEQ_ELEM(CvPoint, result, i);
    pt10.x = p->x;
    pt10.y = p->y;
    cvSeqPush(ptseq, &pt10);
  }
  hull = cvConvexHull2(ptseq, 0, CV_CLOCKWISE, 0);
  int hullcount = hull->total;

  defects = cvConvexityDefects(ptseq, hull, storage2);

  printf(" defect no %d \n", defects->total);

  CvConvexityDefect* defectArray;

  int j = 0;
  //int m_nomdef=0;
  // This cycle marks all defects of convexity of current contours.  
  for (; defects; defects = defects->h_next)
  {
    int nomdef = defects->total; // defect amount  
                                 //outlet_float( m_nomdef, nomdef );  

                                 //printf(" defect no %d \n",nomdef);

    if (nomdef == 0)
      continue;

    // Alloc memory for defect set.     
    //fprintf(stderr,"malloc\n");  
    defectArray = (CvConvexityDefect*)malloc(sizeof(CvConvexityDefect)*nomdef);

    // Get defect set.  
    //fprintf(stderr,"cvCvtSeqToArray\n");  
    cvCvtSeqToArray(defects, defectArray, CV_WHOLE_SEQ);

    // Draw marks for all defects.  
    for (int i = 0; i<nomdef; i++)
    {
      printf(" defect depth for defect %d %f \n", i, defectArray[i].depth);
      cvLine(image, *(defectArray[i].start), *(defectArray[i].depth_point), BLACK, 3, CV_AA, 0);
      cvCircle(image, *(defectArray[i].depth_point), 10, BLACK, 5, 8, 0);
      cvCircle(image, *(defectArray[i].start), 10, BLACK, 5, 8, 0);
      cvLine(image, *(defectArray[i].depth_point), *(defectArray[i].end), BLACK, 3, CV_AA, 0);

    }

    /// ----------------- end approximate shirt ------------------------------------

    /// --------------------- find shirt high --------------------------------------
    // нужно сделать поворот рубашки для получения вертикали

    //CvBox2D shirtRect;
    CvRect shirtRect;
    CvPoint2D32f shirtRect_vtx[4];
    CvPoint point, point0;

    shirtRect = cvBoundingRect(seqT, 0);

    cvRectangle(image,
      cvPoint(shirtRect.x, shirtRect.y),
      cvPoint(shirtRect.x + shirtRect.width,
        shirtRect.y + shirtRect.height),
      BLACK,
      10, 8, 0);

    cvLine(image,
      cvPoint(shirtRect.x + (shirtRect.width / 2), shirtRect.y),
      cvPoint(shirtRect.x + (shirtRect.width / 2), shirtRect.y + shirtRect.height),
      RED,
      20, CV_AA, 0);


    /* cvBoxPoints(shirtRect, shirtRect_vtx);

    point0.x = cvRound(shirtRect_vtx[3].x);
    point0.y = cvRound(shirtRect_vtx[3].y);
    int i = 0;
    for (i = 0; i < 4; i++)
    {

    printf("rect.x = ");
    printf("%d", pt0.x);

    printf("    rect.y = ");
    printf("%d\n", pt0.y);

    point.x = cvRound(shirtRect_vtx[i].x);
    point.y = cvRound(shirtRect_vtx[i].y);
    cvLine(image,
    point0,
    point,
    BLACK,
    10, CV_AA, 0);
    pt0 = pt;
    }*/


    /// ------------------ end find shirt high -------------------------------------

    /// --------------------- find armpit point ------------------------------------

    int armpitindex = 0;
    CvPoint armpitLeft = (0, 0);
    CvPoint armpitRight = (0, 0);
    float maxDefect = defectArray[0].depth;
    bool left = false;

    for (int i = 0; i<nomdef; i++)
    {
      if (defectArray[i].depth > maxDefect)
      {
        maxDefect = defectArray[i].depth;
        armpitindex = i;
      }
    }

    armpitLeft.x = defectArray[armpitindex].depth_point->x;
    armpitLeft.y = defectArray[armpitindex].depth_point->y;

    if (2 * armpitLeft.x < image->width)
    {
      left = true;
      cvCircle(image, armpitLeft, 50, BLACK, 5, 8, 0);
    }
    else
    {
      armpitRight = armpitLeft;
      cvCircle(image, armpitRight, 50, RED, 5, 8, 0);
    }

    defectArray[armpitindex].depth = (0, 0);

    maxDefect = defectArray[0].depth;

    for (int i = 0; i<nomdef; i++)
    {
      if (defectArray[i].depth > maxDefect)
      {
        maxDefect = defectArray[i].depth;
        armpitindex = i;
      }
    }


    if (left = false)
    {
      armpitLeft.x = defectArray[armpitindex].depth_point->x;
      armpitLeft.y = defectArray[armpitindex].depth_point->y;
      cvCircle(image, armpitLeft, 100, BLACK, 5, 8, 0);
    }
    else
    {
      armpitRight.x = defectArray[armpitindex].depth_point->x;
      armpitRight.y = defectArray[armpitindex].depth_point->y;
      cvCircle(image, armpitRight, 100, RED, 5, 8, 0);
    }

    double amrpitLegth = 0;
    amrpitLegth = sqrt(pow((armpitRight.x - armpitLeft.x), 2) +
      pow((armpitRight.y - armpitLeft.y), 2))*unit;

    printf("amrpitLegth = ");
    printf("%f\n", amrpitLegth);

    cvLine(image, armpitLeft, armpitRight, RED, 20, CV_AA, 0);

    /// ---------------- end find armpit point ------------------------------------

    /// -------------------- find cuffs -------------------------------------------
    int cuffIndex = 0;
    int cuffIndex2 = 0;
    CvPoint cuffTopLeft = (0, 0);
    CvPoint cuffDownLeft = (0, 0);
    CvPoint cuffTopRight = (0, 0);
    CvPoint cuffDownRight = (0, 0);
    CvPoint* cuffTmp = (0, 0);

    // Left cuff
    // Top point
    cuffTopLeft.x = image->width;
    for (int i = 0; i<hull2->total; i++)
    {
      cuffTmp = (CvPoint*)cvGetSeqElem(hull2, i);
      if (cuffTmp->x < cuffTopLeft.x)
      {
        cuffTopLeft.x = cuffTmp->x;
        cuffTopLeft.y = cuffTmp->y;
        cuffIndex = i;
      }
    }
    cvCircle(image, cuffTopLeft, 50, BLACK, 5, 8, 0);

    // Down point
    cuffIndex2 = cuffIndex;
    cuffDownLeft.x = image->width;
    for (int i = 0; i<hull2->total; i++)
    {
      cuffTmp = (CvPoint*)cvGetSeqElem(hull2, i);
      if (cuffTmp->x < cuffDownLeft.x)
      {
        if (((cuffTmp->x - cuffTopLeft.x) < (image->width) / 5) && (cuffIndex2 != i))
        {
          cuffDownLeft.x = cuffTmp->x;
          cuffDownLeft.y = cuffTmp->y;
          cuffIndex = i;
        }
      }
    }
    cvCircle(image, cuffDownLeft, 100, BLACK, 5, 8, 0);

    // Left cuff length
    double cuffLeftLegth = 0;
    cuffLeftLegth = sqrt(pow((cuffTopLeft.x - cuffDownLeft.x), 2) +
      pow((cuffTopLeft.y - cuffDownLeft.y), 2))*unit;

    printf("cuffLeftLegth = ");
    printf("%f\n", cuffLeftLegth);

    cvLine(image, cuffTopLeft, cuffDownLeft, RED, 20, CV_AA, 0);


    // Right cuff
    // Top point
    cuffTopRight.x = 0;
    for (int i = 0; i<hull2->total; i++)
    {
      cuffTmp = (CvPoint*)cvGetSeqElem(hull2, i);
      if (cuffTmp->x > cuffTopRight.x)
      {
        cuffTopRight.x = cuffTmp->x;
        cuffTopRight.y = cuffTmp->y;
        cuffIndex = i;
      }
    }
    cvCircle(image, cuffTopRight, 50, RED, 5, 8, 0);

    // Down point
    cuffIndex2 = cuffIndex;
    cuffDownRight.x = 0;
    for (int i = 0; i<hull2->total; i++)
    {
      cuffTmp = (CvPoint*)cvGetSeqElem(hull2, i);
      if (cuffTmp->x > cuffDownRight.x)
      {
        if (((cuffTopRight.x - cuffTmp->x) < (image->width) / 5) && (cuffIndex2 != i))
        {
          cuffDownRight.x = cuffTmp->x;
          cuffDownRight.y = cuffTmp->y;
          cuffIndex = i;
        }
      }
    }
    cvCircle(image, cuffDownRight, 100, RED, 5, 8, 0);

    // Right cuff length
    double cuffRightLegth = 0;
    cuffRightLegth = sqrt(pow((cuffTopRight.x - cuffDownRight.x), 2) +
      pow((cuffTopRight.y - cuffDownRight.y), 2))*unit;

    printf("cuffRightLegth = ");
    printf("%f\n", cuffRightLegth);

    cvLine(image, cuffTopRight, cuffDownRight, RED, 20, CV_AA, 0);

    /// ---------------- end find cuffs -------------------------------------------

    /*char txt[] = "0";
    txt[0] = '0' + nomdef - 1;
    CvFont font;
    cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 1.0, 1.0, 0, 5, CV_AA);
    cvPutText(image, txt, cvPoint(50, 50), &font, cvScalar(0, 0, 255, 0));

    //j++;
    */
    // Free memory.         
    free(defectArray);
  } /// шо це за херня?


  cvReleaseMemStorage(&storage1);
  cvReleaseMemStorage(&storage2);


  /// -------------------------------- Display windows ---------------------------
  // resize picture

  cvSaveImage("src.jpg", image, 0);

  // Create a window to display results
  cvNamedWindow("original", CV_WINDOW_NORMAL);
  // cvNamedWindow("yellow", CV_WINDOW_NORMAL);
  cvNamedWindow("gray", CV_WINDOW_NORMAL);

  // cvNamedWindow("contrast", CV_WINDOW_NORMAL);

  cvNamedWindow("template", CV_WINDOW_NORMAL);

  // Show images
  cvShowImage("original", image);
  //cvShowImage("yellow", yellow_picture);
  cvShowImage("gray", gray_picture);

  //("contrast", contrast_picture);

  cvShowImage("template", shirt);

  // --------------------------------- Clear memory ------------------------------
  // От кого я особенно ненавижу - так ето вас всех!
  // Kill them all!

  // Wait until user finishes program
  cvWaitKey(0);


  // release memory
  cvReleaseImage(&image);
  cvReleaseImage(&yellow_picture);
  cvReleaseImage(&gray_picture);

  cvReleaseImage(&contrast_picture);

  cvReleaseImage(&shirt);

  // Delete windows
  cvDestroyAllWindows();
  return 0;
}
