#include "gtest/gtest.h"
#include "bsub.hpp"

#include <glog/logging.h>

namespace {

class BSubTest : public testing::Test {
protected:
    BSubTest() {
        // Log to stderr
        FLAGS_logtostderr = 1;
        // Disable INFO logs
        FLAGS_minloglevel = 1;
    }

    ~BSubTest() {
        // Enable ALL logs
        FLAGS_minloglevel = 0;
    }

    BSub *subtractor;
};


TEST_F(BSubTest, InitialBackgroundIsEmpty) {
    subtractor = new BSub();
    cv::Mat output_image;
    subtractor->getBackgroundImage(output_image);
    ASSERT_EQ(true, output_image.empty());
    delete subtractor;
}

TEST_F(BSubTest, DISABLED_FirstBackgroundIsFirstInput) {
    subtractor = new BSub();
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
    delete subtractor;
}

TEST_F(BSubTest, DISABLED_TestApply) {
    subtractor = new BSub();
    cv::Mat input_image = cv::Mat::ones(10, 10, CV_8U);
    cv::Mat output_image;
    subtractor->apply(input_image, output_image);
    ASSERT_EQ(false, output_image.empty());
    delete subtractor;
}

TEST_F(BSubTest, DISABLED_TestOperator) {
    subtractor = new BSub();
    cv::Mat input_image = cv::Mat::ones(10, 10, CV_8U);
    cv::Mat output_image;
    subtractor->operator()(input_image, output_image);
    ASSERT_EQ(false, output_image.empty());
    delete subtractor;
}

} // namespace

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    google::InitGoogleLogging(argv[0]);
    return RUN_ALL_TESTS();
}
