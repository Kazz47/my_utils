#include "bsub.hpp"

#include <glog/logging.h>

BSub::BSub(const unsigned int &history) {
    background = new cv::Mat();
    VLOG(1) << "Created!";
}

BSub::~BSub() {
    delete background;
    VLOG(1) << "Deleted!";
}

void BSub::operator()(cv::InputArray image, cv::OutputArray fgmask, double learningRate) {
    this->apply(image, fgmask, learningRate);
}

void BSub::apply(cv::InputArray image, cv::OutputArray fgmask, double learningRate) {
    cv::Mat input_image = image.getMat();
    input_image.copyTo(*background);
    if(fgmask.needed()) {
        fgmask.create(input_image.size(), input_image.type());
        cv::Mat mask = fgmask.getMat();
        input_image.copyTo(mask);
    }
}

void BSub::getBackgroundImage(cv::OutputArray backgroundImage) const {
    if(!backgroundImage.needed() || background->empty()) {
        VLOG(1) << "Background was empty";
        return;
    }

    backgroundImage.create(background->size(), background->type());
    cv::Mat output = backgroundImage.getMat();
    background->copyTo(output);
    VLOG(1) << "Got background image";
}

