#ifndef KOSUB_H
#define KOSUB_H

#include "bsub.hpp"

/**
 * Ko_2008_Background background subtraction class.
 */
class KOSub : public BSub {
public:
    KOSub(const unsigned int radius, const unsigned int &history = 10);
    void operator()(cv::InputArray image, cv::OutputArray fgmask, double learning_rate);
    void apply(cv::InputArray image, cv::OutputArray fgmask, double learning_rate = 0);

private:
    unsigned int radius;

    void updateModel(const cv::Mat &new_model, const double &rate);
    float densityNeighborhood(const cv::Mat &image, const int row, const int col, const int radius);
    unsigned char diracDelta(const unsigned char a, const unsigned char b) {
        return a == b;
    }
};

#endif //KOSUB_H

