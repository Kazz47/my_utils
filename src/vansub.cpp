#include "vansub.hpp"

#include <glog/logging.h>

#include <cmath>
#include <opencv2/imgproc/imgproc.hpp>

VANSub::VANSub(
        const int rows,
        const int cols,
        const int radius,
        const int colors,
        const int history
        ) {
    LOG_IF(ERROR, history <= 0) << "History was set to zero.";

    this->initiated = false;

    this->type = new VideoType(cv::Size(cols, rows));
    this->radius = radius;
    this->colors = colors;
    this->history = history;

    this->color_reduction = static_cast<float>(this->colors)/this->max_colors;
    this->color_expansion = static_cast<float>(this->max_colors)/this->colors;

    this->masks = new std::vector<cv::Rect>();
    this->masks->push_back(cv::Rect(530, 420, 150, 50));

    int sizes[] = {this->type->getHeight(), this->type->getWidth(), this->history};
    this->model = new cv::Mat(3, sizes, CV_8U, cv::Scalar(0));
    this->background_image = new cv::Mat(this->type->getSize(), CV_8U, cv::Scalar(0));
    this->diff = new cv::Mat(this->type->getSize(), CV_32F, cv::Scalar(0));

    std::random_device rd;
    this->gen = new std::mt19937(rd());
    this->history_update = new std::uniform_int_distribution<int>(0, history-1);
    this->update_neighbor= new std::uniform_int_distribution<int>(0, 15);
    //this->update_neighbor= new std::uniform_int_distribution<int>(0, 100);
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
    LOG_IF(WARNING, input_image.rows != this->type->getHeight() || input_image.cols != this->type->getWidth()) << "Different size image: " << input_image.size() << " vs " << this->model->size();

    // Mask
    cv::rectangle(input_image, this->type->getTimestampRect(), cv::Scalar(0,0,0), CV_FILLED);
    cv::rectangle(input_image, this->type->getWatermarkRect(), cv::Scalar(0,0,0), CV_FILLED);
    // Equalization
    //cv::equalizeHist(input_image, input_image);

    if (!this->initiated) {
        //cv::Rect random_init(100, 200, input_image.cols-200, input_image.rows-300);
        cv::Rect random_init(0,0,0,0);
        initiateModel(input_image, random_init);
        this->initiated = true;
    }

    this->diff->setTo(cv::Scalar(1));

    for (int r = 0; r < input_image.rows; r++) {
        for (int c = 0; c < input_image.cols; c++) {
            unsigned char input_val = input_image.at<unsigned char>(r,c) * this->color_reduction;
            int matches = 0;
            for (int z = 0; z < this->history; z++) {
                int dist = abs(static_cast<int>(input_val) - model->at<unsigned char>(r,c,z));
                if (dist <= this->radius) {
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

    if (fgmask.needed()) {
        // Mask image
        for (unsigned int i = 0; i < this->masks->size(); i++) {
            cv::rectangle(*(this->diff), this->masks->at(i), cv::Scalar(0), CV_FILLED);
        }

        cv::Mat char_mat;
        diff->convertTo(char_mat, CV_8U, 255.0);
        fgmask.create(input_image.size(), input_image.type());

        // Smooth mask (remove noise)
        //cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(4,4), cv::Point(0,0));
        cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5,5), cv::Point(0,0));
        cv::morphologyEx(char_mat, char_mat, cv::MORPH_OPEN, kernel);
        cv::morphologyEx(char_mat, char_mat, cv::MORPH_CLOSE, kernel);

        char_mat.copyTo(fgmask);

        // Find Convex Hull
        std::vector<std::vector<cv::Point>> contours;
        std::vector<cv::Vec4i> hierarchy;
        cv::findContours(char_mat, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
        std::vector<std::vector<cv::Point>> hull(contours.size());
        for (unsigned int i = 0; i < contours.size(); i++) {
            cv::convexHull(cv::Mat(contours[i]), hull[i], false);
            cv::drawContours(fgmask, hull, i, cv::Scalar(100), CV_FILLED);
        }
    }
}

void VANSub::getBackgroundImage(cv::OutputArray background_image) const {
    if (!background_image.needed() || model->empty()) {
        VLOG(1) << "Background was empty";
        return;
    }

    for (int r = 0; r < this->type->getHeight(); r++) {
        for (int c = 0; c < this->type->getWidth(); c++) {
            this->background_image->at<unsigned char>(r,c) = model->at<unsigned char>(r,c,2) * this->color_expansion;
        }
    }

    background_image.create(this->type->getSize(), CV_8U);
    cv::Mat output = background_image.getMat();
    this->background_image->copyTo(output);
}

void VANSub::initiateModel(cv::Mat &image, cv::Rect &random_init) {
    std::uniform_int_distribution<int> random_row(0, image.rows-1);
    std::uniform_int_distribution<int> random_col(0, image.cols-1);
    for (int r = 0; r < image.rows; r++) {
        for (int c = 0; c < image.cols; c++) {
            if (!random_init.contains(cv::Point(c,r))) {
                for (int z = 0; z < this->req_matches; z++) {
                    model->at<unsigned char>(r,c,z) = image.at<unsigned char>(r,c) * this->color_reduction;
                }
                for (int z = this->req_matches; z < this->history; z++) {
                    // TODO Add random pixel value here
                    int row = random_row(*(this->gen));
                    int col = random_col(*(this->gen));
                    model->at<unsigned char>(r,c,z) = image.at<unsigned char>(row,col);
                }
            } else {
                // And values randomly from outside the rect
                for (int z = 0; z < this->history; z++) {
                    // TODO Add random pixel value here
                    int row = random_row(*(this->gen));
                    int col = random_col(*(this->gen));
                    model->at<unsigned char>(r,c,z) = image.at<unsigned char>(row,col);
                }
            }
        }
    }
}

