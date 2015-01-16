#ifndef KOSUB_H
#define KOSUB_H

#include "bsub.hpp"

/**
 * Ko_2008_Background background subtraction class.
 */
class KOSub : public BSub {
public:
    KOSub(
        const unsigned int rows,
        const unsigned int cols,
        const unsigned int radius,
        const unsigned int &history = 10);
    ~KOSub();
    void operator()(cv::InputArray image, cv::OutputArray fgmask, double learning_rate);
    void apply(cv::InputArray image, cv::OutputArray fgmask, double learning_rate = 0);
    void getBackgroundImage(cv::OutputArray background_image) const;

private:
    int rows;
    int cols;
    unsigned int radius;

    cv::Mat *model;
    cv::Mat *diff;
    cv::Mat *background_image;

    float densityNeighborhood(const cv::Mat &image, const int row, const int col, const int color, const int radius);
};

#endif //KOSUB_H

