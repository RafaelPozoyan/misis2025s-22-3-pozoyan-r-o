#include "semcv/semcv.hpp"
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include <vector>
#include <iostream>
#include <exception>
#include <string>
#include <numeric>
#include <utility>
#include <fstream>


struct DistributionParams {
    std::vector<double> means;
    std::vector<double> stds;
};

class CollageStatisticsCalculator {
private:
    cv::Mat collage_;
    int collage_height_pixels_;
    int collage_width_pixels_;
    int collage_height_cells_;
    int collage_width_cells;
public:
    CollageStatisticsCalculator() = delete;
    CollageStatisticsCalculator(std::string file_path, int cell_rows = 1, int cell_cols = 1) {
        collage_ = cv::imread(file_path, cv::IMREAD_UNCHANGED);
        if (collage_.empty()) {
            std::cerr << "Error: unsupported format or wrong path";
            throw std::runtime_error("File not found" + file_path);
        }
        collage_height_pixels_ = collage_.rows;
        collage_width_pixels_ = collage_.cols;

        collage_height_cells_ = cell_rows;
        collage_width_cells = cell_cols;
    }

    const cv::Mat get_collage_part(int row, int col) {
        const int cell_pixel_height = collage_height_pixels_ / collage_height_cells_;
        const int cell_pixel_width = collage_width_pixels_ / collage_width_cells;
        cv::Rect roi(cell_pixel_width * col, cell_pixel_height * row,
            cell_pixel_width, cell_pixel_height);
        return collage_(roi);
    }

    DistributionParams const get_collage_masked_statistics(const std::vector<cv::Mat>& masks) {
        DistributionParams data;

        for (int row = 0; row < collage_height_cells_; ++row) {
            for (int col = 0; col < collage_width_cells; ++col) {
                // getting roi

                cv::Scalar mean, std;
                cv::Mat roi = get_collage_part(row, col);

                for (auto mask : masks) {
                    cv::meanStdDev(roi, mean, std, mask);
                    data.means.push_back(mean[0]);
                    data.stds.push_back(std[0]);
                }
            }
        }
        return data;
    }

};

void print_statistics_table(DistributionParams current, DistributionParams expected, const std::string& path) {

    std::ofstream file(path, std::ios::app);
    if (!file) {
        std::cerr << "Could not create md file for folder output";
        throw std::runtime_error("Cant create result file");
    }

    file << "| std_exp | mean_exp | std_cur | mean_cur |\n";
    file << "| --- | --- | --- | --- |\n";

    for (int stat_ind = 0; stat_ind < current.means.size(); ++stat_ind) {
        file << "| " << expected.stds[stat_ind]
            << " | " << expected.means[stat_ind]
            << " | " << current.stds[stat_ind]
            << " | " << current.means[stat_ind] << " |\n";
    }

    file.close();
}


std::vector<cv::Mat> get_regions_masks() {

    std::vector<cv::Mat> res = { gen_tgtimg00(1, 0, 0),
        gen_tgtimg00(0, 1, 0), gen_tgtimg00(0, 0, 1) };

    return res;
}

cv::Mat draw_img_hist(const cv::Mat& source, cv::Scalar background_color) {
    int hist_size = 256;
    float range[] = { 0,256 };
    const float* hist_range[] = { range };

    bool uniform = true, accumulate = false;

    cv::Mat hist;
    cv::calcHist(&source, 1, 0, cv::Mat(), hist, 1, &hist_size,
        hist_range, uniform, accumulate);

    int hist_w = 256, hist_h = 250;
    int bin_w = cvRound((double)hist_w / hist_size);

    cv::Mat hist_img(hist_h, hist_w, CV_8UC3, background_color);
    cv::normalize(hist, hist, 0, hist_img.rows, cv::NORM_MINMAX, -1, cv::Mat());

    for (int i = 1; i < hist_size; ++i) {
        cv::line(hist_img, cv::Point(bin_w * (i - 1), hist_h - cvRound(hist.at<float>(i - 1))),
            cv::Point(bin_w * (i), hist_h - cvRound(hist.at<float>(i))),
            cv::Scalar(0, 0, 0), 2, 8, 0);
    }

    return hist_img;
}

void save_statistics_table(std::string file_path) {
    CollageStatisticsCalculator sc(file_path, 3, 4);

    std::string md_path = "../output/lab2_statistics.md";

    std::vector<std::tuple<int, int, int>> levels = { std::make_tuple(0,127,255),
   std::make_tuple(20,127,235), std::make_tuple(55,127,200), std::make_tuple(90,127,165) };
    std::vector<int> stds = { 3,7,15 };

    DistributionParams exp;
    for (int i = 0; i < stds.size(); i++) {
        for (int j = 0; j < levels.size(); ++j) {
            exp.means.push_back(std::get<0>(levels[j]));
            exp.stds.push_back(stds[i]);
            exp.means.push_back(std::get<1>(levels[j]));
            exp.stds.push_back(stds[i]);
            exp.means.push_back(std::get<2>(levels[j]));
            exp.stds.push_back(stds[i]);
        }
    }
    DistributionParams cur = sc.get_collage_masked_statistics(get_regions_masks());

    print_statistics_table(cur, exp, md_path);
}

// will receive a path to the collage
int main(int argc, char* argv[]) {

    if (argc < 2) {
        std::cerr << "statistics02 didnt receive a path to input file";
        return 1;
    }

    std::string file_path(argv[1]);

    CollageStatisticsCalculator sc(file_path, 3, 4);
        std::vector<cv::Mat> quads;

    std::vector<cv::Scalar> colors = { cv::Scalar(100,100,100), cv::Scalar(150,150,150) };
    // sepatate parts of collage
    for (int row = 0; row < 3; ++row) {
        std::vector<cv::Mat> hist_collage_parts;
        for (int col = 0; col < 4; ++col) {
            cv::Mat hist = draw_img_hist(sc.get_collage_part(row, col), colors[(row * 4 + col) % 2]);
            hist_collage_parts.push_back(hist);
        }
        cv::Mat quad;
        cv::hconcat(hist_collage_parts, quad);
        quads.push_back(quad);
    }

    cv::Mat hist_collage;
    cv::vconcat(quads, hist_collage);

    std::string output_path = "../resources/hist2.png";
    if (!cv::imwrite(output_path, hist_collage)) {
        std::cerr << "Error could not save collage to " << output_path << std::endl;
        return 1;
    }
    std::cout << "Saved collage to " << output_path << std::endl;

    return 0;
}