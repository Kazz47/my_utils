#include <fstream>
#include <sstream>

#include <glog/logging.h>
#include <opencv2/core/core.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

// Name of the main program window
static const std::string W_NAME = "WINDOW";

cv::Mat original_image, mask;
cv::Mat lum, red, blu;

int min_slider = 132;
int max_slider = 141;

// Drawing vars
bool drawing = false;
std::vector<cv::Rect*> boxes;
int box_size = 10;
cv::Scalar box_color(0,0,255);

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

void draw() {
    cv::Mat temp_mask, masked;
    cv::cvtColor(mask, temp_mask, CV_GRAY2BGR);
    for (size_t i = 0; i < boxes.size(); i++) {
        cv::rectangle(temp_mask, *(boxes[i]), box_color, box_size);
    }
    masked = original_image*0.5 + temp_mask*0.5;
    cv::imshow(W_NAME, masked);
}

void onMouseCallback(int event, int x, int y, int flags, void *) {
    if (event == cv::EVENT_LBUTTONDOWN) {
        drawing = true;
        boxes.push_back(new cv::Rect(x,y,0,0));
    } else if (event == cv::EVENT_LBUTTONUP) {
        cv::Rect *box = boxes.back();
        drawing = false;
        if (box->width <= 0) {
            box->x += box->width;
            box->width *= -1;
        }
        if (box->height <= 0) {
            box->y += box->height;
            box->height *= -1;
        }

        draw();
    } else if (event == cv::EVENT_MOUSEMOVE) {
        if (drawing) {
            cv::Rect *box = boxes.back();
            box->width = x - box->x;
            box->height = y - box->y;

            draw();
        }
    }
}

void onSlider(int slider_value, void*) {
    cv::Mat max_mask = red <= max_slider;
    cv::Mat min_mask = red >= min_slider;
    cv::bitwise_and(max_mask, min_mask, mask);

    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5,5), cv::Point(0,0));
    cv::morphologyEx(mask, mask, cv::MORPH_OPEN, kernel);
    cv::morphologyEx(mask, mask, cv::MORPH_CLOSE, kernel);

    draw();
}

void cropImage(int, void*) {
    //Mask image
    cv::Mat masked;
    original_image.copyTo(masked, mask);
    for (size_t i = 0; i < boxes.size(); i++) {
        cv::rectangle(masked, *(boxes[i]), cv::Scalar(0,0,0), CV_FILLED);
    }
    imwrite("masked.jpg", masked);

    // Loop to extract training data from the bison mask
    long long unsigned int inc = 0;
    for (int r = 0; r < original_image.rows-33; r++) {
        for (int c = 0; c < original_image.cols-33; c++) {
            cv::Vec3f pixel = masked.at<cv::Vec3b>(r+16,c+16);
            uchar blue = pixel[0];
            uchar green = pixel[1];
            uchar red = pixel[2];
            if (blue > 0 || green > 0 || red > 0) {
                cv::Rect crop_rect = cv::Rect(c, r, 32, 32);
                std::stringstream ss;
                ss << "Cropping " << crop_rect;
                cv::displayStatusBar(W_NAME, ss.str());
                cv::Mat crop_image(32, 32, CV_8UC3, cv::Scalar(0, 0, 0));
                original_image(crop_rect).copyTo(crop_image);
                std::string num_images = std::to_string(inc);
                imwrite("training_data/" + num_images + ".jpg", crop_image);
                inc++;
            }
        }
    }
    std::string num_images = std::to_string(inc);
    cv::displayStatusBar(W_NAME, "Created " + num_images + " images.", 60000);
}

void toggleRects(int checked, void*) {
    if (checked) {
        box_size = 10;
        box_color = cv::Scalar(0,0,255);
    } else {
        box_size = CV_FILLED;
        box_color = cv::Scalar(0,0,0);
    }
    draw();
}

void clearRects(int, void*) {
    for (size_t i = 0; i < boxes.size(); i++) {
        delete(boxes[i]);
    }
    boxes.clear();
    draw();
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

    original_image = cv::imread(img_filename, CV_LOAD_IMAGE_COLOR);
    //cv::GaussianBlur(original_image, image, cv::Size(0, 0), 5);

    if (original_image.empty()) {
        LOG(FATAL) << "Image failed to load...";
    }

    cv::Mat ycc;
    cv::cvtColor(original_image, ycc, CV_BGR2YCrCb);

    std::vector<cv::Mat> ycc_channels;
    cv::split(ycc, ycc_channels);
    lum = ycc_channels[0];
    red = ycc_channels[1];
    blu = ycc_channels[2];

    cv::namedWindow(W_NAME, CV_GUI_EXPANDED);
    cv::setMouseCallback(W_NAME, onMouseCallback);
    cv::createButton("Show Boxes", toggleRects, NULL, CV_CHECKBOX,1);
    cv::createTrackbar("Max", W_NAME, &max_slider, 255, onSlider);
    cv::createTrackbar("Min", W_NAME, &min_slider, 255, onSlider);
    cv::createButton("Clear Rectangles", clearRects);
    cv::createButton("Run Crop", cropImage);

    onSlider(max_slider, 0);
    cv::waitKey(0);

    return 0;
}
