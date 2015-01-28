#include <fstream>
#include <sstream>

#include <glog/logging.h>
#include <opencv2/core/core.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

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
    bison_params.maxCircularity = 0.7; //0.7
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

    cv::Mat ycc;
    cv::cvtColor(image, ycc, CV_BGR2YCrCb);

    std::vector<cv::Mat> ycc_channels;
    cv::split(ycc, ycc_channels);
    cv::Mat lum = ycc_channels[0];
    cv::Mat red = ycc_channels[1];
    cv::Mat blu = ycc_channels[2];

    cv::Mat thresh = red > 132;
    cv::Mat calf = red > 141;

    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5,5), cv::Point(0,0));
    cv::morphologyEx(thresh, thresh, cv::MORPH_OPEN, kernel);
    cv::morphologyEx(thresh, thresh, cv::MORPH_CLOSE, kernel);
    cv::morphologyEx(calf, calf, cv::MORPH_OPEN, kernel);
    cv::morphologyEx(calf, calf, cv::MORPH_CLOSE, kernel);

    cv::Mat ycc_keypoints;
    imwrite("total_bin.jpg", thresh);
    imwrite("calf_bin.jpg", calf);

    //Mask original
    cv::Mat masked;
    image.copyTo(masked, thresh);
    imwrite("masked.jpg", masked);

    // Loop to extract training data from the bison mask
    /*
    unsigned int inc = 0;
    for (int r = 0; r < image.rows-33; r++) {
        for (int c = 0; c < image.cols-33; c++) {
            cv::Vec3f pixel = masked.at<cv::Vec3b>(r+16,c+16);
            uchar blue = pixel[0];
            uchar green = pixel[1];
            uchar red = pixel[2];
            if (blue > 0 || green > 0 || red > 0) {
                cv::Rect crop_rect = cv::Rect(c, r, 32, 32);
                LOG(INFO) << crop_rect;
                cv::Mat crop_image(32, 32, CV_8UC3, cv::Scalar(0, 0, 0));
                image(crop_rect).copyTo(crop_image);
                imwrite("test/" + std::to_string(inc) + "_000.jpg", crop_image);
                cv::Mat rotation_mat = getRotationMatrix2D(cv::Point(crop_image.rows/2,crop_image.cols/2), 90, 1);
                warpAffine(crop_image, crop_image, rotation_mat, crop_image.size());
                imwrite("test/" + std::to_string(inc) + "_090.jpg", crop_image);
                warpAffine(crop_image, crop_image, rotation_mat, crop_image.size());
                imwrite("test/" + std::to_string(inc) + "_180.jpg", crop_image);
                warpAffine(crop_image, crop_image, rotation_mat, crop_image.size());
                imwrite("test/" + std::to_string(inc) + "_270.jpg", crop_image);
                inc++;
            }
        }
    }
    */

    return 0;
}
