#include <opencv2/opencv.hpp>
#include <iostream>
#include <filesystem>
#include "code_scanner.h"

void processCamera() {
    std::cout << "=== Camera Mode ===\nPress 'q' to exit.\n";
    cv::VideoCapture cap(0);
    if (!cap.isOpened()) {
        std::cerr << "Error: cannot open camera\n";
        return;
    }
    cap.set(cv::CAP_PROP_FRAME_WIDTH, 640);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 480);

    CodeScanner scanner;
    cv::Mat frame;
    std::string decoded;
    std::vector<cv::Point2f> corners;

    while (true) {
        cap >> frame;
        if (frame.empty()) break;

        // Обрабатываем каждый кадр
        if (scanner.detectAndDecode(frame, decoded, corners)) {
            // Рисуем рамку и выводим текст
            scanner.drawQRCode(frame, corners);
            cv::putText(frame, decoded,
                corners[0] + cv::Point2f(0, -10),
                cv::FONT_HERSHEY_SIMPLEX, 0.7,
                cv::Scalar(0, 255, 0), 2);
        }

        cv::imshow("Live Camera", frame);
        if (cv::waitKey(1) == 'q') break;
    }
    cap.release();
    cv::destroyAllWindows();
}

void processImage(const std::string& path) {
    std::cout << "=== Photo Mode ===\nFile: " << path << "\n";
    cv::Mat img = cv::imread(path);
    if (img.empty()) {
        std::cerr << "Error: cannot load image\n";
        return;
    }

    CodeScanner scanner;
    std::vector<cv::Point2f> corners;

    bool found = scanner.detectPhoto(img, corners);
    if (found) {
        scanner.drawQRCode(img, corners);
        cv::imshow("QR in Photo", img);
    }
    else {
        cv::imshow("No QR Found", img);
    }
    cv::waitKey(0);
    cv::destroyAllWindows();
}

int main(int argc, char* argv[]) {
    std::cout << "🔍 QR Scanner\nOpenCV version: " << CV_VERSION << "\n";

    if (argc == 1) {
        processCamera();
    }
    else if (argc == 2) {
        std::string path = argv[1];
        if (!std::filesystem::exists(path)) {
            std::cerr << "Error: file not found: " << path << "\n";
            return -1;
        }
        processImage(path);
    }
    else {
        std::cout << "Usage:\n"
            << "  " << argv[0] << "        # camera mode\n"
            << "  " << argv[0] << " <file> # photo mode\n";
    }
    return 0;
}
