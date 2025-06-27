#pragma once
#include <opencv2/opencv.hpp>
#include <vector>

class ImageProcessor {
public:
    static cv::Mat preprocess(const cv::Mat& gray);

    static std::vector<std::vector<cv::Point>> findFinderPatterns(const cv::Mat& bin);

    static bool findTimingPatterns(const cv::Mat& bin, const cv::Point2f& topLeft,
        const cv::Point2f& topRight, const cv::Point2f& bottomLeft,
        float& moduleSize);

private:
    static bool checkNestedRatio(const std::vector<cv::Point>& outer,
        const std::vector<cv::Point>& inner);

    static bool hasTimingPattern(const cv::Mat& bin,
        const std::vector<cv::Point>& outer);

    static int countTimingModules(const cv::Mat& bin, cv::Point2f start, cv::Point2f end);
};
