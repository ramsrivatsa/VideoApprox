/*
 * Author: Samyak Datta (datta[dot]samyak[at]gmail.com)
 *
 * A program to detect facial feature points using
 * Haarcascade classifiers for face, eyes, nose and mouth
 *
 */

#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <iostream>
#include <cstdio>
#include <vector>
#include <algorithm>

using namespace std;
using namespace cv;

// Functions to parse command-line arguments
static string getCommandOption(const vector<string>&, const string&);
static void setCommandOptions(vector<string>&, int, char**);
static bool doesCmdOptionExist(const vector<string>& , const string&);

// Functions for facial feature detection
static void help();
static void detectFaces(Mat&, vector<Rect_<int> >&, string);
static void detectEyes(Mat&, vector<Rect_<int> >&, string);
static void detectNose(Mat&, vector<Rect_<int> >&, string);
static void detectMouth(Mat&, vector<Rect_<int> >&, string);
static void detectFacialFeaures(Mat&, const vector<Rect_<int> >, string, string, string);

string input_image_path;
string face_cascade_path, eye_cascade_path, nose_cascade_path, mouth_cascade_path;

int main(int argc, char** argv)
{
    if(argc < 3)
    {
        help();
        return 1;
    }

    // Extract command-line options
    vector<string> args;
    setCommandOptions(args, argc, argv);

    input_image_path = argv[1];
    face_cascade_path = argv[2];
    eye_cascade_path = (doesCmdOptionExist(args, "-eyes")) ? getCommandOption(args, "-eyes") : "";
    nose_cascade_path = (doesCmdOptionExist(args, "-nose")) ? getCommandOption(args, "-nose") : "";
    mouth_cascade_path = (doesCmdOptionExist(args, "-mouth")) ? getCommandOption(args, "-mouth") : "";

    // Load image and cascade classifier files
    Mat image,croppedImage;
    image = imread(input_image_path);

    // Detect faces and facial features
    vector<Rect_<int> > faces;
    cout <<"program starts here\n";
    //cout << "size of faces" << faces.size();
    //for (std::size_t f=0; f < faces.size(); f++)
    //    cout << "hello";
    detectFaces(image, faces, face_cascade_path);
    detectFacialFeaures(image, faces, eye_cascade_path, nose_cascade_path, mouth_cascade_path);
    for(unsigned int i = 0; i < faces.size(); ++i) {
        Rect face = faces[i];
	croppedImage = image(Rect(face.x,face.y,face.width,face.height));
    	imshow("Result", croppedImage);
    }

    //imshow("Result", image);

    waitKey(0);
    return 0;
}

void setCommandOptions(vector<string>& args, int argc, char** argv)
{
    for(int i = 1; i < argc; ++i)
    {
        args.push_back(argv[i]);
    }
    return;
}

string getCommandOption(const vector<string>& args, const string& opt)
{
    string answer;
    vector<string>::const_iterator it = find(args.begin(), args.end(), opt);
    if(it != args.end() && (++it != args.end()))
        answer = *it;
    return answer;
}

bool doesCmdOptionExist(const vector<string>& args, const string& opt)
{
    vector<string>::const_iterator it = find(args.begin(), args.end(), opt);
    return (it != args.end());
}

static void help()
{
    cout << "\nThis file demonstrates facial feature points detection using Haarcascade classifiers.\n"
        "The program detects a face and eyes, nose and mouth inside the face."
        "The code has been tested on the Japanese Female Facial Expression (JAFFE) database and found"
        "to give reasonably accurate results. \n";

    cout << "\nUSAGE: ./cpp-example-facial_features [IMAGE] [FACE_CASCADE] [OPTIONS]\n"
        "IMAGE\n\tPath to the image of a face taken as input.\n"
        "FACE_CASCADE\n\t Path to a haarcascade classifier for face detection.\n"
        "OPTIONS: \nThere are 3 options available which are described in detail. There must be a "
        "space between the option and it's argument (All three options accept arguments).\n"
        "\t-eyes : Specify the haarcascade classifier for eye detection.\n"
        "\t-nose : Specify the haarcascade classifier for nose detection.\n"
        "\t-mouth : Specify the haarcascade classifier for mouth detection.\n";


    cout << "EXAMPLE:\n"
        "(1) ./cpp-example-facial_features image.jpg face.xml -eyes eyes.xml -mouth mouth.xml\n"
        "\tThis will detect the face, eyes and mouth in image.jpg.\n"
        "(2) ./cpp-example-facial_features image.jpg face.xml -nose nose.xml\n"
        "\tThis will detect the face and nose in image.jpg.\n"
        "(3) ./cpp-example-facial_features image.jpg face.xml\n"
        "\tThis will detect only the face in image.jpg.\n";

    cout << " \n\nThe classifiers for face and eyes can be downloaded from : "
        " \nhttps://github.com/Itseez/opencv/tree/master/data/haarcascades";

    cout << "\n\nThe classifiers for nose and mouth can be downloaded from : "
        " \nhttps://github.com/Itseez/opencv_contrib/tree/master/modules/face/data/cascades\n";
}

static void detectFaces(Mat& img, vector<Rect_<int> >& faces, string cascade_path)
{
    CascadeClassifier face_cascade;
    face_cascade.load(cascade_path);

    face_cascade.detectMultiScale(img, faces, 1.15, 3, 0|CASCADE_SCALE_IMAGE, Size(30, 30));
    return;
}

static void detectFacialFeaures(Mat& img, const vector<Rect_<int> > faces, string eye_cascade,
        string nose_cascade, string mouth_cascade)
{
    for(unsigned int i = 0; i < faces.size(); ++i)
    {
        // Mark the bounding box enclosing the face
        Rect face = faces[i];
        rectangle(img, Point(face.x, face.y), Point(face.x+face.width, face.y+face.height),
                Scalar(255, 0, 0), 1, 4);

        // Eyes, nose and mouth will be detected inside the face (region of interest)
        Mat ROI = img(Rect(face.x, face.y, face.width, face.height));

        // Check if all features (eyes, nose and mouth) are being detected
        bool is_full_detection = false;
        if( (!eye_cascade.empty()) && (!nose_cascade.empty()) && (!mouth_cascade.empty()) )
            is_full_detection = true;

        // Detect eyes if classifier provided by the user
        if(!eye_cascade.empty())
        {
            vector<Rect_<int> > eyes;
            detectEyes(ROI, eyes, eye_cascade);

            // Mark points corresponding to the centre of the eyes
            for(unsigned int j = 0; j < eyes.size(); ++j)
            {
                Rect e = eyes[j];
                circle(ROI, Point(e.x+e.width/2, e.y+e.height/2), 3, Scalar(0, 255, 0), -1, 8);
                /* rectangle(ROI, Point(e.x, e.y), Point(e.x+e.width, e.y+e.height),
                    Scalar(0, 255, 0), 1, 4); */
            }
        }

        // Detect nose if classifier provided by the user
        double nose_center_height = 0.0;
        if(!nose_cascade.empty())
        {
            vector<Rect_<int> > nose;
            detectNose(ROI, nose, nose_cascade);

            // Mark points corresponding to the centre (tip) of the nose
            for(unsigned int j = 0; j < nose.size(); ++j)
            {
                Rect n = nose[j];
                circle(ROI, Point(n.x+n.width/2, n.y+n.height/2), 3, Scalar(0, 255, 0), -1, 8);
                nose_center_height = (n.y + n.height/2);
            }
        }

        // Detect mouth if classifier provided by the user
        double mouth_center_height = 0.0;
        if(!mouth_cascade.empty())
        {
            vector<Rect_<int> > mouth;
            detectMouth(ROI, mouth, mouth_cascade);

            for(unsigned int j = 0; j < mouth.size(); ++j)
            {
                Rect m = mouth[j];
                mouth_center_height = (m.y + m.height/2);

                // The mouth should lie below the nose
                if( (is_full_detection) && (mouth_center_height > nose_center_height) )
                {
                    rectangle(ROI, Point(m.x, m.y), Point(m.x+m.width, m.y+m.height), Scalar(0, 255, 0), 1, 4);
                }
                else if( (is_full_detection) && (mouth_center_height <= nose_center_height) )
                    continue;
                else
                    rectangle(ROI, Point(m.x, m.y), Point(m.x+m.width, m.y+m.height), Scalar(0, 255, 0), 1, 4);
            }
        }

    }

    return;
}

static void detectEyes(Mat& img, vector<Rect_<int> >& eyes, string cascade_path)
{
    CascadeClassifier eyes_cascade;
    eyes_cascade.load(cascade_path);

    eyes_cascade.detectMultiScale(img, eyes, 1.20, 5, 0|CASCADE_SCALE_IMAGE, Size(30, 30));
    return;
}

static void detectNose(Mat& img, vector<Rect_<int> >& nose, string cascade_path)
{
    CascadeClassifier nose_cascade;
    nose_cascade.load(cascade_path);

    nose_cascade.detectMultiScale(img, nose, 1.20, 5, 0|CASCADE_SCALE_IMAGE, Size(30, 30));
    return;
}

static void detectMouth(Mat& img, vector<Rect_<int> >& mouth, string cascade_path)
{
    CascadeClassifier mouth_cascade;
    mouth_cascade.load(cascade_path);

    mouth_cascade.detectMultiScale(img, mouth, 1.20, 5, 0|CASCADE_SCALE_IMAGE, Size(30, 30));
    return;
}
