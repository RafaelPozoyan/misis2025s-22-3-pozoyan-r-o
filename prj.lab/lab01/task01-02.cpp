#include <opencv2/imgcodecs.hpp>
#include <iostream>
#include "semcv/semcv.hpp"


int main(int argc, char* argv[]) {

    if (argc < 2) {
        std::cerr << "didnt receive a path to save png" << std::endl;
        return -1;
    }

    cv::Mat base = create_greyscale_img();

    std::vector<double> gammas = { 1.0, 1.8, 2.0, 2.2, 2.4, 2.6 };
    std::vector<cv::Mat> collageImgs;

    for (double gamma : gammas) {
        cv::Mat gammaImg = gamma_correction(base, gamma);
        collageImgs.push_back(gammaImg);
    }

    cv::Mat collage;
    cv::vconcat(collageImgs, collage);

    std::string output_path = argv[1];
    if (!cv::imwrite(output_path, collage)) {
        std::cerr << "Error cant save collage to " << output_path << std::endl;
        return -1;
    }

    std::cout << "Successfully saved coolage to " << output_path << std::endl;
    return 0;
}