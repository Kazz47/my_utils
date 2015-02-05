#include "video_type.hpp"

#include <opencv2/imgproc/imgproc.hpp>

// Accessors
VideoType::VideoType(const cv::Size &size) {
    this->size = size;

    loadType();
}

int VideoType::getWidth() {
    return this->size.width;
}

int VideoType::getHeight() {
    return this->size.height;
}

cv::Size VideoType::getSize() {
    return this->size;
}

cv::Rect VideoType::getWatermarkRect() {
    return this->watermark_rect;
}

cv::Rect VideoType::getTimestampRect() {
    return this->timestamp_rect;
}

// Functions

cv::Mat VideoType::getMask() {
    cv::Mat mask(this->getHeight(), this->getWidth(), CV_8UC1, cv::Scalar(1));
    cv::rectangle(mask, this->getTimestampRect(), cv::Scalar(0), CV_FILLED);
    cv::rectangle(mask, this->getWatermarkRect(), cv::Scalar(0), CV_FILLED);
    return mask;
}

void VideoType::drawZones(cv::Mat &frame, const cv::Scalar &color) {
    cv::rectangle(frame, timestamp_rect, color);
    cv::rectangle(frame, watermark_rect, color);
}

// Private

void VideoType::setWatermarkRect(const cv::Point &top_left, const cv::Point &bottom_right) {
    this->watermark_rect = cv::Rect(top_left, bottom_right);
}

void VideoType::setTimestampRect(const cv::Point &top_left, const cv::Point &bottom_right) {
    this->timestamp_rect = cv::Rect(top_left, bottom_right);
}

// TODO This needs to be fixed to load in from a config file.
void VideoType::loadType() {
    if(this->getWidth() == 704 && this->getHeight() == 480) {
        cv::Point watermark_tl(12, 12);
        cv::Point watermark_br(90, 55);
        cv::Point timestamp_tl(520, 415);
        cv::Point timestamp_br(680, 470);
        setWatermarkRect(watermark_tl, watermark_br);
        setTimestampRect(timestamp_tl, timestamp_br);
    } else if(this->getWidth() == 352 && this->getHeight() == 240) {
        cv::Point watermark_tl(12, 12);
        cv::Point watermark_br(90, 55);
        cv::Point timestamp_tl(240, 190);
        cv::Point timestamp_br(335, 230);
        setWatermarkRect(watermark_tl, watermark_br);
        setTimestampRect(timestamp_tl, timestamp_br);
    }
}
