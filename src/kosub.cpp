#include "kosub.hpp"

#include <glog/logging.h>

#include <algorithm>
#include <opencv2/imgproc/imgproc.hpp>

KOSub::KOSub(const unsigned int radius, const unsigned int &history) : BSub(history) {
    this->radius = radius;
}

//Only supports 8-bit images
void KOSub::operator()(cv::InputArray image, cv::OutputArray fgmask, double learning_rate) {
    this->apply(image, fgmask, learning_rate);
}

void KOSub::apply(cv::InputArray image, cv::OutputArray fgmask, double learning_rate) {
    cv::Mat input_image = image.getMat();
    if (input_image.channels() == 3) {
        cv::cvtColor(input_image, input_image, CV_BGR2GRAY);
    }

    cv::Mat *new_model = new cv::Mat(input_image.rows, input_image.cols, CV_32F);
    for (int r = 0; r < input_image.rows; r++) {
        for (int c = 0; c < input_image.cols; c++) {
            // TODO What type of values are we getting here?
            new_model->at<float>(r, c) = densityNeighborhood(input_image, r, c, this->radius);
        }
    }

    if (model->empty()) {
        LOG(INFO) << "Background Model is empty, setting it to a copy of the foreground.";
        model = new cv::Mat(input_image.rows, input_image.cols, CV_32F);
        new_model->copyTo(*model);
    }

    cv::Mat diff, char_mat;
    cv::absdiff(*new_model, *model, diff);


    if (fgmask.needed()) {
        // TODO Check if this thing is working correctly.
        diff.convertTo(char_mat, CV_8U, 255.0/(1-0));
        fgmask.create(input_image.size(), input_image.type());
        cv::Mat mask = fgmask.getMat();
        cv::threshold(char_mat, fgmask, 150, 255, cv::THRESH_BINARY);
    }

    // Update Model
    updateModel(*new_model, learning_rate);
}

float KOSub::densityNeighborhood(const cv::Mat &image, const int row, const int col, const int radius) {
    LOG_IF(ERROR, radius <= 0) << "Radius was set to zero.";
    const unsigned int row_start = std::max(static_cast<int>(row) - radius, 0);
    const unsigned int col_start = std::max(col - radius, 0);
    const unsigned int row_end = std::min(row + radius, image.rows);
    const unsigned int col_end = std::min(col + radius, image.cols);

    float density = 0;
    unsigned char val = image.at<unsigned char>(row, col);
    for (int r = row_start; r < row_end; r++) {
        for (int c = col_start; c < col_end; c++) {
            density += diracDelta(val, image.at<unsigned char>(r, c));
        }
    }
    return density / (radius * radius * 4);
}

void KOSub::updateModel(const cv::Mat &new_model, const double &rate) {
    LOG_IF(ERROR, rate < 0 || rate > 1) << "Invalid rate.";
    LOG_IF(ERROR, new_model.empty() == true) << "Model passed in is empty.";
    LOG_IF(ERROR, model->empty() == true) << "Apptempting to update an empty model.";

    cv::Mat prev_model(*model);
    for (int r = 0; r < model->rows; r++) {
        for (int c = 0; c < model->cols; c++) {
            float prev_val = prev_model.at<float>(r, c);
            float new_val = new_model.at<float>(r, c);
            model->at<float>(r, c) = ((1-rate) * prev_val) + (rate * new_val);
        }
    }
}

