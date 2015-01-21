#ifndef VANSUB_H
#define VANSUB_H

#include <random>
#include "bsub.hpp"

/**
 * van_2014_vibe background subtraction class.
 */
class VANSub : public BSub {
public:
    VANSub(
        const int rows,
        const int cols,
        const unsigned int history = 20);
    ~VANSub();
    void operator()(cv::InputArray image, cv::OutputArray fgmask, double learning_rate);
    void apply(cv::InputArray image, cv::OutputArray fgmask, double learning_rate = 0);
    void getBackgroundImage(cv::OutputArray background_image) const;

private:
    const unsigned int req_matches = 2;

    int rows;
    int cols;
    unsigned int history;
    bool initiated = false;

    cv::Mat *model;
    cv::Mat *diff;
    cv::Mat *background_image;

    std::mt19937 *gen;
    std::uniform_int_distribution<int> *history_update;
    std::uniform_int_distribution<int> *absorb_foreground;
};

#endif //VANSUB_H

