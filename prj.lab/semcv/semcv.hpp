#pragma once
#ifndef POZOYAN_SEMCV_2210021
#define POZOYAN_SEMCV_2210021

#include <string>
#include <filesystem>
#include <vector>

#include <opencv2/core.hpp>

std::string strid_from_mat(const cv::Mat& img, const int n = 4);

cv::Mat create_greyscale_img();
// i guess lecturer prefers such type of returning the result
cv::Mat gamma_correction(const cv::Mat& img, double gamma = 1.0f);

std::vector<std::filesystem::path> get_list_of_file_paths(const std::filesystem::path& path_lst);

cv::Mat gen_tgtimg00(const int lev0, const int lev1, const int lev2);
cv::Mat add_noise_gau(const cv::Mat& img, const int std);

void create_test_images(const std::string& path);

cv::Mat autocontrast(const cv::Mat& img, const double q_black, const double q_white);
#endif