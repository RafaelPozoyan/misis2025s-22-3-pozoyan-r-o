#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>

cv::Mat createLoGKernel(int size, double sigma) {
    cv::Mat kernel(size, size, CV_32F);
    int center = size / 2;

    double sum = 0.0;
    for (int y = 0; y < size; y++) {
        for (int x = 0; x < size; x++) {
            int x1 = x - center;
            int y1 = y - center;
            double r_squared = x1 * x1 + y1 * y1;

            double value = -1.0 / (CV_PI * std::pow(sigma, 4)) *
                (1.0 - r_squared / (2 * sigma * sigma)) *
                std::exp(-r_squared / (2 * sigma * sigma));

            kernel.at<float>(y, x) = value;
            sum += value;
        }
    }

    for (int y = 0; y < size; y++) {
        for (int x = 0; x < size; x++) {
            kernel.at<float>(y, x) -= sum / (size * size);
        }
    }

    return kernel;
}

cv::Mat detectBlobs(const cv::Mat& img, std::vector<cv::KeyPoint>& keypoints) {
    cv::Mat result = img.clone();

    cv::Ptr<cv::SimpleBlobDetector> detector = cv::SimpleBlobDetector::create();

    cv::SimpleBlobDetector::Params params;
    params.minThreshold = 10;
    params.maxThreshold = 200;
    params.thresholdStep = 10;

    params.minArea = 20;
    params.maxArea = 5000;

    params.filterByColor = false;
    params.filterByCircularity = false;
    params.filterByConvexity = false;
    params.filterByInertia = false;

    detector = cv::SimpleBlobDetector::create(params);

    std::vector<cv::Mat> filtered_images;
    std::vector<double> sigmas = { 1.0, 2.0, 4.0, 8.0 };

    for (double sigma : sigmas) {
        int kernel_size = std::max(3, static_cast<int>(2 * std::ceil(3 * sigma) + 1));
        cv::Mat kernel = createLoGKernel(kernel_size, sigma);

        cv::Mat filtered;
        cv::filter2D(img, filtered, CV_32F, kernel);

        cv::normalize(filtered, filtered, 0, 255, cv::NORM_MINMAX);
        filtered.convertTo(filtered, CV_8UC1);

        cv::Mat local_max = filtered.clone();
        cv::dilate(filtered, local_max, cv::Mat());
        cv::Mat mask = (filtered == local_max);

        cv::threshold(filtered, filtered, 200, 255, cv::THRESH_BINARY);
        filtered.copyTo(filtered, mask);

        filtered_images.push_back(filtered);
    }

    cv::Mat combined = cv::Mat::zeros(img.size(), CV_8UC1);
    for (const auto& filtered : filtered_images) {
        cv::add(combined, filtered, combined);
    }

    detector->detect(combined, keypoints);

    cv::drawKeypoints(img, keypoints, result, cv::Scalar(0, 0, 255),
        cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);

    return result;
}

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cout << "Usage: task06 <input_image> <output_result>" << std::endl;
        return -1;
    }

    std::string input_path = argv[1];
    std::string output_path = argv[2];

    cv::Mat img = cv::imread(input_path, cv::IMREAD_GRAYSCALE);
    if (img.empty()) {
        std::cerr << "Failed to read image: " << input_path << std::endl;
        return -1;
    }

    std::vector<cv::KeyPoint> keypoints;
    cv::Mat result = detectBlobs(img, keypoints);

    cv::Mat mask = cv::Mat::zeros(img.size(), CV_8UC1);
    for (const auto& kp : keypoints) {
        cv::circle(mask, kp.pt, kp.size / 2, cv::Scalar(255), -1);
    }

    cv::imwrite(output_path, mask);

    std::cout << "Detection results saved to: " << output_path << std::endl;
    std::cout << "Detected " << keypoints.size() << " objects" << std::endl;

    return 0;
}
