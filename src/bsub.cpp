#include "bsub.hpp"

#include <glog/logging.h>

#include <opencv2/imgproc/imgproc.hpp>

BSub::BSub(const unsigned int &history) {
    model = new cv::Mat();
    VLOG(3) << "Created!";
}

BSub::BSub(const BSub &other) {
    this->model = other.model;
    VLOG(3) << "Created!";
}

BSub::~BSub() {
    VLOG(3) << "Deleted!";
}

void BSub::operator()(cv::InputArray image, cv::OutputArray fgmask, double learning_rate) {
    this->apply(image, fgmask, learning_rate);
}

void BSub::apply(cv::InputArray image, cv::OutputArray fgmask, double learning_rate) {
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
        cv::threshold(diff, fgmask, 150, 255, cv::THRESH_BINARY);
    }

    // Update Model
    updateModel(diff, learning_rate);
}

void BSub::getBackgroundImage(cv::OutputArray background_image) const {
    if (!background_image.needed() || model->empty()) {
        VLOG(1) << "Background was empty";
        return;
    }

    background_image.create(model->size(), model->type());
    cv::Mat output = background_image.getMat();
    model->copyTo(output);
    VLOG(1) << "Got background image";
}

void BSub::updateModel(const cv::Mat &diff, const double &rate) {
    LOG_IF(ERROR, rate < 0 || rate > 1) << "Invalid rate.";
    LOG_IF(ERROR, model->empty() == true) << "Apptempting to update an empty model.";
    cv::Mat prev_model(*model);
    for (int r = 0; r < model->rows; r++) {
        for (int c = 0; c < model->cols; c++) {
            unsigned char prev_val = prev_model.at<unsigned char>(r, c);
            unsigned char distance = diff.at<unsigned char>(r, c);
            model->at<unsigned char>(r, c) = ((1-rate) * prev_val) + (rate * distance);
        }
    }
}

void BSub::read(const cv::FileNode &node) {
    //Load New Matrices
    cv::Mat temp;
    node["MODEL"] >> temp;
    this->model = new cv::Mat(temp);
}

void BSub::write(cv::FileStorage &fs) const {
    fs << "{";
    fs << "MODEL" << *(this->model);
    fs << "}";
}

std::ostream& BSub::print(std::ostream &out) const {
    out << "{ ";
    out << "model = " << this->model->size();
    out << " }";
    return out;
}

