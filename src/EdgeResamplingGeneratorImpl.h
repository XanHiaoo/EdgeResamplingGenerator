#pragma once

#include <opencv2/opencv.hpp>

class EdgeResamplingGeneratorImpl
{
private:
    cv::Mat srcImage_;      // 用于源图像的成员变量
    cv::Mat shapeMask_; // 用于形状掩模图像的成员变量
    cv::Mat imgShow_;
    cv::Mat imgShowOnMask_;
    cv::Mat labels_; //连通域标签
    cv::Mat borderLabels_;
    std::vector<cv::Mat> resamplingImgs_;


    int resamplingWidth_ = 20;
private:
    // 连通组件算法
    std::vector<std::vector<cv::Point>> applyConnectedComponents();
    std::vector<std::vector<cv::Point>> getBorderPoints(std::vector<std::vector<cv::Point>> connectedComponents);
    std::vector<std::vector<cv::Point>> getOrderBorderPoints(std::vector<std::vector<cv::Point>> borderPoints);
    std::vector<std::vector<cv::Point>> getFilteredBorderPoints(std::vector<std::vector<cv::Point>> orderBorderPoints);
    std::vector<cv::Point2f> getNormals(const cv::Point& start, const cv::Point& end, int length);
    std::vector<std::vector<std::vector<cv::Point2f>>> getAllComponentNormals(std::vector<std::vector<cv::Point>> filteredBorderPoints);
    void resamplingImgs(std::vector<std::vector<std::vector<cv::Point2f>>> allComponentNormals);

public:
    EdgeResamplingGeneratorImpl(const cv::Mat& inputImage, const cv::Mat& maskImage);
    const cv::Mat& getSrcImage() const;
    const cv::Mat& getShapeMaskImage() const;
    const std::vector<cv::Mat>& getResamplingImgs() const;
    void setSrcImage(const cv::Mat& srcImage_);
    void setShapeMaskImage(const cv::Mat& maskImage);
    void getResult();
};
