#ifndef BSUB_H
#define BSUB_H

#include <opencv2/video/background_segm.hpp>

/**
 * Simple Background subtraction class.
 */
class BSub : public cv::BackgroundSubtractor {
public:
    BSub(const unsigned int &history = 10);
    ~BSub();
    void operator()(cv::InputArray image, cv::OutputArray fgmask, double learning_rate = 0);
    void apply(cv::InputArray image, cv::OutputArray fgmask, double learning_rate = 0);
    void getBackgroundImage(cv::OutputArray background_image) const;

protected:
    cv::Mat *model;

private:
    void updateModel(const cv::Mat &dist, const double &rate);
};

#endif //BSUB_H

