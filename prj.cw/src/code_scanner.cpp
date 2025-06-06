#include "code_scanner.h"

CodeScanner::CodeScanner() {
    detector = cv::QRCodeDetector();
}


bool CodeScanner::detectAndDecode(const cv::Mat& image, std::string& text, std::vector<cv::Point2f>& points) {
    if (image.empty()) return false;

    text = detector.detectAndDecode(image, points);
    return !text.empty() && points.size() == 4;
}


bool CodeScanner::detectPhoto(const cv::Mat& image, std::vector<cv::Point2f>& points) {
    if (image.empty()) {
        return false;
    }
    points.clear();

    return detector.detect(image, points) && points.size() == 4;
}

void CodeScanner::drawQRCode(cv::Mat& image, const std::vector<cv::Point2f>& points) {
    if (points.size() != 4 || image.empty()) return;
    std::vector<cv::Point> pts;
    for (auto& p : points) {
        pts.emplace_back(cv::Point((int)p.x, (int)p.y));
    }

    cv::polylines(image, pts, true, cv::Scalar(0, 255, 0), 3);
}
