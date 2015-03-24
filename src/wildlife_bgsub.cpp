//Logging
#include <glog/logging.h>

//C++
#include <cstdlib>
#include <fstream>

//OpenCV
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/features2d/features2d.hpp>

//My Libs
#include "video_type.hpp"
#include "vansub.hpp"
#include "hofsub.hpp"
#include "boinc_utils.hpp"

//Defines
//#define GUI

//BOINC
#ifdef _BOINC_APP_
#ifdef _WIN32
#include "boinc_win.h"
#include "str_util.h"
#endif

#include "boinc/diagnostics.h"
#include "boinc/util.h"
#include "boinc/filesys.h"
#include "boinc/boinc_api.h"
#include "boinc/mfile.h"
#endif

/** Staic Vars **/
static const double ALPHA = 0.1;
//static const std::string DOWNLOAD_PREFIX = "http://volunteer.cs.und.edu/csg/wildlife_kgoehner/video_interesting_events.php?video_id=";

/** Global Vars*/
std::vector<cv::Ptr<BSub>> subtractors;

std::vector<double> vibe_means;
std::vector<double> pbas_means;
//std::vector<double> mog_means;

double vibe_exp_mean = 0;
double pbas_exp_mean = 0;
//double mog_exp_mean = 0;


#ifndef _BOINC_APP_
std::ofstream tsv_file;
std::ofstream event_file;
std::ofstream vibe_file;
std::ofstream pbas_file;
//std::ofstream mog_file;
#endif

/** Function Headers */
void help();
void processVideo(const int video_id, cv::VideoCapture &capture);
void writeFramenumber(cv::Mat &frame, double frame_num);
int getVideoId(std::string path);
bool readConfig(std::string filename, std::string *species);
void writeCheckpoint(const int video_id, const int &frame_pos, const std::vector<cv::Ptr<BSub>> &subtractors) throw(std::runtime_error);
bool readCheckpoint(const int video_id, int &frame_pos, std::vector<cv::Ptr<BSub>> &subtractors);
int skipFrames(cv::VideoCapture &capture, int n);

// TODO Update the help info
void help() {
    LOG(INFO) << "--------------------------------------------------------------------------";
    LOG(INFO) << "This is the Wildlife@Home Background Subtraction progoram.";
    LOG(INFO) << "It runs multiple types of background subtraction to be used in the";
    LOG(INFO) << "detection of birds in their native habitats.";
    LOG(INFO) << "Usage:";
    LOG(INFO) << "./wildlife_bgsub <video filename>";
    LOG(INFO) << "for example: ./wildlife_bgsub video.ogv";
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

/*
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
*/

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

#ifdef _BOINC_APP_
    boinc_init();
#endif

#ifdef GUI
    //create GUI windows
    cv::namedWindow("Frame", CV_GUI_NORMAL);
    cv::namedWindow("VIBE Model", CV_GUI_NORMAL);
    cv::namedWindow("PBAS Model", CV_GUI_NORMAL);
    cv::namedWindow("FG Mask VIBE", CV_GUI_NORMAL);
    cv::namedWindow("FG Mask PBAS", CV_GUI_NORMAL);
    //cv::namedWindow("FG Mask MOG", CV_GUI_NORMAL);
#endif

    std::string video_filename(argv[1]);
    video_filename = getBoincFilename(video_filename);

    //create the capture object
    cv::VideoCapture capture(video_filename);

    //VideoCapture capture(0);
    if (!capture.isOpened()) {
        //error in opening the video input
        LOG(ERROR) << "Unable to open video file: " << video_filename;
        exit(EXIT_FAILURE);
    }

    int video_id = getVideoId(video_filename);
    double rows = capture.get(CV_CAP_PROP_FRAME_HEIGHT);
    double cols = capture.get(CV_CAP_PROP_FRAME_WIDTH);

    int frame_pos = 0;
    //Look for a local checkpoint and load it
#ifdef _BOINC_APP_
    if(readCheckpoint(video_id, frame_pos, subtractors)) {
        LOG(INFO) << "Continuing from checkpoint...";
        skipFrames(capture, frame_pos);
    } else {
        LOG(INFO) << "Unsuccessful checkpoint read, starting from beginning of video";
#endif
        cv::Ptr<BSub> pVIBE = new VANSub(rows, cols, 10, 256, 20); //ViBe Background subtractor
        cv::Ptr<BSub> pPBAS = new HOFSub(rows, cols, 10, 256, 20); //PBAS Background subtractor
        //cv::Ptr<cv::BackgroundSubtractor> pMOG = new cv::BackgroundSubtractorMOG(); //MOG Background subtractor
        subtractors.push_back(pVIBE);
        subtractors.push_back(pPBAS);
        //subtractors.push_back(pMOG);
#ifdef _BOINC_APP_
    }
#endif

    processVideo(video_id, capture);
    capture.release();

#ifndef _BOINC_APP_
    //Close files
    tsv_file.close();
    event_file.close();
    vibe_file.close();
    pbas_file.close();
    //mog_file.close();
#endif

#ifdef GUI
    //destroy GUI windows
    cv::destroyAllWindows();
#endif

#ifdef _BOINC_APP_
    boinc_finish(EXIT_SUCCESS);
#endif
    return EXIT_SUCCESS;
}

/**
 * @function processVideo
 */
void processVideo(const int video_id, cv::VideoCapture &capture) {
    cv::Mat frame;

    double rows = capture.get(CV_CAP_PROP_FRAME_HEIGHT);
    double cols = capture.get(CV_CAP_PROP_FRAME_WIDTH);
    double total_frames = capture.get(CV_CAP_PROP_FRAME_COUNT);
    //double fps = capture.get(CV_CAP_PROP_FPS);
    double num_pixels = rows * cols;

    VideoType type(cv::Size(cols, rows));

    std::string video_id_str = std::to_string(static_cast<long long>(video_id));
    //std::vector<size_t> *event_times = openEventFile(video_id, 10);
    //std::vector<double> vibe_window_vals;
    //std::vector<double> mog_window_vals;

#ifndef _BOINC_APP_
    //Open files for data output
    boost::filesystem::path dir(video_id_str);
    boost::filesystem::create_directory(dir);
    tsv_file.open(video_id_str + "/data.tsv");
    event_file.open(video_id_str + "/binary_event.dat");
    vibe_file.open(video_id_str + "/white_vibe_pixels.dat");
    pbas_file.open(video_id_str + "/white_pbas_pixels.dat");
    //mog_file.open(video_id_str + "/white_mog_pixels.dat");
#endif

    //read input data.
    while(capture.read(frame)) {
        double frame_pos = capture.get(CV_CAP_PROP_POS_FRAMES);

        // Mask
        cv::rectangle(frame, type.getTimestampRect(), cv::Scalar(0,0,0), CV_FILLED);
        cv::rectangle(frame, type.getWatermarkRect(), cv::Scalar(0,0,0), CV_FILLED);

        std::vector<cv::Mat*> masks;
        for (int i = 0; i < subtractors.size(); i++) {
            masks.push_back(new cv::Mat(frame));
            subtractors.at(i)->operator()(frame, *(masks.at(i)), 0.1);
        }

#ifdef GUI
        cv::Mat vibe_model, pbas_model;
        subtractors.at(0)->getBackgroundImage(vibe_model);
        subtractors.at(1)->getBackgroundImage(pbas_model);

        //show the current frame and the fg masks
        writeFramenumber(frame, frame_pos);

        imshow("Frame", frame);
        imshow("VIBE Model", vibe_model);
        imshow("PBAS Model", pbas_model);
        imshow("FG Mask VIBE", *(masks.at(0)));
        imshow("FG Mask PBAS", *(masks.at(1)));
        //imshow("FG Mask MOG", *(masks.at(2)));
        //get the input from the keyboard
        cv::waitKey(5);
#endif

        //Count white pixels
        std::vector<double> pixel_counts;
        for (int r = 0; r < frame.rows; r++) {
            for (int c = 0; c < frame.cols; c++) {
                for (int i = 0; i < masks.size(); i++) {
                    pixel_counts.push_back(0);
                    if(masks.at(i)->at<unsigned char>(r, c) > 0) {
                        pixel_counts.at(i)++;
                    }
                }
            }
        }

        for (int i = 0; i < masks.size(); i++) {
            delete masks.at(i);
        }
        masks.clear();

        // Compile results
        double next_vibe_val = pixel_counts.at(0)/num_pixels;
        double next_pbas_val = pixel_counts.at(1)/num_pixels;
        //double next_mog_val = pixel_counts.at(2)/num_pixels;

        vibe_exp_mean = ALPHA * next_vibe_val + (1-ALPHA) * vibe_exp_mean;
        pbas_exp_mean = ALPHA * next_pbas_val + (1-ALPHA) * pbas_exp_mean;
        //mog_exp_mean = ALPHA * next_mog_val + (1-ALPHA) * mog_exp_mean;

        vibe_means.push_back(vibe_exp_mean);
        pbas_means.push_back(pbas_exp_mean);
        //mog_means.push_back(mog_exp_mean);

#ifndef _BOINC_APP_
        //Print Stuff
        vibe_file << vibe_exp_mean << std::endl;
        pbas_file << pbas_exp_mean << std::endl;
        //mog_file << mog_exp_mean << std::endl;
        //tsv_file << video_id << "\t" << vibe_exp_mean << "\t" << pbas_exp_mean << "\t" << mog_exp_mean << std::endl;
        tsv_file << video_id << "\t" << vibe_exp_mean << "\t" << pbas_exp_mean << std::endl;
#endif

#ifdef _BOINC_APP_
		// Update percent completion and look for checkpointing request.
		boinc_fraction_done((double)frame_pos/total_frames);
		if(boinc_time_to_checkpoint()) {
			LOG(INFO) << "Checkpointing...";
			writeCheckpoint(video_id, frame_pos, subtractors);
			boinc_checkpoint_completed();
			LOG(INFO) << "Done checkpointing!";
		}
#endif
    }

#ifdef _BOINC_APP_
    std::cerr << "<results>";
    for (unsigned int i = 0; i < vibe_means.size(); i++) {
        //std::cerr << video_id << "\t" << vibe_means.at(i) << "\t" << pbas_means.at(i) << "\t" << mog_means.at(i) << std::endl;
        std::cerr << video_id << "\t" << vibe_means.at(i) << "\t" << pbas_means.at(i) << std::endl;
    }
    std::cerr << "</results>";
#endif

#ifndef _BOINC_APP_
    //Close files for data output
    tsv_file.close();
    event_file.close();
    vibe_file.close();
    pbas_file.close();
    //mog_file.close();
#endif
}

void writeFramenumber(cv::Mat &frame, double frame_num) {
    std::stringstream ss;
    cv::rectangle(frame, cv::Point(10, 2), cv::Point(100,20), cv::Scalar(255,255,255), -1);
    ss << frame_num;
    std::string frameNumberString = ss.str();
    cv::putText(frame, frameNumberString.c_str(), cv::Point(15, 15), cv::FONT_HERSHEY_SIMPLEX, 0.5 , cv::Scalar(0,0,0));
}

void writeCheckpoint(const int video_id, const int &frame_pos, const std::vector<cv::Ptr<BSub>> &subtractors) throw(std::runtime_error) {
    std::string checkpoint_filename = getBoincFilename(std::to_string(static_cast<long long>(video_id)) + ".yml");
    //writeEventsToFile(checkpoint_filename, event_types);
    cv::FileStorage outfile(checkpoint_filename, cv::FileStorage::WRITE);
    if (!outfile.isOpened()) {
        throw std::runtime_error("Checkpoint file did not open");
    }
    LOG(INFO) << "WRITE_CURRENT_FRAME: " << frame_pos;
    outfile << "CURRENT_FRAME" << frame_pos;
    outfile << "VIBE_MEANS" << vibe_means;
    outfile << "PBAS_MEANS" << pbas_means;
    outfile << "VIBE_EXP_MEAN" << vibe_exp_mean;
    outfile << "PBAS_EXP_MEAN" << pbas_exp_mean;
    outfile << "VANSUB" << *subtractors.at(0);
    outfile << "HOFSUB" << *subtractors.at(1);
    outfile.release();
}

bool readCheckpoint(const int video_id, int &frame_pos, std::vector<cv::Ptr<BSub>> &subtractors) {
    LOG(INFO) << "Reading checkpoint...";
    std::string checkpoint_filename = getBoincFilename(std::to_string(static_cast<long long>(video_id)) + ".yml");
    cv::FileStorage infile(checkpoint_filename, cv::FileStorage::READ);
    if (!infile.isOpened()) {
        return false;
    }
    infile["CURRENT_FRAME"] >> frame_pos;
    LOG(INFO) << "READ_CURRENT_FRAME: " << frame_pos;

    infile["VIBE_MEANS"] >> vibe_means;
    infile["PBAS_MEANS"] >> pbas_means;
    infile["VIBE_EXP_MEAN"] >> vibe_exp_mean;
    LOG(INFO) << "VIBE_EXP_MEAN: " << vibe_exp_mean;
    infile["PBAS_EXP_MEAN"] >> pbas_exp_mean;
    LOG(INFO) << "PBAS_EXP_MEAN: " << pbas_exp_mean;

    VANSub van_sub;
    infile["VANSUB"] >> van_sub;
    LOG(INFO) << van_sub;
    subtractors.push_back(new VANSub(van_sub));

    HOFSub hof_sub;
    infile["HOFSUB"] >> hof_sub;
    LOG(INFO) << hof_sub;
    subtractors.push_back(new HOFSub(hof_sub));

    infile.release();

    LOG(INFO) << "Done reading checkpoint.";
    return true;
}

bool readConfig(std::string config_filename, std::string *species) {
    LOG(INFO) << "Reading config file: '" << config_filename << "'";
    std::string line, event_id, start_time, end_time;
    std::ifstream infile(config_filename.c_str());
    if (infile.is_open()) {
        //Get Species Name
        getline(infile, line);
        *species = line;

        infile.close();
        return true;
    }
    return false;
}

int skipFrames(cv::VideoCapture &capture, int n) {
    for(int i = 0; i < n; ++i) {
        if(!capture.grab()) {
        	return i+1;
        }
    }
	return n;
}
