#pragma once
#include <opencv2/opencv.hpp>
#include <vector>
#include <string>

class CodeScanner {
private:
    cv::QRCodeDetector detector;

public:
    CodeScanner();

    // Детекция + декодирование для фото и видео
    bool detectAndDecode(const cv::Mat& image,
        std::string& text,
        std::vector<cv::Point2f>& points);

    // Только детекция для фото (опционально)
    bool detectPhoto(const cv::Mat& image,
        std::vector<cv::Point2f>& points);

    // Отрисовка рамки
    void drawQRCode(cv::Mat& image,
        const std::vector<cv::Point2f>& points);
};
