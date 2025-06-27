#pragma once
#include <opencv2/opencv.hpp>
#include <string>
#include <vector>

struct QRResult {
    bool ok = false;
    std::string text;
    std::vector<cv::Point2f> quad;
};

class CodeScanner {
public:
    QRResult detectAndDecode(const cv::Mat& bgr);

private:
    static bool chooseTriple(const std::vector<std::vector<cv::Point>>& pats,
        std::vector<cv::Point2f>& triple);
    static bool findQRBoundingBox(const cv::Mat& bin, const std::vector<cv::Point2f>& triple,
        std::vector<cv::Point2f>& quad);
    static cv::Mat cropSquare(const cv::Mat& src,
        const std::vector<cv::Point2f>& quad);
    static cv::Mat modules21(const cv::Mat& qr);
    static std::string dummyDecode(const cv::Mat& mod);
};
