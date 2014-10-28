#include "bsub.hpp"

#include <glog/logging.h>

#include <opencv2/imgproc/imgproc.hpp>

BSub::BSub(const unsigned int &history) {
    model = new cv::Mat();
    VLOG(1) << "Created!";
}

BSub::~BSub() {
    delete model;
    VLOG(1) << "Deleted!";
}

void BSub::operator()(cv::InputArray image, cv::OutputArray fgmask, double learningRate) {
    this->apply(image, fgmask, learningRate);
}

void BSub::apply(cv::InputArray image, cv::OutputArray fgmask, double learningRate) {
    cv::Mat input_image = image.getMat();
    if (input_image.channels() == 3) {
        cv::cvtColor(input_image, input_image, CV_BGR2GRAY);
    }

    if (model->empty()) {
        LOG(INFO) << "Background Model is empty, setting it to a copy of the foreground.";
        input_image.copyTo(*model);
    }
    cv::Mat diff;
    cv::absdiff(input_image, *model, diff);

    if (fgmask.needed()) {
        fgmask.create(input_image.size(), input_image.type());
        cv::Mat mask = fgmask.getMat();
        cv::threshold(diff, fgmask, 0, 255, cv::THRESH_BINARY);
    }

    // Update Model
    static const double rate = LEARNING_RATE;
    updateModel(diff, rate);
}

void BSub::getBackgroundImage(cv::OutputArray backgroundImage) const {
    if (!backgroundImage.needed() || model->empty()) {
        VLOG(1) << "Background was empty";
        return;
    }

    backgroundImage.create(model->size(), model->type());
    cv::Mat output = backgroundImage.getMat();
    model->copyTo(output);
    VLOG(1) << "Got background image";
}

void BSub::updateModel(const cv::Mat &dist, const double &rate) {
    LOG_IF(ERROR, model->empty() == true) << "Apptempting to update an empty model.";
    cv::Mat prev_model(*model);
    for (size_t r = 0; r < model->rows; r++) {
        for (size_t c = 0; c < model->cols; c++) {
            unsigned int prev_val = prev_model.at<unsigned int>(r, c);
            unsigned int distance = dist.at<unsigned int>(r, c);
            model->at<unsigned int>(r, c) = ((1-rate) * prev_val) + rate * distance;
        }
    }
}

