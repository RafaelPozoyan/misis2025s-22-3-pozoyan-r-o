#include <opencv2/opencv.hpp>

class ImageProcessor {
public:
    ImageProcessor();
    cv::Mat preprocessImage(const cv::Mat& input_image);

    cv::Mat enhanceContrast(const cv::Mat& input_image);
};
