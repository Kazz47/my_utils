#ifndef HOFSUB_H
#define HOFSUB_H

#include <vector>
#include <random>
#include "bsub.hpp"

#include "boost/random.hpp"

/**
 * van_2014_vibe background subtraction class.
 */
class HOFSub : public BSub {
public:
    HOFSub(
        const int rows,
        const int cols,
        const int radius = 20,
        const int colors = 256,
        const int history = 20);
    ~HOFSub();
    void operator()(cv::InputArray image, cv::OutputArray fgmask, double learning_rate);
    void apply(cv::InputArray image, cv::OutputArray fgmask, double learning_rate = 0);
    void getBackgroundImage(cv::OutputArray background_image) const;

private:
    const static int req_matches = 2;
    const static int max_colors = 256;
    // Foreground detection threshold
    const static int thresh_min = 18;
    const static int thresh_max = std::numeric_limits<int>::max();
    // Recipricol used to as pixel update probability
    const static int update_min = 2;
    const static int update_max = 200;
    const static double thresh_inc_rate = 0.05;
    const static double thresh_dec_rate = 0.05;
    const static double update_inc_rate = 1.00;
    const static double update_dec_rate = 0.05;

    int rows;
    int cols;
    int radius;
    int colors;
    int history;

    float color_reduction;
    float color_expansion;
    bool initiated;

    std::vector<cv::Rect> *masks;

    cv::Mat *model;
    cv::Mat *diff;
    cv::Mat *background_image;

    std::mt19937 *gen;
    boost::random::uniform_int_distribution<int> *history_update;
    boost::random::uniform_int_distribution<int> *update_neighbor;
    boost::random::uniform_int_distribution<int> *pick_neighbor;
    /*
    std::uniform_int_distribution<int> *history_update;
    std::uniform_int_distribution<int> *update_neighbor;
    std::uniform_int_distribution<int> *pick_neighbor;
    */

    void initiateModel(cv::Mat &image, cv::Rect &random_init);
};

#endif //HOFSUB_H

