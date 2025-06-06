#include <opencv2/opencv.hpp>
#include <vector>
#include <string>

class CodeScanner {
private:
    cv::QRCodeDetector detector;

public:
    CodeScanner();

    bool detectAndDecode(const cv::Mat& image, std::string& text, std::vector<cv::Point2f>& points);

    bool detectPhoto(const cv::Mat& image, std::vector<cv::Point2f>& points);

    void drawQRCode(cv::Mat& image, const std::vector<cv::Point2f>& points);
};
