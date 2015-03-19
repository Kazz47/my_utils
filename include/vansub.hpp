#ifndef VANSUB_H
#define VANSUB_H

#include <vector>
#include <random>
#include "bsub.hpp"

#include "boost/random.hpp"

/**
 * van_2014_vibe background subtraction class.
 */
class VANSub : public BSub {
public:
    VANSub(
        const int rows,
        const int cols,
        const int radius = 20,
        const int colors = 256,
        const int history = 20);
    ~VANSub();
    void operator()(cv::InputArray image, cv::OutputArray fgmask, double learning_rate);
    void apply(cv::InputArray image, cv::OutputArray fgmask, double learning_rate = 0);
    void getBackgroundImage(cv::OutputArray background_image) const;
    void read(const cv::FileNode &node);
    void write(cv::FileStorage &fs) const;

private:
    const static int req_matches = 2;
    const static int max_colors = 256;

    int rows;
    int cols;
    int radius;
    int colors;
    int history;

    float color_reduction;
    float color_expansion;
    bool initiated;

    cv::Mat *model;
    cv::Mat *diff;
    cv::Mat *background_image;

    double num_generated;
    std::mt19937 *gen;
    boost::random::uniform_real_distribution<float> *update;
    boost::random::uniform_int_distribution<int> *history_update;
    boost::random::uniform_int_distribution<int> *pick_neighbor;
    /*
    std::uniform_int_distribution<int> *history_update;
    std::uniform_int_distribution<int> *update_neighbor;
    std::uniform_int_distribution<int> *pick_neighbor;
    */

    std::mt19937 getRNG();
    void initiateModel(cv::Mat &image, cv::Rect &random_init);
    void updateModel(const int &r, const int &c, const unsigned char &val, const bool &update_neighbor = true);
};

#endif //VANSUB_H

