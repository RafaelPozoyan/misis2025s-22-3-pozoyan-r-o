#include <opencv2/opencv.hpp>
#include <iostream>
#include <filesystem>
#include <chrono>
#include "code_scanner.h"

void processCamera() {
    std::cout << "\n=== Camera Mode ===\nPress 'q' to exit.\n" << std::endl;

    cv::VideoCapture cap(0);
    if (!cap.isOpened()) {
        std::cerr << "Error: cannot open camera\n";
        return;
    }
    cap.set(cv::CAP_PROP_FRAME_WIDTH, 640);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 480);

    CodeScanner scanner;
    cv::Mat frame;

    std::vector<cv::Point2f> last_corners;
    std::string last_decoded;
    auto last_detection_time = std::chrono::steady_clock::now();
    const auto display_duration = std::chrono::seconds(2);

    while (true) {
        cap >> frame;
        if (frame.empty()) break;

        std::string decoded;
        std::vector<cv::Point2f> corners;
        bool found = scanner.detectAndDecode(frame, decoded, corners);

        if (found) {
            last_corners = corners;
            last_decoded = decoded;
            last_detection_time = std::chrono::steady_clock::now();
            std::cout << "Detected URL: " << decoded << std::endl;
        }

        auto current_time = std::chrono::steady_clock::now();
        bool should_display = (current_time - last_detection_time) < display_duration;

        if (should_display && !last_corners.empty()) {
            scanner.drawQRCode(frame, last_corners);
            cv::putText(frame, last_decoded,
                cv::Point((int)last_corners[0].x, (int)last_corners[0].y - 10),
                cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 255, 0), 2);
        }

        cv::imshow("Live Camera", frame);
        if (cv::waitKey(1) == 'q') break;
    }

    cap.release();
    cv::destroyAllWindows();
}

void processImage(const std::string& path) {
    std::cout << "\n=== Photo Mode ===\nFile: " << path << "\n" << std::endl;

    cv::Mat img = cv::imread(path);
    if (img.empty()) {
        std::cerr << "Error: cannot load image\n";
        return;
    }

    CodeScanner scanner;
    std::vector<cv::Point2f> corners;
    std::string decoded_text;

    if (scanner.detectAndDecode(img, decoded_text, corners)) {
        scanner.drawQRCode(img, corners);

        cv::Point text_position(
            static_cast<int>(corners[0].x),
            static_cast<int>(corners[0].y) + 200
        );

        cv::putText(img, decoded_text, text_position,
            cv::FONT_HERSHEY_SIMPLEX, 0.9,
            cv::Scalar(0, 0, 0), 2, cv::LINE_AA);

        std::cout << "Detected URL: " << decoded_text << std::endl;
        cv::imshow("QR-code Detector", img);
    }
    else {
        std::cout << "QR-code not found" << std::endl;
        cv::imshow("No QR-code Found", img);
    }

    cv::waitKey(0);
    cv::destroyAllWindows();
}

int main(int argc, char* argv[]) {
    std::cout << "QR Scanner"
        << CV_VERSION << "\n" << std::endl;

    if (argc == 1) {
        processCamera();
    }
    else if (argc == 2) {
        std::string path = argv[1];
        if (!std::filesystem::exists(path)) {
            std::cerr << "Error: file not found: " << path << std::endl;
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
