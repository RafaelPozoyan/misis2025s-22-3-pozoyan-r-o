#include <opencv2/opencv.hpp>
#include <iostream>

namespace detector {

    cv::Mat run(const cv::Mat& inputImage) {
        cv::Mat gray;

        if (inputImage.channels() == 3) {
            cv::cvtColor(inputImage, gray, cv::COLOR_BGR2GRAY);
        }
        else {
            gray = inputImage.clone();
        }

        cv::medianBlur(gray, gray, 3);

        cv::Mat binaryMask;
        cv::threshold(gray, binaryMask, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);

        cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5));
        cv::morphologyEx(binaryMask, binaryMask, cv::MORPH_CLOSE, kernel);

        return binaryMask;
    }

}

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cerr << "Usage: task04-02 <input_image> <output_mask>" << std::endl;
        return 1;
    }

    std::string inputPath = argv[1];
    std::string outputPath = argv[2];

    cv::Mat inputImage = cv::imread(inputPath, cv::IMREAD_UNCHANGED);
    if (inputImage.empty()) {
        std::cerr << "Failed to open image: " << inputPath << std::endl;
        return 2;
    }

    cv::Mat mask = detector::run(inputImage);

    if (!cv::imwrite(outputPath, mask)) {
        std::cerr << "Failed to write output to: " << outputPath << std::endl;
        return 3;
    }

    std::cout << "Detection mask saved to: " << outputPath << std::endl;
    return 0;
}
