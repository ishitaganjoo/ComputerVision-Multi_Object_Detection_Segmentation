#include <stdio.h>
#include <iostream>
#include <fstream>
#include <dirent.h>
#include <Sift.h>
#include <vector>
#include <opencv2/opencv.hpp>
#include <SImage.h>
#include <SImageIO.h>
#include <cmath>
#include "CImg.h"

using namespace cv;
using namespace std;
using namespace cimg_library;

typedef map<string, vector<string> > Dataset;
Mat src, src_gray;
Mat dst, detected_edges;

int edgeThresh = 1;
int lowThreshold = 30;
int const max_lowThreshold = 100;
int ratio = 3;
int kernel_size = 3;
char* window_name = "EdgeMap.jpg";
RNG rng(12345);
int localizedCount = 1;

double calculateDistance(double x, double y, double x1, double y1){

  return sqrt((pow((x - x1), 2)) + (pow((y - y1), 2)));

}

void combineClusters(int desc1, int desc2, vector< vector<SiftDescriptor> > &descArray){

  //cout<<"length "<<descArray[desc1].size()<<endl;
  for(int i=0; i<descArray[desc2].size(); i++){
    descArray[desc1].push_back(descArray[desc2][i]);
  }

  descArray.erase(descArray.begin()+ desc2);

}

void aggClustering(vector<SiftDescriptor> siftDescriptors, Mat inputImage, int classLabel){

  vector< vector<SiftDescriptor> > myArray(siftDescriptors.size());

  cout<<myArray.size()<<endl;
  cout<<myArray[1].size()<<endl;

  for(int i=0; i<siftDescriptors.size(); i++){
    myArray[i].push_back(siftDescriptors[i]);
    //myArray[i].push_back.col = siftDescriptors[i].col;
  }

  double distance  = 0;
  while(myArray.size() > 3){

    double minVal = 10000;
    int desc1 = -1;
    int desc2 = -1;

    for(int i=0; i<myArray.size(); i++){
      double row1 = 0;
      double col1 = 0;
      if(myArray[i].size() > 1){
        for(int k=0; k<myArray[i].size(); k++){
          row1 += myArray[i][k].row;
          col1 += myArray[i][k].col;
        }
        row1 = row1/myArray[i].size();
        col1 = col1/myArray[i].size();
      }
      else{
        row1 = myArray[i][0].row;
        col1 = myArray[i][0].col;
      }
      for(int j=0; j<myArray.size(); j++){
        double row2 = 0;
        double col2 = 0;
        if(myArray[j].size() > 1){
          for(int k=0; k<myArray[j].size(); k++){
            row2 += myArray[j][k].row;
            col2 += myArray[j][k].col;
          }
          row2 = row2/myArray[j].size();
          col2 = col2/myArray[j].size();
        }
        else{
          row2 = myArray[j][0].row;
          col2 = myArray[j][0].col;
        }
        distance = calculateDistance(row1, col1, row2, col2);

        if(i != j &&  distance < minVal){
          minVal = distance;
          desc1 = i;
          desc2 = j;
        }
      }
    }
    //cout<<"Desc 1 "<<desc1<<" desc2 "<<desc2<<" distance "<<minVal<<endl;
    //cout<<"distance between clusters is :"<<minVal<<endl;
    combineClusters(desc1, desc2, myArray);
    //cout<<"length of original "<<myArray.size()<<endl;
  }
  int count = 0;
  while(myArray.size() > 0){
    int max = 0;
    int index = 0;
    for(int i=0; i<myArray.size(); i++){
      if(max < myArray[i].size()){
        max = myArray[i].size();
        index = i;
      }
    }

    double minRow = 10000;
    double minCol = 10000;
    double maxRow = 0;
    double maxCol = 0;
    for(int i=0; i<myArray[index].size(); i++){
      if(minRow > myArray[index][i].row){
        minRow = myArray[index][i].row;
      }
      if(minCol > myArray[index][i].col){
        minCol = myArray[index][i].col;
      }
      if(maxRow < myArray[index][i].row){
        maxRow = myArray[index][i].row;
      }
      if(maxCol < myArray[index][i].col){
        maxCol = myArray[index][i].col;
      }
    }

    Point topLeft = Point(minCol, minRow);
    Point bottomRight = Point(maxCol, maxRow);

    //const unsigned char color[] = {rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255)};
    Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );

    const unsigned int boundary= 1;
	double rectWidth = maxCol - minCol;
	double rectHeight = maxRow - minRow;
	//Mat src;//Source image load here
	Rect R(minCol, minRow, rectWidth, rectHeight); //Create a rect

	Mat ROI = inputImage(R); //Crop the region of interest using above rect

	string sift_localize = "LocalizedImages/";
  sift_localize += "sift_localize_";
	sift_localize += to_string(localizedCount);
	sift_localize += ".jpg";
  //   ofstream of( "testImages\\" );
	// of<< imwrite(sift_localize, ROI);
    //inputImage.draw_rectangle(minCol, minRow, maxCol, maxRow, color, 0.1);
    rectangle(inputImage, topLeft, bottomRight, color);
    imwrite(sift_localize, ROI);


    cout<<"Max "<<max;
    localizedCount++;
    // for(int i=0; i<myArray[index].size(); i++)
    //   {
    //     // for(int j=0; j<5; j++)
    //     //   for(int k=0; k<5; k++)
    //     // if(j==2 || k==2)
    //     //   for(int p=0; p<3; p++)
    //         //inputImage(myArray[index][i].col+k-1, myArray[index][i].row+j-1, 0, p)=0;
    //     inputImage.draw_point(myArray[index][i].col, myArray[index][i].row, color);
    //     // Point pixelVal = Point(myArray[index][i].row, myArray[index][i].col);
    //     // inputImage.at<Scalar>(pixelVal) = color;
    //
    //   }
      myArray.erase(myArray.begin()+index);
  }

    string testFileName = "GenerateImages/";
    testFileName += to_string(classLabel);
    testFileName += "_sift_2.png";
    cout<<testFileName<<endl;
    imwrite(testFileName, inputImage);
    //inputImage.get_normalize(0, 255).save("sift_2.png");

}

void CannyThreshold(int, void*)
{
  /// Reduce noise with a kernel 3x3
  blur( src_gray, detected_edges, Size(3,3) );

  /// Canny detector
  Mat cdst;
  Canny( detected_edges, detected_edges, lowThreshold, lowThreshold*ratio, kernel_size );
  cvtColor(detected_edges, cdst, CV_GRAY2BGR);
  vector<Vec4i> lines;
  HoughLinesP(detected_edges, lines, 1, CV_PI/180, 50, 50, 10 );
    for( size_t i = 0; i < lines.size(); i++ )
    {
      Vec4i l = lines[i];
      line( cdst, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0,0,255), 3, CV_AA);
    }

  imwrite( "detectedLinesHough.jpg", cdst );
  /// Using Canny's output as a mask, we display our result
  // dst = Scalar::all(0);
  //
  // Mat addweight;
  //
  // src.copyTo( dst, detected_edges);
  // cvtColor(detected_edges, detected_edges, CV_GRAY2BGR); // convert canny image to bgr
  // addWeighted( src, 0.5, detected_edges, 0.5, 0.0, addweight); // blend src image with canny image
  // src += detected_edges; // add src image with canny image
  // imwrite( window_name, dst );


 }

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

void hysterisisThresholding(Mat output_after_nms, Mat output_hysterisis)
{
  imwrite("isnideHt.jpg", output_after_nms);
  cout<<"HT"<<endl;
int height = output_after_nms.rows;
int width = output_after_nms.cols;
double low = 140.0, high = 240.0;
 for (int r = 2; r< height - 2; r++)
 {
   	for (int c = 2; c<width-2; c++)
   	{
      if (output_after_nms.at<double>(r, c) >= high )
      	//output_hysterisis[r][c] = output_after_nms[r][c];
      		output_hysterisis.at<double>(r, c) = 255.0;

      else if (output_after_nms.at<double>(r, c) > low && output_after_nms.at<double>(r, c) <high)
      {
        bool found = false;

        if (foundEdge(output_after_nms, 1, r, c))
        {
        	found = true;
        	//output_hysterisis[r][c] = output_after_nms[r][c];
        	output_hysterisis.at<double>(r, c) = 255.0;
        }
        else  if (!found && foundEdge(output_after_nms, 2, r, c))
        {
        //output_hysterisis[r][c] = output_after_nms[r][c];
        		output_hysterisis.at<double>(r, c) = 255.0;

        }
      }
    }
  }
  imwrite("afterinsideHt.jpg",output_hysterisis);
}

void nonMaxSuppression(Mat output_after_nms, Mat output, Mat orientation)
{
  cout<<"Inside nms"<<endl;
 int height = output.rows;
 int width = output.cols;
 for (int r = 1; r< height - 1; r++)
 {
 	for (int c = 1; c<width-1; c++)
 	{
 	double orient = orientation.at<double>(r, c);
 	double magnitude = output.at<double>(r, c);
  //cout<<r<<" "<<c<<" "<<"orient "<<orient<<" magnitude "<<magnitude<<endl;
 	if ((orient == 0) && magnitude >= max(output.at<double>(r, c-1), output.at<double>(r, c+1)))
 		output_after_nms.at<double>(r, c) = magnitude;
 	else if ((orient == 45) && magnitude >= max( output.at<double>(r+1, c-1), output.at<double>(r-1, c+1)))
 		output_after_nms.at<double>(r, c) = magnitude;

 	else if ((orient == 90) && magnitude >= max (output.at<double>(r-1, c), output.at<double>(r+1, c) ))
 		output_after_nms.at<double>(r, c) = magnitude;
    else if ((orient == 135) && magnitude >= max (output.at<double>(r+1, c+1), output.at<double>(r-1, c-1) ))
 		output_after_nms.at<double>(r, c) = magnitude;
 	//cout << "output"<< output_after_nms[r][c];
 	//cout <<"\n";

 	}
 }
}

Mat convolve_general(Mat input, Mat filter)
{

  // Convolution code here
  Mat output = Mat(input.rows, input.cols, CV_64F);
  Mat input_convolve = Mat(input.rows+2, input.cols+2, CV_64F);
  int HEIGHT = input.rows;
  int WIDTH = input.cols;
  int height_inputC = input_convolve.rows;
  int width_inputC = input_convolve.cols;

  for(int r = 1; r < HEIGHT-1 ; r++)
  {
   for(int c = 1; c < WIDTH-1 ; c++)
   {
      input_convolve.at<double>(r, c) = input.at<double>(r-1, c-1);
   }
  }

  for(int c=0; c < WIDTH; c++)
  {
     input_convolve.at<double>(0, c+1) = input.at<double>(0, c);
     input_convolve.at<double>(height_inputC-1, c+1) = input.at<double>(HEIGHT-1, c);
  }

  for(int r = 0; r < HEIGHT; r++)
  {
     input_convolve.at<double>(r+1, 0) = input.at<double>(r, 0);
     input_convolve.at<double>(r+1, width_inputC-1) = input.at<double>(r, WIDTH-1);
  }
  //Boundary pixel values:
  input_convolve.at<double>(0, 0) = input.at<double>(0, 0);
  input_convolve.at<double>(0, width_inputC-1) = input.at<double>(0, WIDTH-1);
  input_convolve.at<double>(height_inputC-1, 0) = input.at<double>(HEIGHT-1, 0);
  input_convolve.at<double>(height_inputC-1, width_inputC-1) = input.at<double>(HEIGHT-1, WIDTH-1);
  for(int row = 1; row < height_inputC-1; row++)
  {
    for(int col = 1; col < width_inputC-1; col++ )
    {
        float addNeighbours = 0;
        float addFilter = 0;
        for(int i = -1; i <= 1; i++ )
        {
                for(int j = -1; j <= 1; j++ )
            {
                    double k = input_convolve.at<double>(row+i, col+j);
                    addNeighbours += k * filter.at<double>(1-i, 1-j);
                    addFilter += filter.at<double>(1-i, 1-j);
                }
        }
        output.at<double>(row-1, col-1) = (double)(addNeighbours/addFilter);
    }
  }
  return output;
}

Mat outputGradientOperator(Mat input_convolve, Mat filter)
{
 int height_inputC = input_convolve.rows;
 int width_inputC = input_convolve.cols;
 Mat output = Mat(height_inputC-2, width_inputC-2, CV_64F);
  for(int row = 1; row < height_inputC-1; row++)
  {
    for(int col = 1; col < width_inputC-1; col++ )
    {
        float addNeighbours = 0;
        float addFilter = 0;
        for(int i = -1; i <= 1; i++ )
        {
                for(int j = -1; j <= 1; j++ )
            {
                    double k = input_convolve.at<double>(row+i, col+j);
                    addNeighbours += k * filter.at<double>(1+i, 1+j);
                }
        }
        output.at<double>(row-1, col-1) = (double)(addNeighbours);
    }
  }
  //cout<<"convolution done"<<endl;
  return output;
}

Mat reflectImage(Mat input)
{

  Mat input_convolve = Mat(input.rows+2, input.cols+2, CV_64F);
  int HEIGHT = input.rows;
  int WIDTH = input.cols;
  int height_inputC = input_convolve.rows;
  int width_inputC = input_convolve.cols;

  for(int r = 1; r < HEIGHT-1 ; r++)
  {
   for(int c = 1; c < WIDTH-1 ; c++)
   {
      input_convolve.at<double>(r, c) = input.at<double>(r-1, c-1);
   }
  }

  for(int c=0; c < WIDTH; c++)
  {
     input_convolve.at<double>(0, c+1) = input.at<double>(0, c);
     input_convolve.at<double>(height_inputC-1, c+1) = input.at<double>(HEIGHT-1, c);
  }

  for(int r = 0; r < HEIGHT; r++)
  {
     input_convolve.at<double>(r+1, 0) = input.at<double>(r, 0);
     input_convolve.at<double>(r+1, width_inputC-1) = input.at<double>(r, WIDTH-1);
  }
  //Boundary pixel values:
  input_convolve.at<double>(0, 0) = input.at<double>(0, 0);
  input_convolve.at<double>(0, width_inputC-1) = input.at<double>(0, WIDTH-1);
  input_convolve.at<double>(height_inputC-1, 0) = input.at<double>(HEIGHT-1, 0);
  input_convolve.at<double>(height_inputC-1, width_inputC-1) = input.at<double>(HEIGHT-1, WIDTH-1);

  //cout<<"new image created"<<endl;
  return input_convolve;
}

Mat sobel_gradient_filter(Mat input, bool _gx)
{
  //cout<<"inside sobel gradient filter"<<endl;
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

  //Mat input_convolve = reflectImage(input);

  //imwrite("insideSobel.jpg", input_convolve);

  //Mat output_Gx = outputGradientOperator(input, sobel_Gx);

  Mat magnitude, angle;

  cout<<"before sobel"<<endl;
  Sobel(input, output_Gx,  CV_64F, 1, 0, 3);
  Sobel(input, output_Gy,  CV_64F, 0, 1, 3);
  //cartToPolar(output_Gx, output_Gy, magnitude, angle, true);


  //Mat output_Gy = outputGradientOperator(input, sobel_Gy);
  //apply sobel
  cout<<"After sobel "<<endl;

  double pi = 180.0;
  //double count = 0;
  for(int r = 0; r < rows; r++)
  {
   for(int c = 0; c < cols; c++)
   {
     //count++;
      output.at<double>(r, c) = sqrt (pow(output_Gx.at<double>(r, c) , 2) + pow(output_Gy.at<double>(r, c) , 2));

     // output[r][c] = 0;
      double tangX = output_Gx.at<double>(r, c);
      double tangY = output_Gy.at<double>(r, c);
      double temp = 0.0;
      temp = tangY/tangX;
      //cout<<temp<<endl;
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

        //cout<<"temp"<<temp<<endl;
      	orientation.at<double>(r, c) = temp;
   }

   imwrite("cannyorient.jpg", orientation);
   imwrite("cannymag.jpg", output);

  }

nonMaxSuppression(output_after_nms, output, orientation);
imwrite("afternms.jpg", output_after_nms);
hysterisisThresholding(output_after_nms, output_hysterisis);
imwrite("afterht.jpg", output_hysterisis);
return output_hysterisis;

}

void cannyDetection(Mat originalImage){
cout<<"inside canny "<<endl;
  Mat grayImage;

  cvtColor(originalImage, grayImage, CV_BGR2GRAY);
  imwrite("cannygrayimage.jpg", grayImage);
  //cout<<resizedImg.rows<<endl;
  //img.convertTo(img, CV_32F, 1/255.0);
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
  // int count = 0;
  // while(count < 8)
  // {
  //    output_image_demo = convolve_general(grayImage, gaussian_filter);
  //    grayImage = output_image_demo;
  //    count ++;
  // }

  cout<<"before converting "<<endl;
  Mat outputGxSobel = Mat(originalImage.rows, originalImage.cols, CV_64F);
  outputGxSobel = sobel_gradient_filter(grayImage, true);
  imwrite("finalCanny.jpg", outputGxSobel);

}


/* Code snippet by Prof. Crandall */
vector<string> files_in_directory(const string &directory, bool prepend_directory = false)
{
  vector<string> file_list;
  DIR *dir = opendir(directory.c_str());
  if(!dir)
    throw std::string("Can't find directory " + directory);

  struct dirent *dirent;
  while ((dirent = readdir(dir)))
    if(dirent->d_name[0] != '.')
      file_list.push_back((prepend_directory?(directory+"/"):"")+dirent->d_name);

  closedir(dir);
  return file_list;
}

vector<SiftDescriptor> calculateSift(CImg<double> inputImage, int count){

  CImg<double> grayImage = inputImage.get_RGBtoHSI().get_channel(2);
  vector<SiftDescriptor> descriptors = Sift::compute_sift(grayImage);
	for(int i=0; i<descriptors.size(); i++)
		{
			for(int j=0; j<5; j++)
				for(int k=0; k<5; k++)
			if(j==2 || k==2)
				for(int p=0; p<3; p++)
					inputImage(descriptors[i].col+k-1, descriptors[i].row+j-1, 0, p)=0;

		}
		string fileName = "sift_";
    fileName += to_string(count);
    fileName += ".png";
		inputImage.get_normalize(0, 255).save(fileName.c_str());
		return descriptors;
}

Mat createImage(vector<double> featureVector){
  Mat hogImage = Mat(35,35, CV_64F);
  int count = 1;
  for(int i=0; i<35; i++){
    for(int j=0; j<35; j++){
      if(count < featureVector.size()){
        hogImage.at<double>(i, j) = featureVector.at(count);
      }
      else{
        hogImage.at<double>(i, j) = 0;
      }
      count++;
    }
  }
  return hogImage;
}

void normalizeFeatureVector(vector<double>* finalFeatureVector){
  double val = 0;
  for(int i=0; i<finalFeatureVector->size(); i++){
	if(finalFeatureVector->at(i) < 0){
		finalFeatureVector->at(i) = 0;
	}
    val += finalFeatureVector->at(i) * finalFeatureVector->at(i);
  }
  val = sqrt(val);
  for(int i=0; i<finalFeatureVector->size(); i++){
    finalFeatureVector->at(i) = finalFeatureVector->at(i) / val;
  }
}


void saveBin(vector<double>* currentBin, double angle, double magnitude)
{
	//cout<<"inside"<<endl;
  //vector<double> histogramBins(9);
  int rem = (int)angle % 20;
  int val = angle / 20.0;
//	cout<<"val "<<val<<endl;
  if(rem == 0)
  {
	if(val < 8){
    		currentBin->at(val) += magnitude;
	}
	else{
		currentBin->at(0) += magnitude;
	}
  }
  else{
    double newVal = rem/20 * magnitude;
    currentBin->at(val) += newVal;
    if(val<8){
      currentBin->at(val+1) += magnitude-newVal;
    }
    else{
      currentBin->at(0) += magnitude-newVal;
    }
  }
  //return histogramBins;
}
int main(int argc, char** argv )
{
    // Read an image
    string image_name = argv[1];
    string mode = "Random/Random1";

  src = imread( argv[1] );

  cannyDetection(src);

 if( !src.data )
 { return -1; }

 /// Create a matrix of the same type and size as src (for dst)
 dst.create( src.size(), src.type() );

 /// Convert the image to grayscale
 cvtColor( src, src_gray, CV_BGR2GRAY );

    Dataset filenames;
    vector<string> class_list = files_in_directory(mode);

    // cout<<class_list[0]<<endl;
    //
    // for(int i=0; i<class_list.size(); i++){
    //   cout<<class_list[i]<<endl;
    // }

    Mat img = imread(image_name, CV_LOAD_IMAGE_COLOR);

	  //img.convertTo(img, CV_32F, 1/255.0);
    int rows = img.rows;
    int cols = img.cols;
    int depth = CV_16S;

    // Crop out a patch


    // Subsample images to size 64*128
    int width = 64;
    int height = 128;
    Mat resizedImg, resizedGray;
    resize(img, resizedImg, Size(width, height));
	  //GaussianBlur(resizedImg, resizedImg, Size(3,3), 0, 0, BORDER_DEFAULT);
    cvtColor(resizedImg, resizedGray, CV_BGR2GRAY);
    //cout<< resizedImg.rows<<endl;
    //cout<<resizedImg.rows<<endl;
    //img.convertTo(img, CV_32F, 1/255.0);

   // Calculate gradients gx, gy
   Mat gx, gy;
   Mat mag, angle;
   Mat angle2 = Mat(height,  width, CV_64F);
   Mat mag2 = Mat(height, width, CV_64F);

   Sobel(resizedGray, gx,  CV_64F, 1, 0, 3);
   Sobel(resizedGray, gy, CV_64F, 0, 1, 3);
   cartToPolar(gx, gy, mag, angle, true);

	//cout<<atan2(gx.at<double>(1,1), gy.at<double>(1,1))<<endl;
	//double val = atan2(gx.at<double>(1,1), gy.at<double>(1,1));

	//val = val*(180.0/3.14);

	//cout<<val<<endl;

    Mat smallImage = Mat(resizedGray, Rect(0, 0, 8, 8));

    for(int i=0; i<height; i++){
       for(int j=0; j<width; j++){
			double val = 0;
			double xVal = gx.at<double>(i, j);
			double yVal = gy.at<double>(i, j);
			val = atan2(xVal, yVal);
			val = val*(180.0/CV_PI);
			double xValSqr = xVal*xVal;
			double yValSqr = yVal*yVal;
			if(val < 0){
				val = abs(val);
			}
			angle2.at<double>(i, j) = val;
			val = sqrt(xValSqr + yValSqr);
			mag2.at<double>(i, j) = val;
			//cout<<angle2.at<double>(i,j)<<" ";
	}
     }

  //cout<<"height and width are"<<angle2.rows <<" "<<angle2.cols<<endl;
   vector< vector<double> > histogramBins;

   for(int i = 0; i<height; i+=8)
   {
    for(int j=0; j<width; j+=8)
    {
	//cout<<"before cutting"<<i<<endl;
	//cout<<"before cutting"<<j<<endl;
      Mat angleImage = Mat(angle2, Rect(j, i, 8, 8));
      Mat magImage = Mat(mag2, Rect(j, i, 8, 8));
	//cout<<"after cutting"<<endl;
      vector<double> histBin(9);
      for(int m = 0; m<8; m++)
      {
        for(int n = 0; n<8; n++)
        {
          saveBin(&histBin, angleImage.at<double>(m,n), magImage.at<double>(m,n));
        }
      }
	//cout<<histBin[0]<<endl;
      histogramBins.push_back(histBin);
	//cout<<"rohil "<<histogramBins[0][0]<<endl;
    }
   }
	cout<<"outside"<<endl;
	vector<double> finalFeatureVector;
	for(int i=0; i<histogramBins.size(); i++){
		for(int j=0; j<9; j++){
			finalFeatureVector.push_back(histogramBins[i][j]);
			//cout<<"Ishita "<<histogramBins[i][j]<<"i "<<i<<" j "<<j<<endl;
		}
	}
  normalizeFeatureVector(&finalFeatureVector);

    // for(int i=0; i<finalFeatureVector.size(); i++){
    //   cout<<finalFeatureVector[i]<<endl;
    // }

    Mat finalImage = createImage(finalFeatureVector);

    // //CImg for SIFT
     //CImg<double> imageCImg(image_name.c_str());
    // CImg<double> imageCImgCopy(image_name.c_str());
    //
    for(int i=0; i<class_list.size(); i++){
      cout<<class_list[i]<<endl;
    }
    for(int i=0; i<class_list.size(); i++){
        cout<<class_list[i]<<endl;
        string readFile = mode;
        readFile += "/";
        readFile += class_list[i];
        CImg<double> imageCImg(readFile.c_str());
        Mat inputImageOCV = imread(readFile, CV_LOAD_IMAGE_COLOR);
      cout<<"before sift"<<endl;
       vector<SiftDescriptor> siftDescriptors = calculateSift(imageCImg, i+1);
       cout<<"after sift"<<endl;
       aggClustering(siftDescriptors, inputImageOCV, i+1);
       cout<<"after clustering"<<endl;
   }

    //CImg Ending

   imwrite("test.jpg", resizedGray);
   imwrite("finalImage.jpg", finalImage);
   return 0;
}
