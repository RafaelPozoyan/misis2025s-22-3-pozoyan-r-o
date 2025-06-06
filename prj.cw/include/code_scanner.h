#pragma once
#include <opencv2/opencv.hpp>
#include <vector>
#include <string>

class CodeScanner {
private:
    cv::QRCodeDetector detector;

public:
    CodeScanner();

    // Для видеопотока — декодирование + детекция
    bool detectAndDecode(const cv::Mat& image,
        std::string& text,
        std::vector<cv::Point2f>& points);

    // Для фото — только детекция (ищем 4 угла настоящего QR)
    bool detectPhoto(const cv::Mat& image,
        std::vector<cv::Point2f>& points);

    // Рисуем рамку по углам
    void drawQRCode(cv::Mat& image,
        const std::vector<cv::Point2f>& points);
};
