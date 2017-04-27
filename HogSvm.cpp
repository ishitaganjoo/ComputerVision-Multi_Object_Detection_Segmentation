#include <dirent.h>
#include <string>
#include <stdio.h>
#include <vector>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
//#include <opencv2/objdetect/detection_based_tracker.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <ctime>
#include <iostream>
#include <iomanip>
#include <stdlib.h>
#include <string>
#include <vector>
#include <sys/types.h>
#include <dirent.h>
#include <map>
#include <numeric>
#include <fstream>
#include <sstream>
#include <iterator>


using namespace std;
using namespace cv;

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

// Get training files
vector<string> files_in_directory(const string &directory, bool prepend_directory = false)
{
   //cout<<"File list"<<directory<<endl;
  vector<string> file_list;
  DIR *dir = opendir(directory.c_str());
  if(!dir)
    throw std::string("Can't find directory " + directory);
  
  struct dirent *dirent;
  while ((dirent = readdir(dir))) {

    if(dirent->d_name[0] != '.')
      file_list.push_back(dirent->d_name);
  }

  closedir(dir);

  return file_list;
}


//Hog calculation
vector<float> calculateHog(Mat img)
{

HOGDescriptor hog;
vector<float> desc;
vector<Point>locs;
resize(img, img, Size(100, 200));
hog.compute(img, desc, Size(64, 128), Size(8, 8), locs);
return desc;
}

// Neighbours

void neighbours(int width, int c, int r, int window, int height, int windowH, Mat img)
{
    int predictionValue = 0;
    ofstream output;
    output.open("test_svm.dat", ios::app);
    //imwrite("igot.jpg", img);
     cout<<"in neigh  "<< c<< " "<< r<<endl;
   if ( width > (c+window) && height > (r+windowH)){
        //cout<< "In if"<<endl;
    // ofstream output("test_svm.dat");
     output<<"1"<<" ";
     Mat smallImage = Mat(img, Rect(c, r, window, windowH));
     imwrite("small.jpg", smallImage);
     //Mat grayImgTest = imread("small.jpg");
     vector<float> hogFeatures = calculateHog(smallImage);
     int end = hogFeatures.size();
     for (int i = 0; i < end-1; i++){
        //cout<< hogFeatures[i]<<endl;
        if (hogFeatures[i] != 0){
        output<<(i+1)<<":"<<hogFeatures[i]<<" ";
         }
     }
     if (hogFeatures[end-1] != 0)
     output<<end<<":"<<hogFeatures[end-1];
     output<<endl;  
     
    
    }
    //return predictionValue;
output.close();
}

// Sliding window
vector<Rect> slidingWindow(Mat img, vector<pair<string, Point> >* labels)
{
map<int, string> classLabels;
classLabels[1] = "person";
classLabels[2] = "bird";
classLabels[3] = "cat";
classLabels[4] = "cow";
classLabels[5] = "dog";
classLabels[6] = "horse";
classLabels[7] = "sheep";
classLabels[8] = "aeroplane";
classLabels[9] = "bicycle";
classLabels[10] = "boat";
classLabels[11] = "bus";
classLabels[12] = "car";
classLabels[13] = "motorbike";
classLabels[14] = "train";
classLabels[15] = "bottle";
classLabels[16] = "chair";
classLabels[17] = "diningtable";
classLabels[18] = "pottedplant";
classLabels[19] = "sofa";
classLabels[20] = "tvmonitor";  
vector<Rect> rectangles;    
int height = img.rows;
int width = img.cols;
cout << height<<endl;
cout<< width<<endl;
vector<int> windowSize = {width/2- 5};
vector<int> windowSizeHeight = {height/2 - 5};
for (int i = 0; i < windowSize.size(); i++)
{
int window = windowSize[i];
for (int j = 0; i < windowSizeHeight.size(); j++)
{
   int windowH = windowSizeHeight[j];


    for (int c = 3; c < width; c+=window+3)
    { 
        for (int r = 3; r < height; r+=windowH+3)
        {   
        ofstream output("test_svm.dat");
        int counts[20] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};  
        cout<<"in SW "<< c<< " "<< r<<endl;
         for (int c1 = -3; c1 <=3; c1++){
           for (int r1 = -3; r1 <=3; r1++)
           neighbours(width, c+c1, r+r1, window, height, windowH, img);

         }
         
        //counts[down-1]++;
       
      
    // figure prediction for this using svm
    string str = "./svm_multiclass_classify test_svm.dat model prediction";
    const char *command = str.c_str();
    system(command);
    //Read the prediction 
    string line;
    ifstream file;

    file.open("prediction");
   
   int predictionValue;
    while(getline(file, line)) {
    cout <<"line is "<<line<<endl;
    istringstream ss(line);
    ss >> predictionValue;
    counts[predictionValue-1]++;
     ss.str(string());
    }
    //cout << "size is "<< discriminant.size()<<endl;
    
    //cout <<"Confi is "<< discriminant[predictionValue-1]<<endl;
    
    
    file.close();

        
           // if (left !=0 && right != 0 && curr !=0 && up !=0 && down != 0 && left == right && right == curr && curr == up && up == down){
           for (int count = 0; count < 20; count++){
        if (counts[count] > 25){ 
        cout<<"Prediction is "<< (count+1)<<endl;
         rectangles.push_back(Rect(c, r, window, windowH)); 
         labels->push_back(make_pair(classLabels[count+1], Point (c, r)));
          }
     }
   }
}
}
}
return rectangles;
}


// Write the classified file
void writeToFile(string fileName, vector<Rect> rectangles, vector<pair<string, Point> > labels, Mat img)
{
for (int i = 0; i < rectangles.size(); i++)
{
    rectangle(img, rectangles[i], Scalar(255, 0, 0), 1);
    for (int j = 0 ; j < labels.size(); j++){
        if (rectangles[i].tl() == labels[j].second)
            putText(img, labels[j].first, Point(labels[j].second.x + 25, labels[j].second.y + 25), FONT_HERSHEY_PLAIN, 1.5, Scalar(255, 0, 0));
}
}
imwrite(fileName, img);
}

int main(int argc, char** argv ){
string mode = argv[1];
if (mode == "Train")
{


ofstream output("train_svm_1.dat");
vector<string> class_list = files_in_directory(mode+"/VOCdevkit/VOC2007/JPEGImages");
int accurate = 0;
// Read all files for testing
for (int i = 0; i < class_list.size(); i++)
{
string fileTrain = class_list[i];
int indexExt = fileTrain.find_last_of(".");
Mat grayImgTrain = imread(mode+"/VOCdevkit/VOC2007/JPEGImages/"+fileTrain);
// Read the annotation
ifstream file;
file.open(mode+"/VOCdevkit/VOC2007/Annotations/"+fileTrain.substr(0, indexExt)+".xml");
string line;
bool foundObject = false;
bool foundPart = false;
int count = 0;
 
while(getline(file, line)) {
    int openBracket = line.find_first_of("<");
    int closeBracket = line.find_first_of(">");
    string className;
    int x, y, height, width;
    string tagName = line.substr(openBracket + 1, closeBracket - openBracket - 1);
    //cout <<tagName<<endl;
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
                //cout <<"class name "<<className<<endl;

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
                cout<< grayImgTrain.rows<<endl;
                cout<< grayImgTrain.cols<<endl;
                cout << x<<" "<<y<<" "<<width<<" "<<height<<endl;
                Mat im = Mat (grayImgTrain, Rect(x, y, height, width));
                imwrite("train_small.jpg", im);

            }
    }
            if (count == 5)
            {
                count = 0;
                output<< classLabels[className]<<" ";
                Mat smallImage = Mat (grayImgTrain, Rect(x, y, height, width));
                 vector<float> hogFeatures = calculateHog(smallImage);
                 int end = hogFeatures.size();
                    for (int i = 0; i < end-1; i++){
                        if (hogFeatures[i] != 0)
                            output<<(i+1)<<":"<<hogFeatures[i]<<" ";        
                        }
                    if (hogFeatures[end-1] != 0)
                        output<<end<<":"<<hogFeatures[end-1];
                    output<<endl;           
            }
}

}
                    flush(output);
                    // Train svm
                    string str = "./svm_multiclass_learn -c 0.1 -t 2 train_svm.dat model";
                    const char *command = str.c_str();
                    system(command);
}




else if (mode == "Test")
{
vector<string> class_list = files_in_directory(mode+"/VOCdevkit/VOC2007/JPEGImages");
// Read all files, calclulate hog and create svm training file
for (int i = 0; i < class_list.size(); i++)
{
//http://stackoverflow.com/questions/22314949/compare-two-bounding-boxes-with-each-other-matlab
string fileName = class_list[i];
Mat grayImg = imread(fileName);
vector<Rect> groundTruth;
vector<string> groundLabels;  
// Read the annotation
ifstream file;
file.open(mode+"/VOCdevkit/VOC2007/Annotations/"+fileTrain.substr(0, indexExt)+".xml");
string line;
bool foundObject = false;
bool foundPart = false;
int count = 0;
while(getline(file, line)) {
    int openBracket = line.find_first_of("<");
    int closeBracket = line.find_first_of(">");
    string className;
    int x, y, xmax, ymax;
    string tagName = line.substr(openBracket + 1, closeBracket - openBracket - 1);
    //cout <<tagName<<endl;
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
                //cout <<"class name "<<className<<endl;

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
                xmax = atoi(line.substr(closeBracket + 1, lastBracket - closeBracket - 1).c_str());
            }
            else if (tagName == "ymax")
            {
                count++;
                int lastBracket = line.find_last_of("<");
                ymax = atoi(line.substr(closeBracket + 1, lastBracket - closeBracket - 1).c_str());
               
                foundPart = false;
                foundObject = false;
                cout <<fileName<<endl;
                cout << x<<" "<<y<<" "<<ymax<<" "<<xmax<<endl;
                }
    }
            if (count == 5)
            {
                count = 0;
                groundTruth.push_back(Rect(x, y, xmax, ymax));// store coordinates
                groundLabels.push_back(className);// store label
    
            }
        }
vector<Rect> rectangles;
vector<pair<string, Point> > labels;
// Sliding window
rectangles = slidingWindow(grayImg, &labels);
cout<<"before"<<rectangles.size()<<endl;
int length = rectangles.size();
for( int i = 0; i < length; i++ )
{
    rectangles.push_back(Rect(rectangles[i]));
}
groupRectangles(rectangles, 1, 0.4);
cout<<rectangles.size()<<endl;
// Write to file
writeToFile("Original.jpg", rectangles, labels, grayImg);
//SI= Max(0, Min(XA2, XB2) - Max(XA1, XB1)) * Max(0, Min(YA2, YB2) - Max(YA1, YB1))
// calculate accuracy

}
}

    return 0;
}
