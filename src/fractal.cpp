#include <fstream>
#include <sstream>
#include <complex>

#include <glog/logging.h>
#include <opencv2/highgui/highgui.hpp>

#define VISUAL

// Name of the main program window
static const std::string W_NAME = "MANDLEBROT";

std::string getUsage() {
    std::stringstream ss;
    ss << "Usage: ";
    return ss.str();
}

std::string getDesc() {
    std::stringstream ss;
    ss << "Description: Program to generate the mandlebrot set.";
    return ss.str();
}

int main(int argc, char** argv) {
    // Log to stderr instead of to the tmp directory
    FLAGS_logtostderr = 1;
    // Initiate Google Logging
    google::InitGoogleLogging(argv[0]);

    if (argc < 1) {
        LOG(ERROR) << "Not enough arguments:";
        LOG(ERROR) << getUsage();
        LOG(ERROR) << getDesc();
        exit(1);
    }

    // Fractal Parameters
    cv::Size canvas_size(1028, 756);
    double cx_min = -2;
    double cx_max = 1;
    double cy_min = -1;
    double cy_max = 1;
    size_t max_iterations = 512;

    // Window Size
    cv::Mat fractal = cv::Mat::zeros(canvas_size, CV_8U);

    size_t x_size = fractal.cols;
    size_t y_size = fractal.rows;
    for (size_t x = 0; x < x_size; ++x) {
        for (size_t y = 0; y < y_size; ++y) {
            std::complex<double> c(cx_min + x/(x_size - 1.0) * (cx_max - cx_min), cy_min + y/(y_size - 1.0) * (cy_max - cy_min));
            std::complex<double> z = 0;

            size_t i;
            for (i = 0; i < max_iterations && std::abs(z) < 2; ++i) {
                z = z * z + c;
                //LOG(INFO) << x << "," << y << ": " << z;
            }

            if (i == max_iterations) {
                fractal.at<unsigned char>(y,x) = 255;
            } else {
                fractal.at<unsigned char>(y,x) = (i*8/static_cast<float>(max_iterations)) * 255;
            }
        }
    }

#ifdef VISUAL
    //cv::namedWindow(W_NAME, cv::WINDOW_NORMAL);
    cv::namedWindow(W_NAME);
    cv::imshow(W_NAME, fractal);
    cv::waitKey();
#endif

#ifdef VISUAL
    cv::destroyWindow(W_NAME);
#endif
    return 0;
}
