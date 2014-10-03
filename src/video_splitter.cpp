#include <sstream>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <glog/logging.h>

// Name of the main program window
static const std::string W_NAME = "WINDOW";

// Frequency in seconds to write frame to disk
size_t WRITE_FREQUENCY = 5;

int main(int argc, char** argv) {
    // Initiate Google Logging
    google::InitGoogleLogging(argv[0]);
    // Log to stderr instead of to the tmp directory
    FLAGS_logtostderr = 1;

    std::string filename = argv[1];
    LOG(INFO) << "Opening video: '" << filename << "'";
    cv::VideoCapture capture(filename);
    cv::Mat frame;

    if (!capture.isOpened()) {
        //TODO Log error!
        throw "Error when reading stream";
    }

    //cv::namedWindow(W_NAME, cv::WINDOW_AUTOSIZE);
    size_t frame_rate = capture.get(CV_CAP_PROP_FPS);
    size_t frame_num = capture.get(CV_CAP_PROP_POS_FRAMES);
    int frame_width = capture.get(CV_CAP_PROP_FRAME_WIDTH);
    int frame_height = capture.get(CV_CAP_PROP_FRAME_HEIGHT);

    cv::Point2i top_left(frame_width * 0.45, frame_height * 0.45);
    cv::Point2i bottom_right(frame_width * 0.55, frame_height * 0.55);
    LOG(INFO) << "TL: " << top_left;
    LOG(INFO) << "BR: " << bottom_right;
    cv::Rect roi(top_left, bottom_right);
    LOG(INFO) << "RoI: " << roi;

    while (true) {
        frame_num += WRITE_FREQUENCY * frame_rate;
        capture.set(CV_CAP_PROP_POS_FRAMES, frame_num);
        capture >> frame;
        if (frame.empty()) {
            capture.release();
            LOG(INFO) << "Done processing video...";
            break;
        }

        //if (frame_num % (WRITE_FREQUENCY * frame_rate) == 0) {
        std::stringstream ss;
        ss << "./" << frame_num << ".jpg";
        cv::imwrite(ss.str(), frame(roi));
        //}

        //cv::imshow(W_NAME, frame);
        //cv::waitKey(static_cast<int64_t>(1000 * 1/frame_rate)); // Draw at correct frame rate.
        //cv::waitKey(1); // Draw as fast as possible.
    }

    cv::destroyWindow(W_NAME);
    LOG(INFO) << "::: EXIT :::";
    return 0;
}
