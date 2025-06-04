#pragma once

#include <opencv2/core.hpp>
#include <vector>

std::string GetImageStrid(const cv::Mat& img, int n = 4);

void DitherThreshold(const cv::Mat& src, cv::Mat& dst, int t);

void DitherWhiteNoise(const cv::Mat& src, cv::Mat& dst);

class WhiteNoise {
public:
    int nx, ny;
    // one dimension for easier storage in memory
    std::vector<uint8_t> matrix;

    // fabric to get instance of a noise
    static WhiteNoise generate(uint32_t xdim, uint32_t ydim);
};