#include "vcrop.hpp"

#include <glog/logging.h>

VCrop::VCrop(cv::VideoCapture &capture, const float &x, const float &y, const float &size) {
    if (!capture.isOpened()) {
        std::string error_message = "Error when reading input stream";
        LOG(ERROR) << error_message;
        throw error_message;
    }

    int frame_width = capture.get(CV_CAP_PROP_FRAME_WIDTH);
    int frame_height = capture.get(CV_CAP_PROP_FRAME_HEIGHT);
    float diameter = sqrt(frame_width * frame_width + frame_height * frame_height);
    cv::Point2i top_left(frame_width * x, frame_height * y);
    cv::Size2i rect_size(diameter * size, diameter * size);
    roi = new cv::Rect(top_left, rect_size);
    VLOG(1) << "RoI: \t" << roi;

    frame_rate = capture.get(CV_CAP_PROP_FPS);
    VLOG(1) << "Frame Rate: \t" << frame_rate;
}

VCrop::~VCrop() {
    delete roi;
}

cv::Mat VCrop::getNextFrame() {
    cv::Mat frame;

    // Calculate next frame
    double frame_num = capture->get(CV_CAP_PROP_POS_FRAMES);
    double next_frame = frame_num + 5 * frame_rate;
    capture->set(CV_CAP_PROP_POS_FRAMES, next_frame);

    if (!capture->read(frame)) {
        frame.release();
    }
    return frame(*roi);
}

