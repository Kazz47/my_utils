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
    this->rows = rows;
    this->cols = cols;
    this->radius = radius;
    this->colors = colors;
    this->history = history;

    this->color_reduction = static_cast<float>(this->colors)/this->max_colors;
    this->color_expansion = static_cast<float>(this->max_colors)/this->colors;

    int sizes[] = {this->rows, this->cols, this->history};
    this->model = new cv::Mat(3, sizes, CV_8U, cv::Scalar(0));
    this->diff = new cv::Mat(this->rows, this->cols, CV_32F, cv::Scalar(0));

    this->seed = 0;
    this->num_generated = 0;
    this->gen = new std::mt19937(this->seed);
    this->update = new boost::random::uniform_real_distribution<float>(0, 1);
    this->history_update = new boost::random::uniform_int_distribution<int>(0, history-1);
    //this->update_neighbor= new boost::random::uniform_int_distribution<int>(0, 100);
    this->pick_neighbor = new boost::random::uniform_int_distribution<int>(0, 7);
}

VANSub::VANSub(const VANSub &other) {
    this->initiated = other.initiated;

    this->rows = other.rows;
    this->cols = other.cols;
    this->radius = other.radius;
    this->colors = other.colors;
    this->history = other.history;

    this->color_reduction = other.color_reduction;
    this->color_expansion = other.color_expansion;

    this->model = other.model;
    this->diff = other.diff;

    this->seed = other.seed;
    this->num_generated = other.num_generated;
    this->gen = new std::mt19937(this->seed);
    this->gen->discard(num_generated);

    this->update = new boost::random::uniform_real_distribution<float>(0, 1);
    this->history_update = new boost::random::uniform_int_distribution<int>(0, history-1);
    //this->update_neighbor= new boost::random::uniform_int_distribution<int>(0, 100);
    this->pick_neighbor = new boost::random::uniform_int_distribution<int>(0, 7);
}

VANSub::~VANSub() {
    delete this->gen;
    delete this->update;
    delete this->history_update;
    //delete this->update_neighbor;
    delete this->pick_neighbor;
}

void VANSub::updateModel(const int &r, const int &c, const unsigned char &val, const bool &update_neighbor) {
    // Add pixel value to background model.
    this->num_generated += 1;
    float rng_update = (*(this->update))(*(this->gen));
    if (rng_update <= 1.0/16.0) {
        this->num_generated += 1;
        int pos = (*(this->history_update))(*(this->gen));
        LOG_IF(ERROR, pos >= this->history) << "RNG Error";
        this->model->at<unsigned char>(r,c,pos) = val;
        if (update_neighbor) {
            this->num_generated += 1;
            int neighbor = (*(this->pick_neighbor))(*(this->gen));
            switch(neighbor) {
                case 0:
                    if (r > 1 && c > 1) {
                        updateModel(r-1, c-1, val, false);
                    }
                    break;
                case 1:
                    if (r > 1) {
                        updateModel(r-1, c, val, false);
                    }
                    break;
                case 2:
                    if (r > 1 && c < this->cols - 1) {
                        updateModel(r-1, c+1, val, false);
                    }
                    break;
                case 3:
                    if (c > 1) {
                        updateModel(r, c-1, val, false);
                    }
                    break;
                case 4:
                    if (c < this->cols - 1) {
                        updateModel(r, c+1, val, false);
                    }
                    break;
                case 5:
                    if (r < this->rows - 1 && c > 1) {
                        updateModel(r+1, c-1, val, false);
                    }
                    break;
                case 6:
                    if (r < this->rows - 1) {
                        updateModel(r+1, c, val, false);
                    }
                    break;
                case 7:
                    if (r < this->rows - 1 && c < this->cols - 1) {
                        updateModel(r+1, c+1, val, false);
                    }
                    break;
                default:
                    LOG(ERROR) << "Unknown case selected for neightbor update.";
                    break;
            }
        }
    }
}

//Only supports 8-bit images
void VANSub::operator()(cv::InputArray image, cv::OutputArray fgmask, double learning_rate) {
    this->apply(image, fgmask, learning_rate);
}

void VANSub::apply(cv::InputArray image, cv::OutputArray fgmask, double learning_rate) {
    cv::Mat input_image = image.getMat();
    if (input_image.channels() == 3) {
        cv::cvtColor(input_image, input_image, CV_BGR2GRAY);
    }
    LOG_IF(WARNING, input_image.rows != this->rows || input_image.cols != this->cols) << "Different size image: " << input_image.size() << " vs " << this->model->size();

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
                updateModel(r, c, input_val);
            } else { // Foreground
                /*
                this->num_generated += 1;
                int rng_update = (*(this->absorb_foreground))(*(this->gen));
                if (rng_update == 0) {
                    this->num_generated += 1;
                    int pos = (*(this->history_update))(*(this->gen));
                    LOG_IF(ERROR, pos >= this->history) << "RNG Error";
                    model->at<unsigned char>(r,c,pos) = input_val;
                }
                */
            }
        }
    }

    if (fgmask.needed()) {
        cv::Mat char_mat;
        diff->convertTo(char_mat, CV_8U, 255);
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

    background_image.create(this->rows, this->cols, CV_8U);
    cv::Mat output = background_image.getMat();

    for (int r = 0; r < this->rows; r++) {
        for (int c = 0; c < this->cols; c++) {
            output.at<unsigned char>(r,c) = this->model->at<unsigned char>(r,c,2) * this->color_expansion;
        }
    }
}

void VANSub::initiateModel(cv::Mat &image, cv::Rect &random_init) {
    boost::random::uniform_int_distribution<int> random_row(0, image.rows-1);
    boost::random::uniform_int_distribution<int> random_col(0, image.cols-1);
    for (int r = 0; r < image.rows; r++) {
        for (int c = 0; c < image.cols; c++) {
            if (!random_init.contains(cv::Point(c,r))) {
                for (int z = 0; z < this->req_matches; z++) {
                    model->at<unsigned char>(r,c,z) = image.at<unsigned char>(r,c) * this->color_reduction;
                }
                for (int z = this->req_matches; z < this->history; z++) {
                    // TODO Add random pixel value here
                    this->num_generated += 2;
                    int row = random_row(*(this->gen));
                    int col = random_col(*(this->gen));
                    model->at<unsigned char>(r,c,z) = image.at<unsigned char>(row,col);
                }
            } else {
                // And values randomly from outside the rect
                for (int z = 0; z < this->history; z++) {
                    // TODO Add random pixel value here
                    this->num_generated += 2;
                    int row = random_row(*(this->gen));
                    int col = random_col(*(this->gen));
                    model->at<unsigned char>(r,c,z) = image.at<unsigned char>(row,col);
                }
            }
        }
    }
}

void VANSub::read(const cv::FileNode &node) {
    //Load New Matrices
    cv::Mat temp;
    node["MODEL"] >> temp;
    this->model = new cv::Mat(temp);
    node["DIFF"] >> temp;
    this->diff = new cv::Mat(temp);

    //Update Values
    node["ROWS"] >> this->rows;
    node["COLS"] >> this->cols;
    node["RADIUS"] >> this->radius;
    node["COLORS"] >> this->colors;
    node["HISTORY"] >> this->history;
    node["COLOR_REDUCTION"] >> this->color_reduction;
    node["COLOR_EXPANSION"] >> this->color_expansion;
    node["INITIATED"] >> this->initiated;
    node["SEED"] >> this->seed;
    node["NUM_GENERATED"] >> this->num_generated;
}

void VANSub::write(cv::FileStorage &fs) const {
    fs << "{";
    fs << "ROWS" << this->rows;
    fs << "COLS" << this->cols;
    fs << "RADIUS" << this->radius;
    fs << "COLORS" << this->colors;
    fs << "HISTORY" << this->history;
    fs << "COLOR_REDUCTION" << this->color_reduction;
    fs << "COLOR_EXPANSION" << this->color_expansion;
    fs << "INITIATED" << this->initiated;
    fs << "SEED" << this->seed;
    fs << "NUM_GENERATED" << this->num_generated;
    fs << "MODEL" << *(this->model);
    fs << "DIFF" << *(this->diff);
    fs << "}";
}

std::ostream& VANSub::print(std::ostream &out) const {
    out << "{ ";
    out << "rows = " << this->rows << ", ";
    out << "cols = " << this->cols << ", ";
    out << "model = " << this->model->size();
    out << " }";
    return out;
}

