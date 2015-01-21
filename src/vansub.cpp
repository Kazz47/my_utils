#include "vansub.hpp"

#include <glog/logging.h>

#include <opencv2/imgproc/imgproc.hpp>

VANSub::VANSub(
        const int rows,
        const int cols,
        const unsigned int history
        ) {
    LOG_IF(ERROR, history <= 0) << "History was set to zero.";
    this->rows = rows;
    this->cols = cols;
    this->history = history;
    int sizes[] = {this->rows, this->cols, 256};
    this->model = new cv::Mat(3, sizes, CV_8U, cv::Scalar(0));
    this->background_image = new cv::Mat(this->rows, this->cols, CV_8U, cv::Scalar(0));
    this->diff = new cv::Mat(this->rows, this->cols, CV_32F, cv::Scalar(0));

    std::random_device rd;
    this->gen = new std::mt19937(rd());
    this->history_update = new std::uniform_int_distribution<int>(0, history-1);
    this->absorb_foreground = new std::uniform_int_distribution<int>(0, 15);
}

//Only supports 8-bit images
void VANSub::operator()(cv::InputArray image, cv::OutputArray fgmask, double learning_rate) {
    this->apply(image, fgmask, learning_rate);
}

VANSub::~VANSub() {
    delete model;
    delete background_image;
    delete diff;
}

void VANSub::apply(cv::InputArray image, cv::OutputArray fgmask, double learning_rate) {
    cv::Mat input_image = image.getMat();
    if (input_image.channels() == 3) {
        cv::cvtColor(input_image, input_image, CV_BGR2GRAY);
    }

    if (!this->initiated) {
        for (int r = 0; r < input_image.rows; r++) {
            for (int c = 0; c < input_image.cols; c++) {
                for (int z = 0; z < this->history; z++) {
                    model->at<unsigned char>(r,c,z) = input_image.at<unsigned char>(r,c);
                }
            }
        }
        this->initiated = true;
    }

    this->diff->setTo(cv::Scalar(1));

    for (int r = 0; r < input_image.rows; r++) {
        for (int c = 0; c < input_image.cols; c++) {
            unsigned char input_val = input_image.at<unsigned char>(r,c);
            unsigned int matches = 0;
            for (int z = 0; z < this->history; z++) {
                if (input_val == model->at<unsigned char>(r,c,z)) {
                    matches++;
                    if (matches >= req_matches) {
                        break;
                    }
                }
            }
            if (matches >= req_matches) {
                // Add pixel value to background model.
                int pos = (*(this->history_update))(*(this->gen));
                LOG_IF(ERROR, pos >= this->history) << "RNG Error";
                model->at<unsigned char>(r,c,pos) = input_val;
                diff->at<float>(r,c) = 0.0;
            } else {
                int rng_update = (*(this->absorb_foreground))(*(this->gen));
                if (rng_update == 0) {
                    int pos = (*(this->history_update))(*(this->gen));
                    LOG_IF(ERROR, pos >= this->history) << "RNG Error";
                    model->at<unsigned char>(r,c,pos) = input_val;
                }
            }
        }
    }

    if (fgmask.needed()) {
        cv::Mat char_mat;
        diff->convertTo(char_mat, CV_8U, 255.0);
        fgmask.create(input_image.size(), input_image.type());
        cv::Mat mask = fgmask.getMat();
        cv::threshold(char_mat, fgmask, 220, 255, cv::THRESH_BINARY);
    }
}

void VANSub::getBackgroundImage(cv::OutputArray background_image) const {
    if (!background_image.needed() || model->empty()) {
        VLOG(1) << "Background was empty";
        return;
    }

    for (int r = 0; r < this->rows; r++) {
        for (int c = 0; c < this->cols; c++) {
            this->background_image->at<unsigned char>(r,c) = this->model->at<unsigned char>(r,c,0);
        }
    }

    background_image.create(rows, cols, CV_8U);
    cv::Mat output = background_image.getMat();
    this->background_image->copyTo(output);
    VLOG(1) << "Got background image";
}

