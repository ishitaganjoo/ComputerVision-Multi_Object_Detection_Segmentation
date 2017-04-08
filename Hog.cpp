#include <stdio.h>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

int main(int argc, char** argv )
{
    // Read an image
    string image_name = argv[1];
    cout<<image_name<<endl;
    Mat img = imread(image_name);
    int rows = img.rows;
    int cols = img.cols;
    
    // Crop out a patch
    

    // Subsample images to size 64*128
    int width = 64;
    int height = 128;
    Mat resizedImg;
    resize(img, resizedImg, Size(width,height));
    cout<< resizedImg.at<double>(10,10)<<endl;
    cout<<resizedImg.rows<<endl;
    //img.convertTo(img, CV_32F, 1/255.0);
 
   // Calculate gradients gx, gy
   //Mat gx, gy; 
   //Sobel(img, gx, CV_32F, 1, 0, 1);
   // Sobel(img, gy, CV_32F, 0, 1, 1);
   // Mat mag, angle; 
   // cartToPolar(gx, gy, mag, angle, 1); 

   // cout<< mag.at<double>(10,10)<<endl;
    return 0;
}
