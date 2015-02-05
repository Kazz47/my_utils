//Logging
#include <glog/logging.h>

//C++
#include <fstream>

//opencv
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/features2d/features2d.hpp>

//My Libs
#include "vansub.hpp"

/** Global Vars*/
cv::Mat fgMaskVIBE; //fg mask fg mask generated by VIBE
cv::Mat fgMaskMOG; //fg mask fg mask generated by MOG
cv::Ptr<cv::BackgroundSubtractor> pVIBE; //My Background subtractor
cv::Ptr<cv::BackgroundSubtractor> pMOG; //MOG Background subtractor
std::ofstream svm_file;
std::ofstream event_file;
std::ofstream variance_file;
std::ofstream vibe_file;
std::ofstream mog_file;

/** Function Headers */
void help();
void processVideo(std::string vid_filename, std::string event_filename);
void writeFramenumber(cv::Mat &frame, double frame_num);

// TODO Update the help info
void help() {
    LOG(INFO) << "--------------------------------------------------------------------------";
    LOG(INFO) << "This program shows how to use background subtraction methods provided by ";
    LOG(INFO) << " OpenCV. You can process both videos (-vid) and images (-img).";
    LOG(INFO) << "Usage:";
    LOG(INFO) << "./wildlife_bgsub <video filename> <event filename>";
    LOG(INFO) << "for example: ./wildlife_bgsub video.avi events.dat";
    LOG(INFO) << "--------------------------------------------------------------------------";
}

double calcMean(std::vector<double> vals) {
    double val = 0;
    for (double x : vals) {
        val += x;
    }
    return val/vals.size();
}

double calcVariance(std::vector<double> vals, const double mean) {
    double var = 0;
    for (double x : vals) {
        double diff = x - mean;
        var = diff * diff;
    }
    var = var/vals.size();
    return var;
}

std::vector<size_t>* openEventFile(std::string event_filename, const double fps) {
    LOG(INFO) << "Load event file: '" << event_filename << "'";

    std::ifstream infile(event_filename);

    std::vector<size_t>* event_times = new std::vector<size_t>();
    std::string event_id, time;

    std::string line;
    size_t temp;
    while (std::getline(infile, line)) {
        std::istringstream iss(line);
        if (!(iss >> event_id >> time)) {
            break;
        }
        temp = atoi(time.c_str())*10;
        event_times->push_back(temp);
    }
    infile.close();
    LOG(INFO) << "Loaded " << event_times->size() << " event times.";
    return event_times;
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
        LOG(ERROR) << "Incorret input list";
        return EXIT_FAILURE;
    }

    //create GUI windows
    cv::namedWindow("Frame", CV_GUI_NORMAL);
    cv::namedWindow("Model", CV_GUI_NORMAL);
    cv::namedWindow("FG Mask VIBE", CV_GUI_NORMAL);
    cv::namedWindow("FG Mask MOG", CV_GUI_NORMAL);

    //Open files
    svm_file.open("svm_data.dat");
    event_file.open("binary_event.dat");
    variance_file.open("variance.dat");
    vibe_file.open("white_vibe_pixels.dat");
    mog_file.open("white_mog_pixels.dat");

    std::string video_filename(argv[1]);
    std::string event_filename(argv[2]);

    processVideo(video_filename, event_filename);

    //Close files
    svm_file.close();
    event_file.close();
    vibe_file.close();
    mog_file.close();

    //destroy GUI windows
    cv::destroyAllWindows();
    return EXIT_SUCCESS;
}

/**
 * @function processVideo
 */
void processVideo(std::string vid_filename, std::string event_filename) {
    cv::Mat frame;

    //create the capture object
    cv::VideoCapture capture(vid_filename);

    //VideoCapture capture(0);
    if (!capture.isOpened()) {
        //error in opening the video input
        LOG(ERROR) << "Unable to open video file: " << vid_filename;
        exit(EXIT_FAILURE);
    }

    double rows = capture.get(CV_CAP_PROP_FRAME_HEIGHT);
    double cols = capture.get(CV_CAP_PROP_FRAME_WIDTH);
    double fps = capture.get(CV_CAP_PROP_FPS);
    double num_pixels = rows * cols;

    std::vector<size_t> *event_times = openEventFile(event_filename, fps);
    std::vector<double> vibe_window_vals;
    std::vector<double> mog_window_vals;

    std::vector<double> vibe_vals;
    std::vector<double> mog_vals;

    //create Background Subtractor objects
    //bsub = new BSub(); // my approach
    //bsub = new KOSub(rows, cols, 20, 2); // ko approach
    pVIBE = new VANSub(rows, cols, 10, 256, 20); // vibe approach
    pMOG = new cv::BackgroundSubtractorMOG(); //MOG approach

    //read input data.
    while(capture.read(frame)){
        bool event_occur = false;
        double frame_pos = capture.get(CV_CAP_PROP_POS_FRAMES);
        for (size_t i = 0; i < event_times->size(); i++) {
            VLOG(3) << frame_pos << " vs " << event_times->at(i);
            if (frame_pos == event_times->at(i)) {
                event_occur = true;
                break;
            }
        }

        //update the background model
        pVIBE->operator()(frame, fgMaskVIBE, 0.1);
        pMOG->operator()(frame, fgMaskMOG, 0.001);

        writeFramenumber(frame, frame_pos);

        cv::Mat model;
        pVIBE->getBackgroundImage(model);

        //show the current frame and the fg masks
        imshow("Frame", frame);
        imshow("Model", model);
        imshow("FG Mask VIBE", fgMaskVIBE);
        imshow("FG Mask MOG", fgMaskMOG);
        //get the input from the keyboard
        cv::waitKey(5);

        //Count white pixels
        double vibe_white = 0;
        for (int r = 0; r < fgMaskVIBE.rows; r++) {
            for (int c = 0; c < fgMaskVIBE.cols; c++) {
                if(fgMaskVIBE.at<float>(r, c) > 0) {
                    vibe_white++;
                }
            }
        }

        double mog_white = 0;
        for (int r = 0; r < fgMaskMOG.rows; r++) {
            for (int c = 0; c < fgMaskMOG.cols; c++) {
                if(fgMaskMOG.at<float>(r, c) > 0) {
                    mog_white++;
                }
            }
        }

        // Compile results
        vibe_vals.push_back(vibe_white/num_pixels);
        mog_vals.push_back(mog_white/num_pixels);

        int window_size = 50;
        vibe_window_vals.push_back(vibe_white/num_pixels);
        mog_window_vals.push_back(mog_white/num_pixels);
        if (vibe_window_vals.size() >  window_size) {
            vibe_window_vals.erase(vibe_window_vals.begin());
        }
        if (mog_window_vals.size() >  window_size) {
            mog_window_vals.erase(mog_window_vals.begin());
        }


        //Print Stuff
        svm_file << event_occur << " 1:" << vibe_white/num_pixels << " 2:" << mog_white/num_pixels << std::endl;
        if (event_occur) {
            event_file << frame_pos << "\t" << 0 << std::endl;
        }
        double vibe_mean = calcMean(vibe_window_vals);
        double mog_mean = calcMean(mog_window_vals);
        variance_file << calcVariance(vibe_window_vals, vibe_mean) << std::endl;
        // Use a mean filter here!
        vibe_file << vibe_mean << std::endl;
        mog_file << mog_mean << std::endl;
    }
    capture.release();

    // Process results
    double vibe_mean = calcMean(vibe_vals);
    double mog_mean = calcMean(mog_vals);
    double vibe_stdv = sqrt(calcVariance(vibe_vals, vibe_mean));
    double mog_stdv = sqrt(calcVariance(mog_vals, vibe_mean));

    double vibe_matches = 0;
    double mog_matches = 0;
    for (double time : *event_times) {
        if (vibe_vals.at(time) > vibe_mean) {
            vibe_matches++;
        }
        if (mog_vals.at(time) > mog_mean) {
            mog_matches++;
        }
    }
    LOG(INFO) << "ViBe accuracy: " << vibe_matches/vibe_vals.size();
    LOG(INFO) << "MOG accuracy: " << mog_matches/vibe_vals.size();
}

void writeFramenumber(cv::Mat &frame, double frame_num) {
    std::stringstream ss;
    cv::rectangle(frame, cv::Point(10, 2), cv::Point(100,20), cv::Scalar(255,255,255), -1);
    ss << frame_num;
    std::string frameNumberString = ss.str();
    cv::putText(frame, frameNumberString.c_str(), cv::Point(15, 15), cv::FONT_HERSHEY_SIMPLEX, 0.5 , cv::Scalar(0,0,0));
}
