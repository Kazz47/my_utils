#include "bsub.hpp"

#include <glog/logging.h>

BSub::BSub() {
    VLOG(1) << "Created!";
}

BSub::BSub(int history) {
    VLOG(1) << "Created!";
}

BSub::~BSub() {
    VLOG(1) << "Deleted!";
}

void BSub::apply(cv::InputArray image, cv::OutputArray fgmask, double learningRate) {
    VLOG(1) << "Applied!";
}

void BSub::getBackgroundImage(cv::OutputArray backgroundImage) const {
    VLOG(1) << "Got background image";
}

