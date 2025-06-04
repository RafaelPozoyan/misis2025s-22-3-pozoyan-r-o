#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>

#include "semcv.hpp"
#include <sstream>
#include <iomanip>
#include <string>
#include <vector>
#include <fstream>
#include <filesystem>
#include <cmath>

std::string strid_from_mat(const cv::Mat& img, const int n) {
    int width = img.cols;
    int height = img.rows;
    int channels = img.channels();
    int depth = img.depth();

    std::ostringstream buffer;
    buffer << std::setw(n) << std::setfill('0') << width << 'x';
    buffer << std::setw(n) << std::setfill('0') << height << '.';
    buffer << channels << '.';

    switch (depth) {
    case CV_8U:  buffer << "uint08"; break;
    case CV_8S:  buffer << "sint08"; break;
    case CV_16U: buffer << "uint16"; break;
    case CV_16S: buffer << "sint16"; break;
    case CV_32S: buffer << "sint32"; break;
    case CV_32F: buffer << "real32"; break;
    case CV_64F: buffer << "real64"; break;
    default: buffer << "unknown"; break;
    }
    //buffer << "\\";

    return buffer.str();
}

void create_test_images(const std::string& path) {
    const int img_dim = 100;
    const int channels_dim = 3;

    std::filesystem::create_directories(path);

    std::vector<int> types = { CV_8U, CV_8S, CV_16U, CV_16S, CV_32S, CV_32F, CV_64F };
    std::vector<std::string> formats = { "png", "tiff", "jpg" };

    std::vector<std::string> filenames;
    for (size_t i = 0; i < types.size(); i++) {
        cv::Mat img(img_dim, img_dim, CV_MAKETYPE(types[i], channels_dim)); // 3-канальное изображение 100x100

        cv::randu(img, cv::Scalar::all(0), cv::Scalar::all(255));

        for (const auto& fmt : formats) {
            std::string filename = strid_from_mat(img) + "." + fmt;
            std::string filepath = path + "/" + filename;
            cv::imwrite(filepath, img);
            std::cout << "Saved: " << filepath << std::endl;
            filenames.push_back(filename);
        }
    }

    std::ofstream listFile(path + "/task01.lst");
    if (listFile.is_open()) {
        for (const auto& name : filenames) {
            listFile << name << "\n";
        }
        listFile.close();
    }
    else {
        std::cerr << "Error when creating task01.lst" << std::endl;
    }
}

cv::Mat create_greyscale_img() {
    const int width = 768;
    const int height = 30;
    const int channels = 1;
    const int numStripes = 256;

    cv::Mat stripeRow(1, numStripes, CV_8UC1);
    for (int i = 0; i < numStripes; ++i) {
        stripeRow.at<uchar>(0, i) = static_cast<uchar>(i);
    }

    cv::Mat stripeRowResized;
    cv::resize(stripeRow, stripeRowResized, cv::Size(width, 1), 0, 0, cv::INTER_NEAREST);

    cv::Mat img;
    cv::repeat(stripeRowResized, height, 1, img);

    // сделать экспорт или возврат
    return img;
}

cv::Mat gamma_correction(const cv::Mat& src, double gamma) {
    cv::Mat lut(1, 256, CV_8UC1);
    for (int i = 0; i < 256; ++i) {
        lut.at<uchar>(i) = cv::saturate_cast<uchar>(std::pow(i / 255.0, gamma) * 255.0);
    }

    cv::Mat dst;
    cv::LUT(src, lut, dst);

    return dst;

}

// backgroud - square - circle
cv::Mat gen_tgtimg00(const int lev0, const int lev1, const int lev2) {
    const int img_size = 256;
    const int square_size = 209;
    const int circle_radius = 83;

    const int offset = (img_size - square_size) / 2;
    cv::Point top_left(offset, offset);
    cv::Point bottom_right(offset + square_size, offset + square_size);

    cv::Scalar background_intensity(lev0);
    cv::Scalar square_intensity(lev1);
    cv::Scalar circle_intensity(lev2);

    cv::Mat img(img_size, img_size, CV_8UC1, background_intensity);

    cv::rectangle(img, top_left, bottom_right, square_intensity, cv::FILLED);

    cv::Point square_center(img_size / 2, img_size / 2);

    cv::circle(img, square_center, circle_radius, circle_intensity, cv::FILLED);

    return img;
}

cv::Mat add_noise_gau(const cv::Mat& img, const int std) {
    CV_Assert(img.type() == CV_8UC1);

    cv::Mat float_img;
    img.convertTo(float_img, CV_32F);

    cv::Mat noise_map(float_img.size(), float_img.type());
    cv::randn(noise_map, 0, std);

    cv::Mat noise_float = float_img + noise_map;

    cv::max(noise_float, 0.0, noise_float);
    cv::min(noise_float, 255.0, noise_float);

    cv::Mat res;
    noise_float.convertTo(res, CV_8UC1);
    return res;
}

std::vector<std::filesystem::path> get_list_of_file_paths(const std::filesystem::path& path_lst) {
    std::vector<std::filesystem::path> file_paths;
    std::ifstream file(path_lst);

    if (!file.is_open()) {
        std::cerr << "get_list_of_file_paths Error: Unable to open file " << path_lst << std::endl;
        return file_paths;
    }

    std::string line;
    std::filesystem::path base = path_lst.parent_path();
    while (std::getline(file, line)) {
        if (!line.empty()) {
            file_paths.emplace_back(base / line);
        }
    }
    file.close();
    return file_paths;
}


cv::Mat autocontrast(const cv::Mat& img, const double q_black, const double q_white) {
    //CV_Assert(!img.empty() && img.type() == CV_8UC1);
    cv::Mat result;

    if (img.channels() == 3) {
        std::vector<cv::Mat> channels;
        cv::split(img, channels);

        for (auto& channel : channels) {
            channel = autocontrast(channel, q_black, q_white);
        }

        cv::merge(channels, result);
    }
    else {

        cv::Mat hist;
        const int channels[] = { 0 };
        const int histSize[] = { 256 };
        const float range[] = { 0, 256 };
        const float* ranges[] = { range };

        cv::calcHist(
            &img,
            1,
            channels,
            cv::Mat(),
            hist,
            1,
            histSize,
            ranges,
            true,
            false
        );

        const int total_pixels = img.rows * img.cols;
        double sum = 0;
        int v_min = 0, v_max = 255;

        for (int i = 0; i < 256; ++i) {
            sum += hist.at<float>(i);
            if (sum >= q_black * total_pixels) {
                v_min = i;
                break;
            }
        }

        sum = 0;
        for (int i = 0; i < 256; ++i) {
            sum += hist.at<float>(i);
            if (sum >= q_white * total_pixels) {
                v_max = i;
                break;
            }
        }

        if (v_min >= v_max) return img.clone();


        const double scale = 255.0 / (v_max - v_min);
        img.convertTo(result, CV_8UC1, scale, -v_min * scale);
        return result;
    }
}

cv::Mat autocontrast_rgb(const cv::Mat& img, double q_black, double q_white) {
    CV_Assert(!img.empty() && img.type() == CV_8UC3);

    std::vector<cv::Mat> channels;
    cv::split(img, channels);

    // Находим общие квантили для всех каналов
    int global_vmin = 255;
    int global_vmax = 0;

    for (const auto& ch : channels) {
        cv::Mat hist;
        const int histSize[] = { 256 };
        const float range[] = { 0, 256 };
        const float* ranges[] = { range };

        cv::calcHist(&ch, 1, 0, cv::Mat(), hist, 1, histSize, ranges);

        const int total = ch.rows * ch.cols;
        double sum = 0;
        int vmin = 0, vmax = 255;

        // Поиск vmin
        for (int i = 0; i < 256; ++i) {
            sum += hist.at<float>(i);
            if (sum >= q_black * total) {
                vmin = i;
                break;
            }
        }

        // Поиск vmax
        sum = 0;
        for (int i = 0; i < 256; ++i) {
            sum += hist.at<float>(i);
            if (sum >= q_white * total) {
                vmax = i;
                break;
            }
        }

        global_vmin = std::min(global_vmin, vmin);
        global_vmax = std::max(global_vmax, vmax);
    }

    if (global_vmin >= global_vmax) return img.clone();

    // Применяем общие значения ко всем каналам
    const double scale = 255.0 / (global_vmax - global_vmin);
    cv::Mat result;
    img.convertTo(result, CV_8UC3, scale, -global_vmin * scale);

    return result;
}

