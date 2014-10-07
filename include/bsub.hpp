#ifndef BG_SUB_H
#define BG_SUB_H

#include <opencv2/core/core.hpp>
//#include <opencv2/highgui/highgui.hpp>
#include <opencv2/video/background_segm.hpp>

/**
 * Background subtraction class.
 */
class BSub : public cv::BackgroundSubtractor {
public:
    BSub();
    BSub(int history);
    ~BSub();
    void apply(cv::InputArray image, cv::OutputArray fgmask, double learningRate = 0);
    void getBackgroundImage(cv::OutputArray backgroundImage) const;
};

#endif //BG_SUB_H

