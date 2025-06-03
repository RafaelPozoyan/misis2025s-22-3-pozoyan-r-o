#include "include/dither/dither.hpp"

#include <iostream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <random>

#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>

std::string GetImageStrid(const cv::Mat& img, int n) {
    //   correct input?

    int width = img.cols;
    int height = img.rows;
    int channels = img.channels();
    int depth = img.depth();

    std::ostringstream buffer;
    buffer << std::setw(n) << std::setfill('0') << width << 'x';
    buffer << std::setw(n) << std::setfill('0') << height << '.';
    buffer << channels << '.';

    switch (depth) {
    case CV_8U:  buffer << "uint08"; break;
    case CV_8S:  buffer << "sint08"; break;
    case CV_16U: buffer << "uint16"; break;
    case CV_16S: buffer << "sint16"; break;
    case CV_32S: buffer << "sint32"; break;
    case CV_32F: buffer << "real32"; break;
    case CV_64F: buffer << "real64"; break;
    default: buffer << "unknown"; break;
    }
    //buffer << "\\";

    return buffer.str();
}

void DitherThreshold(const cv::Mat& src, cv::Mat& dst, int t) {
    int const kMaxValue = 255;
    int const kThresholdType = 0;
    cv::threshold(src, dst, t, kMaxValue, kThresholdType);
}

WhiteNoise WhiteNoise::generate(uint32_t xdim, uint32_t ydim) {
    WhiteNoise wn;
    wn.nx = xdim;
    wn.ny = ydim;
    wn.matrix.resize(xdim * ydim);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);

    for (uint32_t i = 0; i < xdim * ydim; ++i) {
        wn.matrix[i] = static_cast<uint8_t>(dis(gen));
    }
    return wn;
}

void DitherWhiteNoise(const cv::Mat& src, cv::Mat& dst) {
    cv::Mat gray;
    if (src.channels() == 3) {
        cv::cvtColor(src, gray, cv::COLOR_BGR2GRAY);
    }
    else {
        gray = src.clone();
    }

    WhiteNoise noise = WhiteNoise::generate(gray.cols, gray.rows);

    dst.create(gray.size(), CV_8UC1);

    int width = gray.cols;
    int height = gray.rows;

    for (int y = 0; y < height; ++y) {
        const uint8_t* gray_row = gray.ptr<uint8_t>(y);
        uint8_t* dst_row = dst.ptr<uint8_t>(y);
        for (int x = 0; x < width; ++x) {
            uint8_t pixel_val = gray_row[x];
            uint8_t noise_val = noise.matrix[y * width + x];
            dst_row[x] = (pixel_val > noise_val) ? 255 : 0;
        }
    }
}