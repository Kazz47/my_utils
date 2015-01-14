//opencv
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/video/background_segm.hpp>
#include <opencv2/imgproc/imgproc.hpp>
//C++
#include <cstdio>
#include <iostream>
#include <fstream>
#include <sstream>
//Logging
#include <glog/logging.h>
//My Libs
#include "bsub.hpp"

using namespace cv;
using namespace std;

// Global variables
Mat frame; //current frame
Mat fgMaskBSUB; //fg mask fg mask generated by my method
Mat fgMaskMOG; //fg mask fg mask generated by MOG method
Mat fgMaskMOG2; //fg mask fg mask generated by MOG2 method
SimpleBlobDetector::Params blob_params;
Ptr<SimpleBlobDetector> blob_detector;
Ptr<BackgroundSubtractor> bsub; //My Background subtractor
Ptr<BackgroundSubtractor> pMOG; //MOG Background subtractor
Ptr<BackgroundSubtractor> pMOG2; //MOG2 Background subtractor
int keyboard; //input from keyboard

// Output
ofstream bsub_file;
ofstream mog_file;
ofstream mog2_file;

/** Function Headers */
void help();
void processVideo(char* videoFilename);
void processImages(char* firstFrameFilename);

void help()
{
    cout
    << "--------------------------------------------------------------------------" << endl
    << "This program shows how to use background subtraction methods provided by "  << endl
    << " OpenCV. You can process both videos (-vid) and images (-img)."             << endl
                                                                                    << endl
    << "Usage:"                                                                     << endl
    << "./bs {-vid <video filename>|-img <image filename>}"                         << endl
    << "for example: ./bs -vid video.avi"                                           << endl
    << "or: ./bs -img /data/images/1.png"                                           << endl
    << "--------------------------------------------------------------------------" << endl
    << endl;
}

/**
 * @function main
 */
int main(int argc, char* argv[])
{
    FLAGS_logtostderr = 1;
    google::InitGoogleLogging(argv[0]);
    //print help information
    help();

    //check for the input parameter correctness
    if(argc != 3) {
        cerr <<"Incorret input list" << endl;
        cerr <<"exiting..." << endl;
        return EXIT_FAILURE;
    }

    //Set Blob detection params
    blob_params.minDistBetweenBlobs = 10.0f;
    blob_params.filterByInertia = false;
    blob_params.filterByConvexity = false;
    blob_params.filterByColor = false;
    blob_params.filterByCircularity = false;
    blob_params.filterByArea = true;
    blob_params.minArea = 100.0f;
    blob_params.maxArea = 2000.0f;

    //create GUI windows
    namedWindow("Frame");
    namedWindow("FG Mask BSUB");
    namedWindow("FG Mask MOG");
    namedWindow("FG Mask MOG 2");

    //create Blob Detector
    blob_detector = new SimpleBlobDetector(blob_params);

    //create Background Subtractor objects
    bsub = new BSub(); // my approach
    pMOG = new BackgroundSubtractorMOG(); //MOG approach
    pMOG2 = new BackgroundSubtractorMOG2(); //MOG2 approach

    //Open files
    bsub_file.open("bsub_file.dat");
    mog_file.open("mog_file.dat");
    mog2_file.open("mog2_file.dat");

    if(strcmp(argv[1], "-vid") == 0) {
        //input data coming from a video
        processVideo(argv[2]);
    }
    else if(strcmp(argv[1], "-img") == 0) {
        //input data coming from a sequence of images
        processImages(argv[2]);
    }
    else {
        //error in reading input parameters
        cerr <<"Please, check the input parameters." << endl;
        cerr <<"Exiting..." << endl;
        return EXIT_FAILURE;
    }

    //Close files
    bsub_file.close();
    mog_file.close();
    mog2_file.close();

    //destroy GUI windows
    destroyAllWindows();
    return EXIT_SUCCESS;
}

/**
 * @function processVideo
 */
void processVideo(char* videoFilename) {
    //create the capture object
    VideoCapture capture(videoFilename);
    //VideoCapture capture(0);
    if(!capture.isOpened()){
        //error in opening the video input
        cerr << "Unable to open video file: " << videoFilename << endl;
        exit(EXIT_FAILURE);
    }
    //read input data. ESC or 'q' for quitting
    while( (char)keyboard != 'q' && (char)keyboard != 27 ){
        //read the current frame
        if(!capture.read(frame)) {
            cerr << "Unable to read next frame." << endl;
            cerr << "Exiting..." << endl;
            exit(EXIT_FAILURE);
        }
        //Blur frame
        //int t = 30;
        Mat src = frame.clone();
        //GaussianBlur(src, frame, Size(9, 9), 0, 0); //Large video
        GaussianBlur(src, frame, Size(5, 5), 0, 0); //Small Video
        //src = frame.clone();
        //bilateralFilter(src, frame, t, t*2, t/2);


        //update the background model
        bsub->operator()(frame, fgMaskBSUB, 0.001);
        pMOG->operator()(frame, fgMaskMOG, 0.001);
        pMOG2->operator()(frame, fgMaskMOG2, 0.001);

        //get the frame number and write it on the current frame
        stringstream ss;
        rectangle(frame, cv::Point(10, 2), cv::Point(100,20),
                  cv::Scalar(255,255,255), -1);
        ss << capture.get(CV_CAP_PROP_POS_FRAMES);
        string frameNumberString = ss.str();
        putText(frame, frameNumberString.c_str(), cv::Point(15, 15),
                FONT_HERSHEY_SIMPLEX, 0.5 , cv::Scalar(0,0,0));

        //Detect blobs in masks
        vector<KeyPoint> bsub_keypoints;
        vector<KeyPoint> mog_keypoints;
        vector<KeyPoint> mog2_keypoints;
        blob_detector->detect(fgMaskBSUB, bsub_keypoints);
        blob_detector->detect(fgMaskMOG, mog_keypoints);
        blob_detector->detect(fgMaskMOG2, mog2_keypoints);

        drawKeypoints(fgMaskBSUB, bsub_keypoints, fgMaskBSUB, Scalar::all(-1), DrawMatchesFlags::DEFAULT);
        drawKeypoints(fgMaskMOG, mog_keypoints, fgMaskMOG, Scalar::all(-1), DrawMatchesFlags::DEFAULT);
        drawKeypoints(fgMaskMOG2, mog2_keypoints, fgMaskMOG2, Scalar::all(-1), DrawMatchesFlags::DEFAULT);

        Mat model;
        bsub->getBackgroundImage(model);

        //show the current frame and the fg masks
        imshow("Frame", frame);
        imshow("Model", model);
        imshow("FG Mask BSUB", fgMaskBSUB);
        imshow("FG Mask MOG", fgMaskMOG);
        imshow("FG Mask MOG 2", fgMaskMOG2);
        //get the input from the keyboard
        keyboard = waitKey(30);

        //Count white pixels
        size_t bsub_white = 0;
        for (int r = 0; r < fgMaskBSUB.rows; r++) {
            for (int c = 0; c < fgMaskBSUB.cols; c++) {
                if(fgMaskBSUB.at<float>(r, c) > 0) {
                    bsub_white++;
                }
            }
        }

        size_t mog_white = 0;
        for (int r = 0; r < fgMaskMOG.rows; r++) {
            for (int c = 0; c < fgMaskMOG.cols; c++) {
                if(fgMaskMOG.at<float>(r, c) > 0) {
                    mog_white++;
                }
            }
        }

        size_t mog2_white = 0;
        for (int r = 0; r < fgMaskMOG2.rows; r++) {
            for (int c = 0; c < fgMaskMOG2.cols; c++) {
                if(fgMaskMOG2.at<float>(r, c) > 0) {
                    mog2_white++;
                }
            }
        }

        //Print Stuff
        //bsub_file << bsub_keypoints.size() << endl;
        bsub_file << bsub_white << endl;
        //mog_file << mog_keypoints.size() << endl;
        mog_file << mog_white << endl;
        //mog2_file << mog2_keypoints.size() << endl;
        mog2_file << mog2_white << endl;
    }
    //delete capture object
    capture.release();
}

/**
 * @function processImages
 */
void processImages(char* fistFrameFilename) {
    //read the first file of the sequence
    frame = imread(fistFrameFilename);
    if(frame.empty()){
        //error in opening the first image
        cerr << "Unable to open first image frame: " << fistFrameFilename << endl;
        exit(EXIT_FAILURE);
    }
    //current image filename
    string fn(fistFrameFilename);
    //read input data. ESC or 'q' for quitting
    while( (char)keyboard != 'q' && (char)keyboard != 27 ){
        //update the background model
        bsub->operator()(frame, fgMaskBSUB);
        pMOG->operator()(frame, fgMaskMOG);
        pMOG2->operator()(frame, fgMaskMOG2);
        //get the frame number and write it on the current frame
        size_t index = fn.find_last_of("/");
        if(index == string::npos) {
            index = fn.find_last_of("\\");
        }
        size_t index2 = fn.find_last_of(".");
        string prefix = fn.substr(0,index+1);
        string suffix = fn.substr(index2);
        string frameNumberString = fn.substr(index+1, index2-index-1);
        istringstream iss(frameNumberString);
        int frameNumber = 0;
        iss >> frameNumber;
        rectangle(frame, cv::Point(10, 2), cv::Point(100,20),
                  cv::Scalar(255,255,255), -1);
        putText(frame, frameNumberString.c_str(), cv::Point(15, 15),
                FONT_HERSHEY_SIMPLEX, 0.5 , cv::Scalar(0,0,0));
        //show the current frame and the fg masks
        imshow("Frame", frame);
        imshow("FG Mask BSUB", fgMaskBSUB);
        imshow("FG Mask MOG", fgMaskMOG);
        imshow("FG Mask MOG 2", fgMaskMOG2);
        //get the input from the keyboard
        keyboard = waitKey( 30 );
        //search for the next image in the sequence
        ostringstream oss;
        oss << (frameNumber + 1);
        string nextFrameNumberString = oss.str();
        string nextFrameFilename = prefix + nextFrameNumberString + suffix;
        //read the next frame
        frame = imread(nextFrameFilename);
        if(frame.empty()){
            //error in opening the next image in the sequence
            cerr << "Unable to open image frame: " << nextFrameFilename << endl;
            exit(EXIT_FAILURE);
        }
        //update the path of the current frame
        fn.assign(nextFrameFilename);
    }
}
