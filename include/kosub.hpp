#ifndef KOSUB_H
#define KOSUB_H

#include "bsub.hpp"

#include <cmath>

/**
 * Ko_2008_Background background subtraction class.
 */
class KOSub : public BSub {
public:
    KOSub(
        const int rows,
        const int cols,
        const int colors = 256,
        const int radius = 5);
    ~KOSub();
    void operator()(cv::InputArray image, cv::OutputArray fgmask, double learning_rate);
    void apply(cv::InputArray image, cv::OutputArray fgmask, double learning_rate = 0);
    void getBackgroundImage(cv::OutputArray background_image) const;

private:
    const int max_colors = 256;

    int rows;
    int cols;
    int colors;
    unsigned int radius;

    float color_reduction;
    float color_expansion;

    cv::Mat *model;
    cv::Mat *diff;
    cv::Mat *background_image;

    inline float densityNeighborhood(cv::Mat &image, const int row, const int col, const int color, const int radius) {
        const unsigned int row_start = std::max(row - radius, 0);
        const unsigned int col_start = std::max(col - radius, 0);
        const unsigned int row_end = std::min(row + radius, image.rows);
        const unsigned int col_end = std::min(col + radius, image.cols);

        float density = 0;
        for (int r = row_start; r < row_end; r++) {
            unsigned char *p = image.ptr<unsigned char>(r);
            for (int c = col_start; c < col_end; c++) {
                density += (color == floor(this->color_reduction * p[c]));
            }
        }
        return density / (radius * radius * 4);
    }
};

#endif //KOSUB_H

