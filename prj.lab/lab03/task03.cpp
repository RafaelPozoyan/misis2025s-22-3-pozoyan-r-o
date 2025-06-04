#include <iostream>
#include <opencv2/opencv.hpp>
#include <semcv/semcv.hpp>

int main(int argc, char** argv) {
    if (argc != 5) {
        std::cerr << "Usage: " << argv[0] 
                  << " [naive|rgb] input.jpg q_black q_white output.jpg\n";
        return 1;
    }

    try {
        const std::string mode = argv[1];
        const cv::Mat img = cv::imread(argv[2], cv::IMREAD_ANYCOLOR);
        const double q_black = std::stod(argv[3]);
        const double q_white = std::stod(argv[4]);
        
        if (img.empty()) throw std::runtime_error("Failed to load image");
        
        cv::Mat result;
        
        //if (mode == "naive") {
        result = autocontrast(img, q_black, q_white);
        //} else if (mode == "rgb") {
        //    result = autocontrast_rgb(img, q_black, q_white);
        //} else {
        //    throw std::invalid_argument("Invalid mode");
        //}
        
        cv::imwrite(argv[4], result);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 2;
    }
    return 0;
}
