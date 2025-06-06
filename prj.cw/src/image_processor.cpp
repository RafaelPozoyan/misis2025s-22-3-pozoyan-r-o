#include "image_processor.h"

ImageProcessor::ImageProcessor() {
}

cv::Mat ImageProcessor::preprocessImage(const cv::Mat& input_image) {
    if (input_image.empty()) return cv::Mat();

    cv::Mat result = input_image.clone();

    int target_size = 800;
    if (result.cols > target_size || result.rows > target_size) {
        double scale = (double)target_size / std::max(result.cols, result.rows);
        cv::resize(result, result, cv::Size(0, 0), scale, scale, cv::INTER_AREA);
    }
    else if (result.cols < 300 && result.rows < 300) {
        double scale = 300.0 / std::min(result.cols, result.rows);
        cv::resize(result, result, cv::Size(0, 0), scale, scale, cv::INTER_CUBIC);
    }

    return result;
}

cv::Mat ImageProcessor::enhanceContrast(const cv::Mat& input_image) {
    if (input_image.empty()) return cv::Mat();

    cv::Mat result;

    input_image.convertTo(result, -1, 2.5, 50);

    return result;
}
