#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <test_image_path> <result_image_path>\n";
        return 1;
    }

    const int square_size = 127;
    const int r = 40;
    const cv::Size image_size(square_size * 3, square_size * 2);
    const std::vector<int> intensities{ 235, 127, 0 };

    cv::Mat test_image(image_size, CV_8UC1, cv::Scalar(0));

    for (int i = 0; i < 2; ++i) {
        for (int j = 0; j < 3; ++j) {
            const int square_value = intensities[j];
            int circle_value = intensities[i];

            if (i == 1 & j == 1) {
                circle_value = intensities[2];
            }

            if (i == 0 & j == 0) {
                circle_value = intensities[2];
            }

            cv::Rect roi(j * square_size, i * square_size, square_size, square_size);
            test_image(roi).setTo(square_value);

            cv::Point center(
                j * square_size + square_size / 2,
                i * square_size + square_size / 2
            );

            cv::circle(test_image, center, r, circle_value, -1);
        }
    }

    cv::imwrite(argv[1], test_image);

    cv::Mat I1, I2;
    cv::Mat kernel1 = cv::Mat::zeros(3, 3, CV_32F);
    kernel1.at<float>(0, 0) = 1; kernel1.at<float>(0, 1) = 2; kernel1.at<float>(0, 2) = 1;
    kernel1.at<float>(2, 0) = -1; kernel1.at<float>(2, 1) = -2; kernel1.at<float>(2, 2) = -1;

    cv::Mat kernel2 = cv::Mat::zeros(3, 3, CV_32F);
    kernel2.at<float>(0, 0) = -1; kernel2.at<float>(0, 1) = -2; kernel2.at<float>(0, 2) = -1;
    kernel2.at<float>(2, 0) = 1; kernel2.at<float>(2, 1) = 2; kernel2.at<float>(2, 2) = 1;


    cv::filter2D(test_image, I1, CV_32F, kernel1);
    cv::filter2D(test_image, I2, CV_32F, kernel2);

    cv::Mat I3;
    cv::magnitude(I1, I2, I3);

    cv::Mat V1, V2, V3;
    cv::normalize(I1, V1, 0, 255, cv::NORM_MINMAX, CV_8UC1);
    cv::normalize(I2, V2, 0, 255, cv::NORM_MINMAX, CV_8UC1);
    cv::normalize(I3, V3, 0, 255, cv::NORM_MINMAX, CV_8UC1);

    std::vector<cv::Mat> rgb_channels;
    rgb_channels.push_back(V1);
    rgb_channels.push_back(V2);
    rgb_channels.push_back(V3);
    cv::Mat V4;
    cv::merge(rgb_channels, V4);

    int image_width = test_image.cols;
    int image_height = test_image.rows;

    cv::Mat collage;
    collage.create(image_height * 2, image_width * 2, V4.type());

    cv::Mat V1_bgr, V2_bgr, V3_bgr;
    cv::cvtColor(V1, V1_bgr, cv::COLOR_GRAY2BGR);
    cv::cvtColor(V2, V2_bgr, cv::COLOR_GRAY2BGR);
    cv::cvtColor(V3, V3_bgr, cv::COLOR_GRAY2BGR);

    cv::Rect top_left(0, 0, image_width, image_height);
    cv::Rect top_right(image_width, 0, image_width, image_height);
    cv::Rect bottom_left(0, image_height, image_width, image_height);
    cv::Rect bottom_right(image_width, image_height, image_width, image_height);

    V1_bgr.copyTo(collage(top_left));
    V2_bgr.copyTo(collage(top_right));
    V3_bgr.copyTo(collage(bottom_left));
    V4.copyTo(collage(bottom_right));

    cv::imwrite(argv[2], collage);
    return 0;

}
