#include <semcv/semcv.hpp>
#include <opencv2/imgcodecs.hpp>

#include <vector>
#include <numeric>
#include <iostream>

int main(int argc, char* argv[]) {

    if (argc < 2) {
        std::cerr << "task02 didnt receive a path to save img";
        return 1;
    }

    std::vector<std::tuple<int, int, int>> levels = { std::make_tuple(0,127,255),
    std::make_tuple(20,127,235), std::make_tuple(55,127,200), std::make_tuple(90,127,165) };

    std::vector<cv::Mat> test_imgs;

    for (int i = 0; i < levels.size(); ++i) {
        test_imgs.push_back(gen_tgtimg00(std::get<0>(levels[i]), std::get<1>(levels[i]),
            std::get<2>(levels[i])));
    }

    cv::Mat simple_collage;
    cv::hconcat(test_imgs, simple_collage);
    cv::imwrite("../output/gray2.png", simple_collage);

    std::vector<int> stds = { 3,7,15 };
    std::vector<cv::Mat> triplets;

    for (int std : stds) {
        std::vector<cv::Mat> noisy_imgs;
        for (cv::Mat img : test_imgs) {
            noisy_imgs.push_back(add_noise_gau(img, std));
        }
        cv::Mat collage;
        cv::hconcat(noisy_imgs, collage);
        triplets.push_back(collage);
    }

    cv::Mat result;
    cv::vconcat(triplets, result);

    std::string output_path = argv[1];
    if (!cv::imwrite(output_path, result)) {
        std::cerr << "Error could not save collage to " << output_path << std::endl;
        return 1;
    }
    std::cout << "Saved collage to " << output_path << std::endl;



    return 0;
}