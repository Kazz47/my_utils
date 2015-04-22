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
        const int rows = 0,
        const int cols = 0,
        const int threshold = 20,
        const int colors = 256,
        const int history = 20);
    HOFSub(const HOFSub &other);
    ~HOFSub();
    void operator()(cv::InputArray image, cv::OutputArray fgmask, double learning_rate);
    void apply(cv::InputArray image, cv::OutputArray fgmask, double learning_rate = 0);
    void getBackgroundImage(cv::OutputArray background_image) const;
    std::ostream& print(std::ostream &out) const;
    void read(const cv::FileNode &node);
    void write(cv::FileStorage &fs) const;

private:
    static const int REQ_MATCHES = 2;
    static const int MAX_COLORS= 256;
    // Foreground detection threshold
    static const float THRESH_SCALE;
    static const float THRESH_MIN;
    static const float THRESH_MAX;
    // Recipricol used to as pixel update probability
    static const float UPDATE_MIN;
    static const float UPDATE_MAX;
    static const float THRESH_INC_RATE;
    static const float THRESH_DEC_RATE;
    static const float UPDATE_INC_RATE;
    static const float UPDATE_DEC_RATE;

    int rows;
    int cols;
    int colors;
    int history;
    float color_reduction;
    float color_expansion;
    bool initiated;

    cv::Ptr<cv::Mat> model;
    cv::Ptr<cv::Mat> decision_distance;
    cv::Ptr<cv::Mat> threshold;
    cv::Ptr<cv::Mat> update_val;
    cv::Ptr<cv::Mat> background_image;

    double seed;
    double num_generated;
    std::mt19937 *gen;
    boost::random::uniform_real_distribution<float> *update;
    boost::random::uniform_int_distribution<int> *history_update;
    boost::random::uniform_int_distribution<int> *pick_neighbor;

    void initiateModel(cv::Mat &image, cv::Rect &random_init);
    void updateModel(const int &r, const int &c, const unsigned char &val, const bool &update_neighbor = true);
};

static void write(cv::FileStorage &fs, const std::string&, const HOFSub &x) {
    x.write(fs);
}

static void read(const cv::FileNode &node, HOFSub &x, const HOFSub &default_value = HOFSub()) {
    if (node.empty()) {
        x = default_value;
    } else {
        x.read(node);
    }
}

static std::ostream& operator<<(std::ostream &out, const HOFSub &x) {
    x.print(out);
    return out;
}

#endif //HOFSUB_H

