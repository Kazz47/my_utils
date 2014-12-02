#include <fstream>
#include <sstream>

#include <glog/logging.h>
#include <opencv2/core/core.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#define VISUAL

// Name of the main program window
static const std::string W_NAME = "WINDOW";

// Frequency in seconds to write frame to disk
size_t WRITE_FREQUENCY = 5;

std::string getUsage() {
    std::stringstream ss;
    ss << "Usage: <image> <min_area> <max_area>";
    return ss.str();
}

std::string getDesc() {
    std::stringstream ss;
    ss << "Description: All inputs should be normalized: [0, 1]";
    return ss.str();
}

int main(int argc, char** argv) {
    // Log to stderr instead of to the tmp directory
    FLAGS_logtostderr = 1;
    // Initiate Google Logging
    google::InitGoogleLogging(argv[0]);

    if (argc < 4) {
        LOG(ERROR) << "Not enough arguments:";
        LOG(ERROR) << getUsage();
        LOG(ERROR) << getDesc();
        exit(1);
    }

    // Read input values
    std::string img_filename = argv[1];
    double min_area = atoi(argv[2]);
    double max_area = atoi(argv[3]);

    // Blob Detection Params
    cv::SimpleBlobDetector::Params params;
    params.minDistBetweenBlobs = 100.0f;
    params.filterByInertia = false;
    params.filterByConvexity = false;
    params.filterByColor = true;
    params.blobColor = 255;
    params.filterByCircularity = false;
    params.filterByArea = true;
    params.minArea = min_area; //600
    params.maxArea = max_area; //7200

    cv::Ptr<cv::FeatureDetector> blob_detect = new cv::SimpleBlobDetector(params);
    blob_detect->create("BisonBlob");

    LOG(INFO) << "Checking for blobs in: '" << img_filename << "'";
    cv::Mat image = cv::imread(img_filename, CV_LOAD_IMAGE_COLOR);
    //cv::resize(image, image, cv::Size(2056,1028));

    if (image.empty()) {
        LOG(FATAL) << "Image failed to load...";
    }

#ifdef VISUAL
    cv::namedWindow(W_NAME, cv::WINDOW_NORMAL);
    //cv::imshow(W_NAME, image);
    //cv::waitKey(5000); // Wait for 5 seconds
#endif

    cv::Mat hsv;
    cv::cvtColor(image, hsv, CV_BGR2HSV);

#ifdef VISUAL
    //cv::imshow(W_NAME, hsv);
    //cv::waitKey(5000); // Wait for 5 seconds
#endif

    std::vector<cv::Mat> channels;
    cv::split(hsv, channels);
    cv::Mat hue = channels[0];
    cv::Mat thresh;
    cv::threshold(hue, thresh, 22, 255, cv::THRESH_BINARY_INV);
    std::vector<cv::KeyPoint> keypoints;
    blob_detect->detect(thresh, keypoints);
    LOG(INFO) << "Bison found: " << keypoints.size();

#ifdef VISUAL
    //cv::imshow(W_NAME, thresh);
    //cv::waitKey(5000); // Wait for 5 seconds
#endif

    cv::Mat hsv_keypoints;
    drawKeypoints(thresh, keypoints, hsv_keypoints, cv::Scalar::all(-1), cv::DrawMatchesFlags::DEFAULT);

#ifdef VISUAL
    cv::imshow(W_NAME, hsv_keypoints);
    cv::waitKey(10000); // Wait for 5 seconds
#endif

#ifdef VISUAL
    cv::destroyWindow(W_NAME);
#endif
    return 0;
}
