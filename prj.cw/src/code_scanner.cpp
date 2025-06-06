#include "code_scanner.h"

CodeScanner::CodeScanner() {
    detector = cv::QRCodeDetector();
}

// Восстановленный оригинальный метод для видео
bool CodeScanner::detectAndDecode(const cv::Mat& image,
    std::string& text,
    std::vector<cv::Point2f>& points) {
    if (image.empty()) return false;
    // Самое простое: декодируем и сразу рисуем
    text = detector.detectAndDecode(image, points);
    return !text.empty() && points.size() == 4;
}

// Упрощённый, но надёжный метод для фото
bool CodeScanner::detectPhoto(const cv::Mat& image,
    std::vector<cv::Point2f>& points) {
    if (image.empty()) return false;
    points.clear();
    // detect — возвращает true, если найдены ВСЕ четыре угла QR
    return detector.detect(image, points) && points.size() == 4;
}

void CodeScanner::drawQRCode(cv::Mat& image,
    const std::vector<cv::Point2f>& points) {
    if (points.size() != 4 || image.empty()) return;
    std::vector<cv::Point> pts;
    for (auto& p : points)
        pts.emplace_back(cv::Point((int)p.x, (int)p.y));

    // Толстая зелёная рамка
    cv::polylines(image, pts, true, cv::Scalar(0, 255, 0), 4);
}
