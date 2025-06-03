#include <opencv2/opencv.hpp>
#include <iostream>
#include "../prj.lab/semcv/semcv.hpp"

int main() {
    create_test_images("../resources");
    const std::filesystem::path lst = "../resources/task01.lst";
    auto paths = get_list_of_file_paths(lst);
    for (auto path : paths) {
        std::cout << "\nPath to img: ";;
        std::cout << path << "\n";
        std::cout << "\nImgs true format: ";
        cv::Mat img = cv::imread(path.string(), cv::IMREAD_UNCHANGED);
        std::string expected = strid_from_mat(img, 4);
        std::cout << expected << "\n";
    }

    return 0;
}