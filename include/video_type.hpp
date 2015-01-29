#ifndef VIDEO_TYPE_H
#define VIDEO_TYPE_H

#include <opencv2/core/core.hpp>

class VideoType {
    private:
        cv::Size size;
        cv::Rect watermark_rect;
        cv::Rect timestamp_rect;

        void setWatermarkRect(const cv::Point&, const cv::Point&);
        void setTimestampRect(const cv::Point&, const cv::Point&);
        void loadType();

    public:
        VideoType(const cv::Size&);

        int getWidth();
        int getHeight();
        cv::Size getSize();
        cv::Rect getWatermarkRect();
        cv::Rect getTimestampRect();

        cv::Mat getMask();
        void drawZones(cv::Mat &frame, const cv::Scalar &color);
};

#endif //VIDEO_TYPE_H
