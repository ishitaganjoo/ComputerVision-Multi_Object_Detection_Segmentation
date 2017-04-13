#include <stdio.h>
#include <Sift.h>
#include <vector>
#include <opencv2/opencv.hpp>
#include "CImg.h"

using namespace cv;
using namespace std;
using namespace cimg_library;

void calculateSift(CImg<double> inputImage){

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
		string fileName = "sift.png";
		inputImage.get_normalize(0, 255).save(fileName.c_str());
		//return descriptors;
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
	cout<<"inside loop "<<finalFeatureVector->at(i)<<endl;
	if(finalFeatureVector->at(i) < 0){
		finalFeatureVector->at(i) = 0;
	}
    val += finalFeatureVector->at(i) * finalFeatureVector->at(i);
  }
	cout<<"value before is "<<val<<endl;
  val = sqrt(val);
	cout<<"value is "<<val<<endl;
  for(int i=0; i<finalFeatureVector->size(); i++){
    finalFeatureVector->at(i) = finalFeatureVector->at(i) / val;
  }
	cout <<"Tatti "<<finalFeatureVector->at(0)<<endl;
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

    //cout<<image_name<<endl;
    Mat img = imread(image_name, CV_LOAD_IMAGE_COLOR);
	  img.convertTo(img, CV_32F, 1/255.0);
    int rows = img.rows;
    int cols = img.cols;
    int depth = CV_16S;
    vector< vector< float> > v_descriptorValues;
    vector< vector< Point>> v_locations;

    // Crop out a patch


    // Subsample images to size 64*128
    int width = 64;
    int height = 128;
    Mat resizedImg, resizedGray;
    resize(img, resizedImg, Size(width, height));
	  //GaussianBlur(resizedImg, resizedImg, Size(3,3), 0, 0, BORDER_DEFAULT);
    cvtColor(resizedImg, resizedGray, CV_BGR2GRAY);
    cout<< resizedImg.rows<<endl;
    //cout<<resizedImg.rows<<endl;
    //img.convertTo(img, CV_32F, 1/255.0);

   // Calculate gradients gx, gy
   Mat gx, gy;
   Mat mag, angle;
   Mat angle2 = Mat(height,  width, CV_64F);
   Mat mag2 = Mat(height, width, CV_64F);

   Sobel(resizedGray, gx,  CV_32F, 1, 0, 3);
   Sobel(resizedGray, gy, CV_32F, 0, 1, 3);
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
			cout<<"Ishita "<<histogramBins[i][j]<<"i "<<i<<" j "<<j<<endl;
		}
	}
  normalizeFeatureVector(&finalFeatureVector);

    for(int i=0; i<finalFeatureVector.size(); i++){
      cout<<finalFeatureVector[i]<<endl;
    }

    Mat finalImage = createImage(finalFeatureVector);

    // //CImg for SIFT
     CImg<double> imageCImg(image_name.c_str());
    //
     calculateSift(imageCImg);

    //CImg Ending

   imwrite("test.jpg", resizedGray);
   imwrite("finalImage.jpg", finalImage);
   return 0;
}
