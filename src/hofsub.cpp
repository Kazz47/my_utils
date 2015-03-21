#include "hofsub.hpp"

#include <glog/logging.h>

#include <cmath>
#include <opencv2/imgproc/imgproc.hpp>

HOFSub::HOFSub(
        const int rows,
        const int cols,
        const int threshold,
        const int colors,
        const int history
        ) {
    LOG_IF(ERROR, history <= 0) << "History was set to zero.";

    this->initiated = false;

    this->rows = rows;
    this->cols = cols;
    this->colors = colors;
    this->history = history;

    this->color_reduction = static_cast<float>(this->colors)/this->MAX_COLORS;
    this->color_expansion = static_cast<float>(this->MAX_COLORS)/this->colors;

    int sizes[] = {this->rows, this->cols, this->history};
    this->model = new cv::Mat(3, sizes, CV_8U, cv::Scalar(0));
    this->decision_distance = new cv::Mat(this->rows, this->cols, CV_32F, cv::Scalar(0));
    this->threshold = new cv::Mat(this->rows, this->cols, CV_32F, cv::Scalar(threshold));
    this->update_val = new cv::Mat(this->rows, this->cols, CV_32F, cv::Scalar(UPDATE_MIN));

    std::random_device rd;
    this->num_generated = 0;
    this->gen = new std::mt19937(0);
    this->update = new boost::random::uniform_real_distribution<float>(0, 1);
    this->history_update = new boost::random::uniform_int_distribution<int>(0, history-1);
    this->pick_neighbor = new boost::random::uniform_int_distribution<int>(0, 7);
}

HOFSub::HOFSub(const HOFSub &other) {
    this->initiated = other.initiated;

    this->rows = other.rows;
    this->cols = other.cols;
    this->colors = other.colors;
    this->history = other.history;

    this->color_reduction = other.color_reduction;
    this->color_expansion = other.color_expansion;

    this->model = other.model;
    this->decision_distance = other.decision_distance;
    this->threshold = other.threshold;
    this->update_val = other.update_val;

    this->seed = other.seed;
    this->num_generated = other.num_generated;
    this->gen = new std::mt19937(seed);
    this->gen->discard(num_generated);

    this->update = new boost::random::uniform_real_distribution<float>(0, 1);
    this->history_update = new boost::random::uniform_int_distribution<int>(0, history-1);
    this->pick_neighbor = new boost::random::uniform_int_distribution<int>(0, 7);
}

HOFSub::~HOFSub() {
    delete this->update;
    delete this->history_update;
    delete this->pick_neighbor;
}

void HOFSub::updateModel(const int &r, const int &c, const unsigned char &val, const bool &update_neighbor) {
    // Add pixel value to background model.
    this->num_generated += 1;
    float rng_update = (*(this->update))(*(this->gen));
    if (rng_update <= 1/this->update_val->at<float>(r,c)) {
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
void HOFSub::operator()(cv::InputArray image, cv::OutputArray fgmask, double learning_rate) {
    this->apply(image, fgmask, learning_rate);
}

void HOFSub::apply(cv::InputArray image, cv::OutputArray fgmask, double learning_rate) {
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

    cv::Mat mask(this->rows, this->cols, CV_8U, cv::Scalar(255));

    for (int r = 0; r < input_image.rows; r++) {
        for (int c = 0; c < input_image.cols; c++) {
            unsigned char input_val = input_image.at<unsigned char>(r,c) * this->color_reduction;
            int matches = 0;
            float min_dist = THRESH_MAX;
            for (int z = 0; z < this->history; z++) {
                float dist = abs(static_cast<float>(input_val) - this->model->at<unsigned char>(r,c,z));
                if (dist <= this->threshold->at<float>(r,c)) {
                    matches++;
                }
                if (dist < min_dist) {
                    min_dist = dist;
                }
            }
            this->decision_distance->at<float>(r,c) = learning_rate * min_dist + (1 - learning_rate) * this->decision_distance->at<float>(r,c);

            // Update threshold
            if (this->threshold->at<float>(r,c) > this->decision_distance->at<float>(r,c)  * THRESH_SCALE) {
                this->threshold->at<float>(r,c) = this->threshold->at<float>(r,c) * (1 - THRESH_DEC_RATE);
                if (this->threshold->at<float>(r,c) < THRESH_MIN) {
                    this->threshold->at<float>(r,c) = THRESH_MIN;
                }
            } else {
                this->threshold->at<float>(r,c) = this->threshold->at<float>(r,c) * (1 + THRESH_INC_RATE);
                if (this->threshold->at<float>(r,c) > THRESH_MAX) {
                    this->threshold->at<float>(r,c) = THRESH_MAX;
                }
            }

            // Update model
            if (matches >= REQ_MATCHES) { // Background
                // Set foreground mask to zero.
                mask.at<unsigned char>(r,c) = 0;
                this->update_val->at<float>(r,c) = this->update_val->at<float>(r,c) - UPDATE_DEC_RATE/this->decision_distance->at<float>(r,c);
                if (this->update_val->at<float>(r,c) < UPDATE_MIN) {
                    this->update_val->at<float>(r,c) = UPDATE_MIN;
                }
                updateModel(r, c, input_val);
            } else { // Foreground
                this->update_val->at<float>(r,c) = this->update_val->at<float>(r,c) + UPDATE_INC_RATE/this->decision_distance->at<float>(r,c);
                if (this->update_val->at<float>(r,c) > UPDATE_MAX) {
                    this->update_val->at<float>(r,c) = UPDATE_MAX;
                }
            }
        }
    }

    if (fgmask.needed()) {
        fgmask.create(input_image.size(), input_image.type());

        // Smooth mask (remove noise)
        //cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(4,4), cv::Point(0,0));
        cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5,5), cv::Point(0,0));
        cv::morphologyEx(mask, mask, cv::MORPH_OPEN, kernel);
        cv::morphologyEx(mask, mask, cv::MORPH_CLOSE, kernel);

        mask.copyTo(fgmask);

        // Find Convex Hull
        /*
        std::vector<std::vector<cv::Point>> contours;
        std::vector<cv::Vec4i> hierarchy;
        cv::findContours(mask, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
        std::vector<std::vector<cv::Point>> hull(contours.size());
        for (unsigned int i = 0; i < contours.size(); i++) {
            cv::convexHull(cv::Mat(contours[i]), hull[i], false);
            cv::drawContours(fgmask, hull, i, cv::Scalar(100), CV_FILLED);
        }
        */
    }
}

void HOFSub::getBackgroundImage(cv::OutputArray background_image) const {
    if (!background_image.needed() || this->model->empty()) {
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

void HOFSub::initiateModel(cv::Mat &image, cv::Rect &random_init) {
    boost::random::uniform_int_distribution<int> random_row(0, image.rows-1);
    boost::random::uniform_int_distribution<int> random_col(0, image.cols-1);
    for (int r = 0; r < image.rows; r++) {
        for (int c = 0; c < image.cols; c++) {
            if (!random_init.contains(cv::Point(c,r))) {
                for (int z = 0; z < this->REQ_MATCHES; z++) {
                    this->model->at<unsigned char>(r,c,z) = image.at<unsigned char>(r,c) * this->color_reduction;
                }
                for (int z = this->REQ_MATCHES; z < this->history; z++) {
                    this->num_generated += 2;
                    int row = random_row(*(this->gen));
                    int col = random_col(*(this->gen));
                    this->model->at<unsigned char>(r,c,z) = image.at<unsigned char>(row,col);
                }
            } else {
                // And values randomly from outside the rect
                for (int z = 0; z < this->history; z++) {
                    this->num_generated += 2;
                    int row = random_row(*(this->gen));
                    int col = random_col(*(this->gen));
                    this->model->at<unsigned char>(r,c,z) = image.at<unsigned char>(row,col);
                }
            }
        }
    }
}

void HOFSub::read(const cv::FileNode &node) {
    //Load New Matrices
    cv::Mat temp;
    node["MODEL"] >> temp;
    this->model = new cv::Mat(temp);
    node["DECISION_DIST"] >> temp;
    this->decision_distance = new cv::Mat(temp);
    node["THRESHOLD"] >> temp;
    this->threshold = new cv::Mat(temp);
    node["UPDATE_VAL"] >> temp;
    this->update_val = new cv::Mat(temp);

    //Update Values
    node["ROWS"] >> this->rows;
    node["COLS"] >> this->cols;
    node["COLORS"] >> this->colors;
    node["HISTORY"] >> this->history;
    node["COLOR_REDUCTION"] >> this->color_reduction;
    node["COLOR_EXPANSION"] >> this->color_expansion;
    node["INITIATED"] >> this->initiated;
    node["NUM_GENERATED"] >> this->num_generated;
}

void HOFSub::write(cv::FileStorage &fs) const {
    fs << "{";
    fs << "ROWS" << this->rows;
    fs << "COLS" << this->cols;
    fs << "COLORS" << this->colors;
    fs << "HISTORY" << this->history;
    fs << "COLOR_REDUCTION" << this->color_reduction;
    fs << "COLOR_EXPANSION" << this->color_expansion;
    fs << "INITIATED" << this->initiated;
    fs << "NUM_GENERATED" << this->num_generated;
    fs << "MODEL" << *(this->model);
    fs << "DECISION_DIST" << *(this->decision_distance);
    fs << "THRESHOLD" << *(this->threshold);
    fs << "UPDATE_VAL" << *(this->update_val);
    fs << "}";
}

std::ostream& HOFSub::print(std::ostream &out) const {
    out << "{ ";
    out << "rows = " << this->rows << ", ";
    out << "cols = " << this->cols << ", ";
    out << "model = " << this->model->size();
    out << " }";
    return out;
}

