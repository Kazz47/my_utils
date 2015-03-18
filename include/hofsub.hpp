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
        const int threshold = 20,
        const int colors = 256,
        const int history = 20);
    ~HOFSub();
    void operator()(cv::InputArray image, cv::OutputArray fgmask, double learning_rate);
    void apply(cv::InputArray image, cv::OutputArray fgmask, double learning_rate = 0);
    void getBackgroundImage(cv::OutputArray background_image) const;

private:
    static constexpr int REQ_MATCHES = 2;
    static constexpr int MAX_COLORS= 256;
    // Foreground detection threshold
    static constexpr float THRESH_SCALE = 5;
    static constexpr float THRESH_MIN = 18;
    static constexpr float THRESH_MAX = 1000000;
    // Recipricol used to as pixel update probability
    static constexpr float UPDATE_MIN = 2;
    static constexpr float UPDATE_MAX = 200;
    static constexpr float THRESH_INC_RATE = 0.05;
    static constexpr float THRESH_DEC_RATE = 0.05;
    static constexpr float UPDATE_INC_RATE = 1.00;
    static constexpr float UPDATE_DEC_RATE = 0.05;

    int rows;
    int cols;
    int colors;
    int history;

    float color_reduction;
    float color_expansion;
    bool initiated;

    cv::Mat *model;
    cv::Mat *decision_distance;
    cv::Mat *threshold;
    cv::Mat *update_val;
    cv::Mat *mask;
    cv::Mat *background_image;

    std::mt19937 *gen;
    boost::random::uniform_real_distribution<float> *update;
    boost::random::uniform_int_distribution<int> *history_update;
    boost::random::uniform_int_distribution<int> *pick_neighbor;
    /*
    std::uniform_int_distribution<int> *history_update;
    std::uniform_int_distribution<int> *update_neighbor;
    std::uniform_int_distribution<int> *pick_neighbor;
    */

    void initiateModel(cv::Mat &image, cv::Rect &random_init);
    void updateModel(const int &r, const int &c, const unsigned char &val, const bool &update_neighbor = true);
};

#endif //HOFSUB_H

