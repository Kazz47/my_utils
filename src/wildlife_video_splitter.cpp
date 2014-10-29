#include <fstream>
#include <sstream>

#include <glog/logging.h>
#include <opencv2/highgui/highgui.hpp>

#include "vcrop.cpp"

#define VISUAL

// Name of the main program window
static const std::string W_NAME = "WINDOW";

// Frequency in seconds to write frame to disk
size_t WRITE_FREQUENCY = 5;

std::string getUsage() {
    std::stringstream ss;
    ss << "Usage: <video> <time_file> <x position> <y position> <scale size>";
    return ss.str();
}

std::string getDesc() {
    std::stringstream ss;
    ss << "Description: All inputs should be normalized: [0, 1]";
    return ss.str();
}

int main(int argc, char** argv) {
    // Log to stderr instead of to the tmp directory
    FLAGS_logtostderr = 1;
    // Initiate Google Logging
    google::InitGoogleLogging(argv[0]);

    if (argc < 6) {
        LOG(ERROR) << "Not enough arguments:";
        LOG(ERROR) << getUsage();
        LOG(ERROR) << getDesc();
        exit(1);
    }

    // Read input values
    std::string vid_filename = argv[1];
    std::string time_filename = argv[2];
    float x_val = atof(argv[3]);
    float y_val = atof(argv[4]);
    float size_val = atof(argv[5]);

    LOG(INFO) << "Load time file: '" << time_filename << "'";

    std::ifstream infile(time_filename);

    std::vector<std::pair<size_t, size_t>*> positive_intervals;
    std::string event_id, start_time, end_time;

    std::string line;
    std::pair<size_t, size_t> *temp;
    while (std::getline(infile, line)) {
        std::istringstream iss(line);
        if (!(iss >> event_id >> start_time >> end_time)) {
            break;
        }
        //TODO Change this to FPS instead of 10.
        temp = new std::pair<size_t, size_t>(atoi(start_time.c_str())*10, atoi(end_time.c_str())*10);
        positive_intervals.push_back(temp);
    }
    infile.close();
    LOG(INFO) << "Loaded " << positive_intervals.size() << " time intervals.";

    LOG(INFO) << "Cropping video: '" << vid_filename << "'";
    cv::VideoCapture capture(vid_filename);
    cv::Mat frame;

    VCrop cropper(capture, x_val, y_val, size_val);

    while (true) {
        frame = cropper.getNextFrame();
        if (frame.empty()) {
            capture.release();
            LOG(INFO) << "Done processing video...";
            break;
        }

#ifdef VISUAL
        //cv::rectangle(frame, roi, cv::Scalar(0, 0, 255));
        cv::imshow(W_NAME, frame);
        //cv::waitKey(static_cast<int64_t>(1000 * 1/frame_rate)); // Draw at correct frame rate.
        cv::waitKey(1); // Draw as fast as possible.
#endif

        std::string directory = "negative";
        double frame_pos = capture.get(CV_CAP_PROP_POS_FRAMES);
        for (size_t i = 0; i < positive_intervals.size(); i++) {
            VLOG(3) << frame_pos << " vs " << positive_intervals[i]->first << " & " << positive_intervals[i]->second;
            if (frame_pos >= positive_intervals[i]->first && frame_pos <= positive_intervals[i]->second) {
                directory = "positive";
                break;
            }
        }

        std::stringstream ss;
        ss << "./" << directory << "/" << frame_pos << ".jpg";
        VLOG(2) << "Write to '" << ss.str() << "'";
        cv::imwrite(ss.str(), frame);
    }

#ifdef VISUAL
    cv::destroyWindow(W_NAME);
#endif
    return 0;
}
