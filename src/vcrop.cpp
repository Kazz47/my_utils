#include "vcrop.hpp"

#include <glog/logging.h>

VCrop::VCrop(cv::VideoCapture &capture, const float &x, const float &y, const float &size) : capture(&capture) {
    if (!capture.isOpened()) {
        std::string error_message = "Error when reading input stream";
        LOG(ERROR) << error_message;
        throw error_message;
    }

    int frame_width = capture.get(CV_CAP_PROP_FRAME_WIDTH);
    int frame_height = capture.get(CV_CAP_PROP_FRAME_HEIGHT);
    VLOG(2) << "Frame Width: " << frame_width;
    VLOG(2) << "Frame Height: " << frame_height;
    LOG_IF(FATAL, frame_width <= 0) << "Frame width is less than zero.";
    LOG_IF(FATAL, frame_height <= 0) << "Frame height is less than zero.";
    float diameter = sqrt(frame_width * frame_width + frame_height * frame_height);
    cv::Point2i top_left(frame_width * x, frame_height * y);
    cv::Size2i rect_size(diameter * size, diameter * size);
    if (top_left.x + rect_size.width > frame_width || top_left.y + rect_size.height > frame_height) {
        LOG(ERROR) << "Size(" << rect_size << ") to too large for given x(" << top_left.x << ") and y(" << top_left.y << ") coordinate.";
    }
    roi = new cv::Rect(top_left, rect_size);
    VLOG(1) << "RoI: \t" << *roi;

    frame_rate = capture.get(CV_CAP_PROP_FPS);
    if (isnan(frame_rate) || frame_rate <= 0) {
        LOG(WARNING) << "Failed to get frame rate, setting rate to 10fps.";
        frame_rate = 10;
    }
    VLOG(1) << "Frame Rate: \t" << frame_rate;
}

VCrop::~VCrop() {
    delete roi;
}

cv::Mat VCrop::getNextFrame() {
    if (!capture->isOpened()) {
        std::string error_message = "Error when reading input stream";
        LOG(ERROR) << error_message;
        throw error_message;
    }
    cv::Mat frame;

    // Calculate next frame
    double frame_num = capture->get(CV_CAP_PROP_POS_FRAMES) - 1; //Last read frame
    double next_frame = frame_num + 5 * frame_rate;
    capture->set(CV_CAP_PROP_POS_FRAMES, next_frame);

    if (!capture->read(frame)) {
        frame.release();
        return frame;
    }
    return frame(*roi);
}
