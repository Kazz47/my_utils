#include "kosub.hpp"

#include <glog/logging.h>

#include <cmath>
#include <algorithm>
#include <opencv2/imgproc/imgproc.hpp>

KOSub::KOSub(
        const int rows,
        const int cols,
        const int colors,
        const int radius
        ) {
    this->rows = rows;
    this->cols = cols;
    this->colors = colors;
    this->radius = radius;

    this->color_reduction = static_cast<float>(this->colors)/this->max_colors;
    this->color_expansion = static_cast<float>(this->max_colors)/this->colors;

    int sizes[] = {this->rows, this->cols, this->colors};
    this->model = new cv::Mat(3, sizes, CV_32F, cv::Scalar(0));
    this->background_image = new cv::Mat(this->rows, this->cols, CV_8U, cv::Scalar(0));
    this->diff = new cv::Mat(this->rows, this->cols, CV_32F, cv::Scalar(0));
}

//Only supports 8-bit images
void KOSub::operator()(cv::InputArray image, cv::OutputArray fgmask, double learning_rate) {
    this->apply(image, fgmask, learning_rate);
}

KOSub::~KOSub() {
    delete model;
    delete background_image;
    delete diff;
}

void KOSub::apply(cv::InputArray image, cv::OutputArray fgmask, double learning_rate) {
    cv::Mat input_image = image.getMat();
    if (input_image.channels() == 3) {
        cv::cvtColor(input_image, input_image, CV_BGR2GRAY);
    }

    for (int r = 0; r < input_image.rows; r++) {
        for (int c = 0; c < input_image.cols; c++) {
            unsigned char bg_color = 0;
            float max_freq = 0;
            float val = 0;
            for (int z = 0; z < this->colors; z++) {
                LOG_IF(ERROR, this->radius <= 0) << "Radius was set to zero.";
                float new_val = densityNeighborhood(input_image, r, c, z, this->radius);
                float prev_val = model->at<float>(r,c,z);
                val += new_val * prev_val;
                model->at<float>(r,c,z) = ((1-learning_rate) * prev_val) + (learning_rate * new_val);
                if (model->at<float>(r,c,z) > max_freq) {
                    bg_color = z;
                }
            }
            this->background_image->at<unsigned char>(r,c) = bg_color * this->color_expansion;
            diff->at<float>(r,c) = 1-sqrt(val);
        }
    }

    if (fgmask.needed()) {
        cv::Mat char_mat;
        diff->convertTo(char_mat, CV_8U, 255.0);
        fgmask.create(input_image.size(), input_image.type());
        cv::threshold(char_mat, fgmask, 150, 255, cv::THRESH_BINARY);
    }
}

void KOSub::getBackgroundImage(cv::OutputArray background_image) const {
    if (!background_image.needed() || model->empty()) {
        VLOG(1) << "Background was empty";
        return;
    }

    background_image.create(rows, cols, CV_8U);
    cv::Mat output = background_image.getMat();
    this->background_image->copyTo(output);
    VLOG(1) << "Got background image";
}

