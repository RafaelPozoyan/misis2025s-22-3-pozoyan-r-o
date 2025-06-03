#include <opencv2/imgcodecs.hpp>
#include <iostream>
#include "semcv/semcv.hpp"

std::string check_file_format(const std::filesystem::path& file_path) {
    cv::Mat img = cv::imread(file_path.string(), cv::IMREAD_UNCHANGED);
    if (img.empty()) {
        return "Error: unsupported format";
    }

    std::string expected = strid_from_mat(img, 4);
    std::string actual = file_path.stem().string();

    if (actual == expected) {
        return "good";
    }
    else {
        return "bad, should be " + expected + " but found: " + actual;
    }
}

int main(int argc, char* argv[]) {

    if (argc < 2) {
        std::cerr << "didnt receive a path to lst";
        return 1;
    }

    std::filesystem::path lst(argv[1]);
    auto paths = get_list_of_file_paths(lst);

    for (const auto& path : paths) {
        std::cout << path.string() << "\t" << check_file_format(path) << std::endl;
    }
    return 0;
}