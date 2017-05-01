// This code is inspired from the pseudocode of Otsu algorithm given by
// A C++ Implementation of Otsu’s Image Segmentation Method Juan Pablo Balarini, Sergio Nesmachnow

// The code for canny edge detection is taken from our implementation in Assignment 1.

#include <limits>
#include <string>
#include <stdio.h>
#include <vector>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <ctime>
#include <iostream>
#include <iomanip>
#include <stdlib.h>
#include <string>
#include <vector>
#include <sys/types.h>
#include <map>
#include <numeric>
#include <fstream>
#include <sstream>
#include <iterator>

using namespace cv;
using namespace std;


bool foundEdge(Mat output_after_nms, int window, int row, int col)
{
for (int r = row-window; r<=row+window; r++)
{
  for(int c = col-window; col<=col+window; c++)
  {
    if (output_after_nms.at<double>(r, c) >= 100.0)
    return true;
  }
}
return false;
}

// Hysterisis thresholding

void hysterisisThresholding(Mat output_after_nms, Mat output_hysterisis)
{
    int height = output_after_nms.rows;
    int width = output_after_nms.cols;
    double low = 140.0, high = 240.0;
    for (int r = 2; r< height - 2; r++)
    {
        for (int c = 2; c<width-2; c++)
        {
        if (output_after_nms.at<double>(r, c) >= high )
        output_hysterisis.at<double>(r, c) = 255.0;
        else if (output_after_nms.at<double>(r, c) > low && output_after_nms.at<double>(r, c) <high)
         {
        bool found = false;
        if (foundEdge(output_after_nms, 1, r, c))
        {
        found = true;
        output_hysterisis.at<double>(r, c) = 255.0;
        }
        else  if (!found && foundEdge(output_after_nms, 2, r, c))
        {
        output_hysterisis.at<double>(r, c) = 255.0;

        }
      }
    }
  }
}

// Function for non max suppression
void nonMaxSuppression(Mat output_after_nms, Mat output, Mat orientation)
{
    int height = output.rows;
    int width = output.cols;
    for (int r = 1; r< height - 1; r++)
    {
        for (int c = 1; c<width-1; c++)
        {
        double orient = orientation.at<double>(r, c);
        double magnitude = output.at<double>(r, c);
        if ((orient == 0) && magnitude >= max(output.at<double>(r, c-1), output.at<double>(r, c+1)))
        output_after_nms.at<double>(r, c) = magnitude;
        else if ((orient == 45) && magnitude >= max( output.at<double>(r+1, c-1), output.at<double>(r-1, c+1)))
        output_after_nms.at<double>(r, c) = magnitude;
        else if ((orient == 90) && magnitude >= max (output.at<double>(r-1, c), output.at<double>(r+1, c) ))
        output_after_nms.at<double>(r, c) = magnitude;
        else if ((orient == 135) && magnitude >= max (output.at<double>(r+1, c+1), output.at<double>(r-1, c-1) ))
        output_after_nms.at<double>(r, c) = magnitude;
  }
 }
}

// Function for sobel filter.
Mat sobel_gradient_filter(Mat input, bool _gx)
{
  int rows = input.rows;
  int cols = input.cols;
  Mat output = Mat(rows, cols, CV_64F);
  Mat orientation = Mat(rows, cols, CV_64F);
  Mat output_after_nms = Mat(rows, cols, CV_64F);
  Mat output_hysterisis = Mat(rows, cols, CV_64F);
  // Implement a sobel gradient estimation filter with 1-d filters
  Mat sobel_Gx = Mat(3,3, CV_64F);
  Mat sobel_Gy = Mat(3,3, CV_64F);
  sobel_Gx.at<double>(0, 0) = -1;
  sobel_Gx.at<double>(0, 1) = 0;
  sobel_Gx.at<double>(0, 2) = 1;
  sobel_Gx.at<double>(1, 0) = -2;
  sobel_Gx.at<double>(1, 1) = 0;
  sobel_Gx.at<double>(1, 2) = 2;
  sobel_Gx.at<double>(2, 0) = -1;
  sobel_Gx.at<double>(2, 1) = 0;
  sobel_Gx.at<double>(2, 2) = 1;
  sobel_Gy.at<double>(0, 0) = -1;
  sobel_Gy.at<double>(0, 1) = -2;
  sobel_Gy.at<double>(0, 2) = -1;
  sobel_Gy.at<double>(1, 0) = 0;
  sobel_Gy.at<double>(1, 1) = 0;
  sobel_Gy.at<double>(1, 2) = 0;
  sobel_Gy.at<double>(2, 0) = 1;
  sobel_Gy.at<double>(2, 1) = 2;
  sobel_Gy.at<double>(2, 2) = 1;

  Mat output_Gx = Mat(rows, cols, CV_64F);
  Mat output_Gy = Mat(rows, cols, CV_64F);

  for(int i=0; i<rows; i++){
    for(int j=0; j<cols; j++){
      output.at<double>(i, j) = 0;
      orientation.at<double>(i, j) = 0;
      output_Gx.at<double>(i, j) = 0;
      output_Gy.at<double>(i, j) = 0;
    }
  }
  Mat magnitude, angle;
  Sobel(input, output_Gx,  CV_64F, 1, 0, 3);
  Sobel(input, output_Gy,  CV_64F, 0, 1, 3);
  //apply sobel
  double pi = 180.0;
  for(int r = 0; r < rows; r++)
  {
   for(int c = 0; c < cols; c++)
   {
      output.at<double>(r, c) = sqrt (pow(output_Gx.at<double>(r, c) , 2) + pow(output_Gy.at<double>(r, c) , 2));
      double tangX = output_Gx.at<double>(r, c);
      double tangY = output_Gy.at<double>(r, c);
      double temp = 0.0;
      temp = tangY/tangX;
      if (temp < 0)
        temp = ((atan(temp)*180)/3.14 ) + 180;
      else
        temp = ((atan(temp)*180)/3.14 );
      if (temp >22.5 && temp <=67.5)
        temp = 45;
      else if (temp > 67.5 && temp <=112.5)
        temp = 90;
       else if (temp > 112.5 && temp <=157.5)
        temp = 135;
      else if ((temp >157.5 && temp <=202.5) || (temp > 0 && temp <=22.5) || (temp > 337.5 && temp <=360) )
        temp = 0;
      else if (temp > 202.5 && temp <=247.5)
        temp = 45;
      else if (temp >247.5 && temp <=292.5)
        temp = 90;
      else
        temp = 135;
        orientation.at<double>(r, c) = temp;
   }
  }

    nonMaxSuppression(output_after_nms, output, orientation);
    hysterisisThresholding(output_after_nms, output_hysterisis);
    return output_hysterisis;
}

// Function for canny edge detection.
void cannyDetection(Mat originalImage, string fileName){
    Mat grayImage;
    cvtColor(originalImage, grayImage, CV_BGR2GRAY);
    //imwrite("cannygrayimage.jpg", grayImage);
    Mat gaussian_filter = Mat(3,3, CV_64F);
    gaussian_filter.at<double>(0, 0) = 0.0625;
    gaussian_filter.at<double>(0, 1) = 0.125;
    gaussian_filter.at<double>(0, 2) = 0.0625;
    gaussian_filter.at<double>(1, 0) = 0.125;
    gaussian_filter.at<double>(1, 1) = 0.25;
    gaussian_filter.at<double>(1, 2) = 0.125;
    gaussian_filter.at<double>(2, 0) = 0.0625;
    gaussian_filter.at<double>(2, 1) = 0.125;
    gaussian_filter.at<double>(2, 2) = 0.0625;

    Mat output_image_demo = Mat(originalImage.rows, originalImage.cols, CV_64F);
    Mat outputGxSobel = Mat(originalImage.rows, originalImage.cols, CV_64F);
    outputGxSobel = sobel_gradient_filter(grayImage, true);
    imwrite(fileName+"_canny.jpg", outputGxSobel);
}

// Main function
int main(int argc, char** argv ){
    string method = argv[1];
    string fileName = argv[2];

    if (method == "otsu"){
    // Read an image and conver to grayscale
    Mat grayImg = imread(fileName, CV_LOAD_IMAGE_GRAYSCALE);	
    int height = grayImg.rows;
    int width = grayImg.cols;
    Mat outputImg = Mat::zeros(height, width, CV_32F);
    int histogram[256];
    float threshold = 0;

    // Initialise the histogram
    for (int i = 0; i <= 255; i++)
    	histogram[i] = 0;

     for (int i = 0; i < width-1; i++)
    {
    	for (int j = 0; j < height-1; j++){
    		histogram[(int)grayImg.at<uchar>(j, i) ] += 1; 		
    	}
    }

    // Use Otsu to calculate threshold
    int sum = 0, sum1 = 0;
    float mean1, mean2, q1, q2, variance, max_variance = numeric_limits<float>::min();
    int total = height*width;

    for (int i = 0; i <= 255; i++)
    	sum += i*histogram[i];

    for (int i = 0; i <= 255; i++)
    {
    	q1 += histogram[i];
    	if (q1 == 0)
    		continue;
    	q2 = total - q1;
    	sum1 += i*histogram[i];
    	mean1 = sum1/q1;
    	mean2 = (sum-sum1)/q2;
    	variance = q1*q2*(mean1-mean2)*(mean1-mean2);
    	if (variance > max_variance){
    		threshold = i;
    		max_variance = variance;
    	}
    }

    for (int i = 0; i < width-1; i++)
    {
    	for (int j = 0; j < height-1; j++){
    		if((int)grayImg.at<uchar>(j, i) > threshold){
    			outputImg.at<float>(j, i) = 255;
    		}    		
    	}
    }
   imwrite(fileName+"_otsu.jpg", outputImg);
}

// Segmentation using canny
else if (method == "canny")
    {
        Mat src = imread( fileName );
        if( !src.data )
        { 
            return -1; 
        }
        cannyDetection(src, fileName);
    }
	return 0;
}

