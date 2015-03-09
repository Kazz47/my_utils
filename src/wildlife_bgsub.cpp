//Logging
#include <glog/logging.h>

//C++
#include <cstdlib>
#include <fstream>

//Boost
#include <boost/filesystem.hpp>

//OpenCV
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/features2d/features2d.hpp>

//My Libs
#include "video_type.hpp"
#include "vansub.hpp"
#include "hofsub.hpp"

//Defines
//#define GUI

/** Staic Vars **/
static const double ALPHA = 0.1;
static const std::string DOWNLOAD_PREFIX = "http://volunteer.cs.und.edu/csg/wildlife_kgoehner/video_interesting_events.php?video_id=";

/** Global Vars*/
cv::Ptr<cv::BackgroundSubtractor> pVIBE; //ViBe Background subtractor
cv::Ptr<cv::BackgroundSubtractor> pPBAS; //PBAS Background subtractor
cv::Ptr<cv::BackgroundSubtractor> pMOG; //MOG Background subtractor
std::ofstream tsv_file;
std::ofstream event_file;
std::ofstream vibe_file;
std::ofstream pbas_file;
std::ofstream mog_file;

/** Function Headers */
void help();
void processVideo(std::string vid_filename);
void writeFramenumber(cv::Mat &frame, double frame_num);
int getVideoId(std::string path);

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

int getVideoId(std::string path) {
    int firstIndex = path.find_last_of("/\\");
    int lastIndex = path.find_last_of(".");
    return atoi(path.substr(firstIndex+1, lastIndex).c_str());
}

double calcMean(std::vector<double> vals) {
    double val = 0;
    for (size_t i = 0; i < vals.size(); i++) {
        val += vals[i];
    }
    return val/vals.size();
}

double calcVariance(std::vector<double> vals, const double mean) {
    double var = 0;
    for (size_t i = 0; i < vals.size(); i++) {
        double diff = vals[i] - mean;
        var = diff * diff;
    }
    var = var/vals.size();
    return var;
}

std::vector<size_t>* openEventFile(const int video_id, const double fps) {
    std::string video_id_str(std::to_string(static_cast<long long>(video_id)));
    std::string command_curl = "curl " + DOWNLOAD_PREFIX +  video_id_str + " -s -o " + video_id_str + ".dat";

    system(command_curl.c_str());
    std::string event_filename(video_id_str + ".dat");

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
        temp = atoi(time.c_str())*fps;
        event_times->push_back(temp);
    }

    infile.close();
    std::string command_rm = "rm " + video_id_str + ".dat";
    system(command_rm.c_str());

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
    if(argc != 2) {
        LOG(ERROR) << "Incorret input list";
        return EXIT_FAILURE;
    }

#ifdef GUI
    //create GUI windows
    cv::namedWindow("Frame", CV_GUI_NORMAL);
    cv::namedWindow("VIBE Model", CV_GUI_NORMAL);
    cv::namedWindow("PBAS Model", CV_GUI_NORMAL);
    cv::namedWindow("FG Mask VIBE", CV_GUI_NORMAL);
    cv::namedWindow("FG Mask PBAS", CV_GUI_NORMAL);
    cv::namedWindow("FG Mask MOG", CV_GUI_NORMAL);
#endif

    //Create Output Directory

    std::string video_filename(argv[1]);

    processVideo(video_filename);

    //Close files
    tsv_file.close();
    event_file.close();
    vibe_file.close();
    pbas_file.close();
    mog_file.close();

#ifdef GUI
    //destroy GUI windows
    cv::destroyAllWindows();
#endif

    return EXIT_SUCCESS;
}

/**
 * @function processVideo
 */
void processVideo(std::string vid_filename) {
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
    //double fps = capture.get(CV_CAP_PROP_FPS);
    double num_pixels = rows * cols;

    VideoType type(cv::Size(cols, rows));

    int video_id = getVideoId(vid_filename);
    std::string video_id_str = std::to_string(static_cast<long long>(video_id));
    std::vector<size_t> *event_times = openEventFile(video_id, 10);
    //std::vector<double> vibe_window_vals;
    //std::vector<double> mog_window_vals;
    double vibe_exp_mean = 0;
    double pbas_exp_mean = 0;
    double mog_exp_mean = 0;

    std::vector<double> vibe_vals;
    std::vector<double> pbas_vals;
    std::vector<double> mog_vals;

    //create Background Subtractor objects
    //bsub = new BSub(); // my approach
    //bsub = new KOSub(rows, cols, 20, 2); // ko approach
    pVIBE = new VANSub(rows, cols, 10, 256, 20); // vibe approach
    pPBAS = new HOFSub(rows, cols, 10, 256, 20); // PBAS approach
    pMOG = new cv::BackgroundSubtractorMOG(); //MOG approach

    //Open files for data output
    boost::filesystem::path dir(video_id_str);
    boost::filesystem::create_directory(dir);
    tsv_file.open(video_id_str + "/data.tsv");
    event_file.open(video_id_str + "/binary_event.dat");
    vibe_file.open(video_id_str + "/white_vibe_pixels.dat");
    pbas_file.open(video_id_str + "/white_pbas_pixels.dat");
    mog_file.open(video_id_str + "/white_mog_pixels.dat");

    //read input data.
    while(capture.read(frame)) {
        bool event_occur = false;
        double frame_pos = capture.get(CV_CAP_PROP_POS_FRAMES);
        for (size_t i = 0; i < event_times->size(); i++) {
            VLOG(3) << frame_pos << " vs " << event_times->at(i);
            if (frame_pos == event_times->at(i)) {
                event_occur = true;
                break;
            }
        }

        // Mask
        cv::rectangle(frame, type.getTimestampRect(), cv::Scalar(0,0,0), CV_FILLED);
        cv::rectangle(frame, type.getWatermarkRect(), cv::Scalar(0,0,0), CV_FILLED);

        cv::Mat fgMaskVIBE(frame); //fg mask fg mask generated by VIBE
        cv::Mat fgMaskPBAS(frame); //fg mask fg mask generated by VIBE
        cv::Mat fgMaskMOG(frame); //fg mask fg mask generated by MOG

        //update the background model
        pVIBE->operator()(frame, fgMaskVIBE, 0.1);
        pPBAS->operator()(frame, fgMaskPBAS, 0.1);
        pMOG->operator()(frame, fgMaskMOG, 0.001);

#ifdef GUI
        cv::Mat vibe_model, pbas_model;
        pVIBE->getBackgroundImage(vibe_model);
        pPBAS->getBackgroundImage(pbas_model);

        //show the current frame and the fg masks
        writeFramenumber(frame, frame_pos);

        imshow("Frame", frame);
        imshow("VIBE Model", vibe_model);
        imshow("PBAS Model", pbas_model);
        imshow("FG Mask VIBE", fgMaskVIBE);
        imshow("FG Mask PBAS", fgMaskPBAS);
        imshow("FG Mask MOG", fgMaskMOG);
        //get the input from the keyboard
        cv::waitKey(5);
#endif

        //Count white pixels
        double vibe_white = 0;
        double pbas_white = 0;
        double mog_white = 0;
        for (int r = 0; r < frame.rows; r++) {
            for (int c = 0; c < frame.cols; c++) {
                if(fgMaskVIBE.at<unsigned char>(r, c) > 0) {
                    vibe_white++;
                }
                if(fgMaskPBAS.at<unsigned char>(r, c) > 0) {
                    pbas_white++;
                }
                if(fgMaskMOG.at<unsigned char>(r, c) > 0) {
                    mog_white++;
                }
            }
        }

        // Compile results
        double next_vibe_val = vibe_white/num_pixels;
        double next_pbas_val = pbas_white/num_pixels;
        double next_mog_val = mog_white/num_pixels;
        vibe_vals.push_back(next_vibe_val);
        pbas_vals.push_back(next_pbas_val);
        mog_vals.push_back(next_mog_val);

        vibe_exp_mean = ALPHA * next_vibe_val + (1-ALPHA) * vibe_exp_mean;
        pbas_exp_mean = ALPHA * next_pbas_val + (1-ALPHA) * pbas_exp_mean;
        mog_exp_mean = ALPHA * next_mog_val + (1-ALPHA) * mog_exp_mean;

        //Print Stuff
        if (event_occur) {
            event_file << frame_pos << "\t" << 0 << std::endl;
        }
        vibe_file << vibe_exp_mean << std::endl;
        pbas_file << pbas_exp_mean << std::endl;
        mog_file << mog_exp_mean << std::endl;
        tsv_file << video_id << "\t" << event_occur << "\t" << vibe_exp_mean << "\t" << pbas_exp_mean << "\t" << mog_exp_mean << std::endl;
    }
    capture.release();

    //Close files for data output
    tsv_file.close();
    event_file.close();
    vibe_file.close();
    pbas_file.close();
    mog_file.close();
}

void writeFramenumber(cv::Mat &frame, double frame_num) {
    std::stringstream ss;
    cv::rectangle(frame, cv::Point(10, 2), cv::Point(100,20), cv::Scalar(255,255,255), -1);
    ss << frame_num;
    std::string frameNumberString = ss.str();
    cv::putText(frame, frameNumberString.c_str(), cv::Point(15, 15), cv::FONT_HERSHEY_SIMPLEX, 0.5 , cv::Scalar(0,0,0));
}
