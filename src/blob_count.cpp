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

    if (argc < 2) {
        LOG(ERROR) << "Not enough arguments:";
        LOG(ERROR) << getUsage();
        LOG(ERROR) << getDesc();
        exit(1);
    }

    // Read input values
    std::string img_filename = argv[1];
    //double min_area = atoi(argv[2]);
    //double max_area = atoi(argv[3]);

    // Blob Detection Params
    cv::SimpleBlobDetector::Params bison_params;
    bison_params.minDistBetweenBlobs = 10.0f;
    bison_params.filterByInertia = false;
    bison_params.filterByConvexity = false;
    bison_params.filterByColor = false;
    bison_params.filterByCircularity = true;
    bison_params.minCircularity = 0.1; //0.1
    bison_params.maxCircularity = 0.7; //0.9
    bison_params.filterByArea = true;
    bison_params.minArea = 450; //450
    bison_params.maxArea = 9500; //9500

    cv::SimpleBlobDetector::Params calf_params;
    calf_params.minDistBetweenBlobs = 10.0f;
    calf_params.filterByInertia = false;
    calf_params.filterByConvexity = false;
    calf_params.filterByColor = false;
    calf_params.filterByCircularity = true;
    calf_params.minCircularity = 0.2; //0.2
    calf_params.maxCircularity = 0.7; //0.7
    calf_params.filterByArea = true;
    calf_params.minArea = 250; //250
    calf_params.maxArea = 1200; //1200

    cv::Ptr<cv::FeatureDetector> bison_detect = new cv::SimpleBlobDetector(bison_params);
    cv::Ptr<cv::FeatureDetector> calf_detect = new cv::SimpleBlobDetector(calf_params);
    bison_detect->create("BisonBlob");
    calf_detect->create("CalfBlob");

    LOG(INFO) << "Checking for blobs in: '" << img_filename << "'";
    cv::Mat image = cv::imread(img_filename, CV_LOAD_IMAGE_COLOR);
    //cv::GaussianBlur(image, image, cv::Size(0, 0), 5);

    if (image.empty()) {
        LOG(FATAL) << "Image failed to load...";
    }

#ifdef VISUAL
    cv::namedWindow(W_NAME, cv::WINDOW_NORMAL);
    //cv::namedWindow("Hue", cv::WINDOW_NORMAL);
    //cv::namedWindow("Sat", cv::WINDOW_NORMAL);
    //cv::namedWindow("Val", cv::WINDOW_NORMAL);
    //cv::imshow(W_NAME, image);
    //cv::waitKey(5000); // Wait for 5 seconds
#endif

    cv::Mat ycc;
    cv::cvtColor(image, ycc, CV_BGR2YCrCb);

#ifdef VISUAL
    //cv::imshow(W_NAME, hsv);
    //cv::waitKey(5000); // Wait for 5 seconds
#endif

    std::vector<cv::Mat> channels;
    cv::split(ycc, channels);
    cv::Mat lum = channels[0];
    cv::Mat red = channels[1];
    cv::Mat blu = channels[2];
#ifdef VISUAL
    //cv::imshow("Luminance", lum);
    //cv::imshow("Red Change", red);
    //cv::imshow("Blue Change", blu);
    //cv::waitKey(5000); // Wait for 5 seconds
#endif
    //cv::Mat mask = cv::Mat::zeros(sat.size(), sat.type());
    //mask = sat > 20;
    cv::Mat thresh = red > 132;
    cv::Mat calf = red > 141;
    std::vector<cv::KeyPoint> bison_keypoints;
    std::vector<cv::KeyPoint> calf_keypoints;
    bison_detect->detect(thresh, bison_keypoints);
    calf_detect->detect(calf, calf_keypoints);
    LOG(INFO) << "Total Bison found: " << bison_keypoints.size();
    LOG(INFO) << "Calves found: " << calf_keypoints.size();

#ifdef VISUAL
    //cv::imshow(W_NAME, mask);
    //cv::imshow(W_NAME, thresh);
    //cv::waitKey(5000); // Wait for 5 seconds
#endif

    cv::Mat ycc_keypoints;
    drawKeypoints(thresh, bison_keypoints, ycc_keypoints, cv::Scalar(255, 0, 0), cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
    imwrite("total_bin.jpg", ycc_keypoints);
    drawKeypoints(calf, calf_keypoints, ycc_keypoints, cv::Scalar(0, 255, 0), cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
    imwrite("calf_bin.jpg", ycc_keypoints);

#ifdef VISUAL
    cv::imshow(W_NAME, ycc_keypoints);
    cv::waitKey();
#endif

#ifdef VISUAL
    cv::destroyWindow(W_NAME);
#endif
    return 0;
}
