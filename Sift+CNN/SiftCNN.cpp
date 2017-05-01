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


RNG rng(12345);
int localizedCount = 1;

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

// Calculate distance between points.
double calculateDistance(double x, double y, double x1, double y1){
  return sqrt((pow((x - x1), 2)) + (pow((y - y1), 2)));
}

// Combine clusters
void combineClusters(int desc1, int desc2, vector< vector<SiftDescriptor> > &descArray){
  for(int i=0; i<descArray[desc2].size(); i++){
    descArray[desc1].push_back(descArray[desc2][i]);
  }
  descArray.erase(descArray.begin()+ desc2);
}

// Agglomerative clustering
void aggClustering(vector<SiftDescriptor> siftDescriptors, Mat inputImage, int classLabel){
  vector< vector<SiftDescriptor> > myArray(siftDescriptors.size());
  cout<<myArray.size()<<endl;
  cout<<myArray[1].size()<<endl;
  for(int i=0; i<siftDescriptors.size(); i++){
    myArray[i].push_back(siftDescriptors[i]);
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
    combineClusters(desc1, desc2, myArray);
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
   rectangle(inputImage, topLeft, bottomRight, color);
   imwrite(sift_localize, ROI);
   localizedCount++;
   myArray.erase(myArray.begin()+index);
  }
    string testFileName = "GenerateImages/";
    testFileName += to_string(classLabel);
    testFileName += "_sift_2.png";
    imwrite(testFileName, inputImage);
    //inputImage.get_normalize(0, 255).save("sift_2.png");
}

  // Calculate Sift decsriptors
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

// Main function
int main(int argc, char** argv )
{
  string mode = "Test";
  // Prepare ground truth for all test files
  map<string, int> classLabels;
  classLabels["person"] = 1;
  classLabels["bird"] = 2;
  classLabels["cat"] = 3;
  classLabels["cow"] = 4;
  classLabels["dog"] = 5;
  classLabels["horse"] = 6;
  classLabels["sheep"] = 7;
  classLabels["aeroplane"] = 8;
  classLabels["bicycle"] = 9;
  classLabels["boat"] = 10;
  classLabels["bus"] = 11;
  classLabels["car"] = 12;
  classLabels["motorbike"] = 13;
  classLabels["train"] = 14;
  classLabels["bottle"] = 15;
  classLabels["chair"] = 16;
  classLabels["diningtable"] = 17;
  classLabels["pottedplant"] = 18;
  classLabels["sofa"] = 19;
  classLabels["tvmonitor"] = 20;
  vector<string> class_list = files_in_directory("../"+mode+"/VOCdevkit/VOC2007/JPEGImages");
  int objectCount = 0;
  ofstream output("ground_truth_sift_training.txt");
  // Read all files for testing
  for (int i = 0; i < class_list.size(); i++)
    {
    string fileTrain = class_list[i];
    output << fileTrain << " ";
    int indexExt = fileTrain.find_last_of(".");
    Mat grayImgTrain = imread("../"+mode+"/VOCdevkit/VOC2007/JPEGImages/"+fileTrain);
    // Read the annotation
    ifstream file;
    file.open("../"+mode+"/VOCdevkit/VOC2007/Annotations/"+fileTrain.substr(0, indexExt)+".xml");
    string line;
    bool foundObject = false;
    bool foundPart = false;
    int count = 0;
    string className;
    int x, y, height, width;
    while(getline(file, line)) {
        int openBracket = line.find_first_of("<");
        int closeBracket = line.find_first_of(">");
        string tagName = line.substr(openBracket + 1, closeBracket - openBracket - 1);
        if (tagName == "object")
          foundObject = true;
        else if (tagName == "part")
          foundPart = true;
        else if (foundObject && !foundPart)
          {
            if (tagName == "name")
              {
                count++;
                int lastBracket = line.find_last_of("<");
                className = line.substr(closeBracket + 1, lastBracket - closeBracket - 1);
                cout <<"class name "<<className<<endl;
              }
            else if (tagName == "xmin")
              {
                count++;
                int lastBracket = line.find_last_of("<");
                x = atoi(line.substr(closeBracket + 1, lastBracket - closeBracket - 1).c_str());
              }
            else if (tagName == "ymin")
              {
                count++;
                int lastBracket = line.find_last_of("<");
                y = atoi(line.substr(closeBracket + 1, lastBracket - closeBracket - 1).c_str());
              }
            else if (tagName == "xmax")
            {
                count++;
                int lastBracket = line.find_last_of("<");
                int xmax = atoi(line.substr(closeBracket + 1, lastBracket - closeBracket - 1).c_str());
                height = xmax - x;
            }
            else if (tagName == "ymax")
            {
                count++;
                int lastBracket = line.find_last_of("<");
                int ymax = atoi(line.substr(closeBracket + 1, lastBracket - closeBracket - 1).c_str());
                width = ymax - y;
                foundPart = false;
                foundObject = false;
                cout <<fileTrain<<endl;              
            }
          }
            if (count == 5)
            {
                count = 0;
                objectCount++;
                Mat im = Mat (grayImgTrain, Rect(x, y, height, width));
                output << className << " " << x << " " << y << " " << height << " " << width << " ";
                Mat smallImage = Mat (grayImgTrain, Rect(x, y, height, width));         
            }
}
  output << endl;
  file.close();
}
     
  flush(output);
  cout << "Making directories " << endl;
  string str = "mkdir LocalizedImages";
  const char *command = str.c_str();
  system(command);
  str = "mkdir GenerateImages";
  command = str.c_str();
  system(command);
  cout << "Start SIFT localization" << endl;
  for(int i=0; i<class_list.size(); i++){
  string readFile = "../"+mode+"/VOCdevkit/VOC2007/JPEGImages/"+class_list[i];
  CImg<double> imageCImg(readFile.c_str());
  Mat inputImageOCV = imread(readFile, CV_LOAD_IMAGE_COLOR);
  vector<SiftDescriptor> siftDescriptors = calculateSift(imageCImg, i+1);
  aggClustering(siftDescriptors, inputImageOCV, i+1);
   }
  return 0;
}