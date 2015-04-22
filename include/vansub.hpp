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
        const int rows = 0,
        const int cols = 0,
        const int radius = 20,
        const int colors = 256,
        const int history = 20);
    VANSub(const VANSub &other);
    ~VANSub();
    void operator()(cv::InputArray image, cv::OutputArray fgmask, double learning_rate);
    void apply(cv::InputArray image, cv::OutputArray fgmask, double learning_rate = 0);
    void getBackgroundImage(cv::OutputArray background_image) const;
    std::ostream& print(std::ostream &out) const;
    void read(const cv::FileNode &node);
    void write(cv::FileStorage &fs) const;

private:
    static const int req_matches = 2;
    static const int max_colors = 256;

    int rows;
    int cols;
    int radius;
    int colors;
    int history;

    float color_reduction;
    float color_expansion;
    bool initiated;

    cv::Ptr<cv::Mat> model;
    cv::Ptr<cv::Mat> diff;
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

static void write(cv::FileStorage &fs, const std::string&, const VANSub &x) {
    x.write(fs);
}

static void read(const cv::FileNode &node, VANSub &x, const VANSub &default_value = VANSub()) {
    if (node.empty()) {
        x = default_value;
    } else {
        x.read(node);
    }
}

static std::ostream& operator<<(std::ostream &out, const VANSub &x) {
    x.print(out);
    return out;
}

#endif //VANSUB_H

