#include <opencv2/opencv.hpp>
#include "EdgeResamplingGeneratorImpl.h"
using namespace cv;
int main() {
    cv::Mat src_image = cv::imread("D:\\code\\c++\\EdgeResamplingGenerator\\src\\origin_image_scratch.png");
    cv::Mat shape_mask = cv::imread("D:\\code\\c++\\EdgeResamplingGenerator\\src\\shape_mask_scratch.png");
    /*cv::Mat src_image = cv::imread("D:\\code\\c++\\EdgeResamplingGenerator\\src\\src.png");
    cv::Mat shape_mask = cv::imread("D:\\code\\c++\\EdgeResamplingGenerator\\src\\mask.png");*/
    if (src_image.empty() || shape_mask.empty()) {
        std::cerr << "Error: Could not read input images." << std::endl;
        return -1;
    }

    if (src_image.channels() > 1) {
        cv::cvtColor(src_image, src_image, cv::COLOR_BGR2GRAY);
    }

    if (shape_mask.channels() > 1) {
        cv::cvtColor(shape_mask, shape_mask, cv::COLOR_BGR2GRAY);
    }

    /*int dilation_size = 2;  
    cv::Mat element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(2 * dilation_size + 1, 2 * dilation_size + 1));
    cv::erode(shape_mask, shape_mask, element);*/

    EdgeResamplingGeneratorImpl generator = EdgeResamplingGeneratorImpl(src_image, shape_mask);
    generator.getResult();
    std::vector<cv::Mat> result = generator.getResamplingImgs();

    return 0;
}