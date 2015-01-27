#include "vansub.hpp"

#include <glog/logging.h>

#include <cmath>
#include <opencv2/imgproc/imgproc.hpp>

VANSub::VANSub(
        const int rows,
        const int cols,
        const int colors,
        const int history
        ) {
    LOG_IF(ERROR, history <= 0) << "History was set to zero.";
    this->rows = rows;
    this->cols = cols;
    this->colors = colors;
    this->history = history;

    this->color_reduction = static_cast<float>(this->colors)/this->max_colors;
    this->color_expansion = static_cast<float>(this->max_colors)/this->colors;

    int sizes[] = {this->rows, this->cols, this->history};
    this->model = new cv::Mat(3, sizes, CV_8U, cv::Scalar(0));
    this->background_image = new cv::Mat(this->rows, this->cols, CV_8U, cv::Scalar(0));
    this->diff = new cv::Mat(this->rows, this->cols, CV_32F, cv::Scalar(0));

    std::random_device rd;
    this->gen = new std::mt19937(rd());
    this->history_update = new std::uniform_int_distribution<int>(0, history-1);
    this->update_neighbor= new std::uniform_int_distribution<int>(0, 15);
    this->pick_neighbor = new std::uniform_int_distribution<int>(0, 7);
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
        initiateModel(input_image);
        this->initiated = true;
    }

    this->diff->setTo(cv::Scalar(1));

    for (int r = 0; r < input_image.rows; r++) {
        for (int c = 0; c < input_image.cols; c++) {
            unsigned char input_val = input_image.at<unsigned char>(r,c) * this->color_reduction;
            unsigned int matches = 0;
            for (int z = 0; z < this->history; z++) {
                if (input_val == model->at<unsigned char>(r,c,z)) {
                    matches++;
                    if (matches >= req_matches) {
                        break;
                    }
                }
            }
            if (matches >= req_matches) { // Background
                // Set foreground mask to zero.
                diff->at<float>(r,c) = 0.0;

                // Add pixel value to background model.
                int pos = (*(this->history_update))(*(this->gen));
                LOG_IF(ERROR, pos >= this->history) << "RNG Error";
                model->at<unsigned char>(r,c,pos) = input_val;

                // Randomly add value to neighbor pixel
                // TODO Make this a function
                int rng_update = (*(this->update_neighbor))(*(this->gen));
                if (rng_update == 0) {
                    int neighbor = (*(this->pick_neighbor))(*(this->gen));
                    int pos = (*(this->history_update))(*(this->gen));
                    LOG_IF(ERROR, pos >= this->history) << "RNG Error";
                    switch(neighbor) {
                        case 0:
                            if (r > 1 && c > 1) {
                                model->at<unsigned char>(r-1,c-1,pos) = input_val;
                            }
                            break;
                        case 1:
                            if (r > 1) {
                                model->at<unsigned char>(r-1,c,pos) = input_val;
                            }
                            break;
                        case 2:
                            if (r > 1 && c < input_image.cols - 1) {
                                model->at<unsigned char>(r-1,c+1,pos) = input_val;
                            }
                            break;
                        case 3:
                            if (c > 1) {
                                model->at<unsigned char>(r,c-1,pos) = input_val;
                            }
                            break;
                        case 4:
                            if (c < input_image.cols - 1) {
                                model->at<unsigned char>(r,c+1,pos) = input_val;
                            }
                            break;
                        case 5:
                            if (r < input_image.rows - 1 && c > 1) {
                                model->at<unsigned char>(r+1,c-1,pos) = input_val;
                            }
                            break;
                        case 6:
                            if (r < input_image.rows - 1) {
                                model->at<unsigned char>(r+1,c,pos) = input_val;
                            }
                            break;
                        case 7:
                            if (r < input_image.rows - 1 && c < input_image.cols - 1) {
                                model->at<unsigned char>(r+1,c+1,pos) = input_val;
                            }
                            break;
                    }
                }
            } else { // Foreground
                /*
                int rng_update = (*(this->absorb_foreground))(*(this->gen));
                if (rng_update == 0) {
                    int pos = (*(this->history_update))(*(this->gen));
                    LOG_IF(ERROR, pos >= this->history) << "RNG Error";
                    model->at<unsigned char>(r,c,pos) = input_val;
                }
                */
            }
        }
    }

   //TODO Use (open-close filter && mask) as new mask for updating

    if (fgmask.needed()) {
        cv::Mat char_mat;
        diff->convertTo(char_mat, CV_8U, 255.0);
        fgmask.create(input_image.size(), input_image.type());
        cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(4,4), cv::Point(0,0));
        cv::morphologyEx(char_mat, char_mat, cv::MORPH_OPEN, kernel);
        cv::morphologyEx(char_mat, fgmask, cv::MORPH_CLOSE, kernel);
    }
}

void VANSub::getBackgroundImage(cv::OutputArray background_image) const {
    if (!background_image.needed() || model->empty()) {
        VLOG(1) << "Background was empty";
        return;
    }

    for (int r = 0; r < this->rows; r++) {
        for (int c = 0; c < this->cols; c++) {
            this->background_image->at<unsigned char>(r,c) = model->at<unsigned char>(r,c,0) * this->color_expansion;
        }
    }

    background_image.create(rows, cols, CV_8U);
    cv::Mat output = background_image.getMat();
    this->background_image->copyTo(output);
    VLOG(1) << "Got background image";
}

void VANSub::initiateModel(cv::Mat &image) {
    std::uniform_int_distribution<int> color_range(0, 5);
    for (int r = 0; r < image.rows; r++) {
        for (int c = 0; c < image.cols; c++) {
            for (int z = 0; z < this->req_matches; z++) {
                model->at<unsigned char>(r,c,z) = image.at<unsigned char>(r,c) * this->color_reduction;
            }
            for (int z = this->req_matches; z < this->history; z++) {
                // TODO Add random pixel value here
                if (z%2 == 0) {
                    model->at<unsigned char>(r,c,z) = model->at<unsigned char>(r,c,0) + color_range(*(this->gen));
                } else {
                    model->at<unsigned char>(r,c,z) = model->at<unsigned char>(r,c,0) - color_range(*(this->gen));
                }
                //LOG(INFO) << "(" << r << "," << c << "," << z << ") = " << static_cast<unsigned int>(model->at<unsigned char>(r,c,z));
            }
        }
    }
}
