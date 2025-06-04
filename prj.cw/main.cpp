#include <iostream>

#include "lib/include/dither/dither.hpp"

#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>

int main() {
	cv::Mat img = cv::imread("../prj.cw/data/road_crossing.jpg", 0);
	cv::Mat trh;
	DitherThreshold(img, trh, 128);
	cv::imwrite("../prj.cw/data/threshold.jpg", trh);
	cv::Mat white;
	DitherWhiteNoise(img, white);
	cv::imwrite("../prj.cw/data/white_noise.jpg", white);
	return 0;
}