#include "code_scanner.h"

CodeScanner::CodeScanner() {
    detector = cv::QRCodeDetector();
}

bool CodeScanner::detectAndDecode(const cv::Mat& image,
    std::string& text,
    std::vector<cv::Point2f>& points) {
    if (image.empty()) return false;
    text = detector.detectAndDecode(image, points);
    return !text.empty() && points.size() == 4;
}

bool CodeScanner::detectPhoto(const cv::Mat& image,
    std::vector<cv::Point2f>& points) {
    if (image.empty()) return false;
    return detector.detect(image, points) && points.size() == 4;
}

void CodeScanner::drawQRCode(cv::Mat& image,
    const std::vector<cv::Point2f>& points) {
    if (points.size() != 4 || image.empty()) return;

    std::vector<cv::Point> int_points;
    for (const auto& p : points) {
        int_points.push_back(cv::Point(static_cast<int>(p.x),
            static_cast<int>(p.y)));
    }

    cv::polylines(image, int_points, true, cv::Scalar(0, 255, 0), 4);
}
