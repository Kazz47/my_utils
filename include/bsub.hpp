#ifndef BSUB_H
#define BSUB_H

#include <opencv2/video/background_segm.hpp>

/**
 * Simple Background subtraction class.
 */
class BSub : public cv::BackgroundSubtractor {
public:
    BSub(const unsigned int &history = 10);
    BSub(const BSub &other);
    ~BSub();
    void operator()(cv::InputArray image, cv::OutputArray fgmask, double learning_rate = 0);
    void apply(cv::InputArray image, cv::OutputArray fgmask, double learning_rate = 0);
    void getBackgroundImage(cv::OutputArray background_image) const;
    std::ostream& print(std::ostream &out) const;
    void read(const cv::FileNode &node);
    void write(cv::FileStorage &fs) const;

protected:
    cv::Ptr<cv::Mat> model;

private:
    void updateModel(const cv::Mat &dist, const double &rate);
};

static void write(cv::FileStorage &fs, const std::string&, const BSub &x) {
    x.write(fs);
}

static void read(const cv::FileNode &node, BSub &x, const BSub &default_value = BSub()) {
    if (node.empty()) {
        x = default_value;
    } else {
        x.read(node);
    }
}

static std::ostream& operator<<(std::ostream &out, const BSub &x) {
    x.print(out);
    return out;
}

#endif //BSUB_H

