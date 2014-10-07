#include "gtest/gtest.h"
#include "bsub.hpp"

TEST(BSubTest, InitialBackgroundIsEmpty) {
    BSub *subtractor = new BSub();
    cv::Mat output_image;
    subtractor->getBackgroundImage(output_image);
    ASSERT_EQ(true, output_image.empty());
}

TEST(BSubTest, FirstBackgroundIsFirstInput) {
    BSub *subtractor = new BSub();
    cv::Mat input_image = cv::Mat::ones(10, 10, CV_8U);
    cv::Mat output_image;
    subtractor->apply(input_image, cv::noArray());
    subtractor->getBackgroundImage(output_image);

    ASSERT_EQ(input_image.rows, output_image.rows);
    ASSERT_EQ(input_image.cols, output_image.cols);
    for (size_t r = 0; r < input_image.rows; r++) {
        for (size_t c = 0; c < input_image.cols; c++) {
            ASSERT_EQ(input_image.at<unsigned char>(r, c), output_image.at<unsigned char>(r, c));
        }
    }
}

TEST(BSubTest, TestApply) {
    BSub *subtractor = new BSub();
    cv::Mat input_image = cv::Mat::ones(10, 10, CV_8U);
    cv::Mat output_image;
    subtractor->apply(input_image, output_image);
    ASSERT_EQ(false, output_image.empty());
    delete subtractor;
}

TEST(BSubTest, TestOperator) {
    BSub *subtractor = new BSub();
    cv::Mat input_image = cv::Mat::ones(10, 10, CV_8U);
    cv::Mat output_image;
    subtractor->operator()(input_image, output_image);
    ASSERT_EQ(false, output_image.empty());
    delete subtractor;
}

