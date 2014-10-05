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
    ss << "Usage: <video> <x position> <y position> <scale size>";
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

    if (argc < 5) {
        LOG(ERROR) << "Not enough arguments:";
        LOG(ERROR) << getUsage();
        LOG(ERROR) << getDesc();
        exit(1);
    }

    // Read input values
    std::string filename = argv[1];
    float x_val = atof(argv[2]);
    float y_val = atof(argv[3]);
    float size_val = atof(argv[4]);

    LOG(INFO) << "Opening video: '" << filename << "'";
    cv::VideoCapture capture(filename);
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

        std::stringstream ss;
        ss << "./" << capture.get(CV_CAP_PROP_POS_FRAMES) << ".jpg";
        cv::imwrite(ss.str(), frame);
    }

#ifdef VISUAL
    cv::destroyWindow(W_NAME);
#endif
    return 0;
}
