#ifndef VCROP_H
#define VCROP_H

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

/**
 * VCrop class.
 */
class VCrop {
public:
    VCrop(cv::VideoCapture &capture, const float &x, const float &y, const float &size);
    ~VCrop();
    cv::Mat getNextFrame();
    size_t getFrameNum();

private:
    cv::VideoCapture *capture;
    cv::Rect *roi;
    double frame_rate;
};

#endif //VCROP_H

