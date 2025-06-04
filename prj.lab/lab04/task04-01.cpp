#include <opencv2/opencv.hpp>
#include <chrono>
#include <iostream>
#include <random>

struct Config {
    int rows = 4;
    int cols = 4;
    int cell = 256;
    int margin = 32;
    cv::Scalar bg_color{ 40, 40, 40 };
    cv::Scalar noise_mean{ 0, 0, 0 };
    cv::Scalar noise_std{ 8, 8, 8 };
    std::pair<int, int> axes_rng{ 30, 80 };
    std::pair<int, int> blur_rng{ 1, 4 };
    std::pair<int, int> brightness{ 180, 255 };
    unsigned seed = 0;
};

namespace config_io {

    void saveDefault(const std::string& path) {
        cv::FileStorage fs(path, cv::FileStorage::WRITE | cv::FileStorage::FORMAT_YAML);
        fs << "rows" << 4 << "cols" << 4 << "cell" << 256 << "margin" << 32;
        fs << "bg" << 40 << "noise_mean" << 0 << "noise_std" << 8;
        fs << "axes_min" << 30 << "axes_max" << 80;
        fs << "blur_min" << 1 << "blur_max" << 4;
        fs << "brightness_min" << 180 << "brightness_max" << 255;
        fs << "seed" << 0;
        std::cout << "Default config written to: " << path << std::endl;
    }

    Config load(const std::string& path) {
        Config cfg;
        cv::FileStorage fs(path, cv::FileStorage::READ);
        if (!fs.isOpened())
            throw std::runtime_error("Failed to open config file: " + path);

        int bg, nm, ns, tmp_seed;
        fs["rows"] >> cfg.rows;
        fs["cols"] >> cfg.cols;
        fs["cell"] >> cfg.cell;
        fs["margin"] >> cfg.margin;

        fs["bg"] >> bg;
        cfg.bg_color = cv::Scalar(bg, bg, bg);

        fs["noise_mean"] >> nm;
        fs["noise_std"] >> ns;
        cfg.noise_mean = cv::Scalar(nm, nm, nm);
        cfg.noise_std = cv::Scalar(ns, ns, ns);

        fs["axes_min"] >> cfg.axes_rng.first;
        fs["axes_max"] >> cfg.axes_rng.second;

        fs["blur_min"] >> cfg.blur_rng.first;
        fs["blur_max"] >> cfg.blur_rng.second;

        fs["brightness_min"] >> cfg.brightness.first;
        fs["brightness_max"] >> cfg.brightness.second;

        fs["seed"] >> tmp_seed;
        cfg.seed = static_cast<unsigned>(tmp_seed);

        return cfg;
    }

}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: task04-01 <config.yml> [<out_image> <out_gt> [seed]]\n";
        std::cerr << "If only <config.yml> is given, saves default config file." << std::endl;
        return 1;
    }

    const std::string cfgPath = argv[1];

    if (argc == 2) {
        config_io::saveDefault(cfgPath);
        return 0;
    }

    if (argc < 4) {
        std::cerr << "Error: missing output paths for image and ground truth." << std::endl;
        return 2;
    }

    std::string imgPath = argv[2];
    std::string gtPath = argv[3];

    unsigned seed_arg = (argc >= 5) ? static_cast<unsigned>(std::stoul(argv[4])) : 0;

    Config cfg = config_io::load(cfgPath);
    if (seed_arg)
        cfg.seed = seed_arg;
    if (cfg.seed == 0)
        cfg.seed = static_cast<unsigned>(
            std::chrono::high_resolution_clock::now().time_since_epoch().count());

    cv::RNG rng(cfg.seed);

    const int width = cfg.cols * cfg.cell;
    const int height = cfg.rows * cfg.cell;

    cv::Mat img(height, width, CV_8UC1, cfg.bg_color[0]);
    cv::Mat gt = cv::Mat::zeros(height, width, CV_8UC1);

    for (int row = 0; row < cfg.rows; ++row) {
        for (int col = 0; col < cfg.cols; ++col) {
            cv::Point2i origin(col * cfg.cell, row * cfg.cell);
            cv::Point2i center = origin + cv::Point2i(cfg.cell / 2, cfg.cell / 2);

            int ax1 = rng.uniform(cfg.axes_rng.first, cfg.axes_rng.second);
            int ax2 = rng.uniform(cfg.axes_rng.first, cfg.axes_rng.second);
            double angle = rng.uniform(0.0, 180.0);

            int dx = rng.uniform(-(cfg.cell / 2 - cfg.margin), cfg.cell / 2 - cfg.margin);
            int dy = rng.uniform(-(cfg.cell / 2 - cfg.margin), cfg.cell / 2 - cfg.margin);

            cv::Point2i objCenter = center + cv::Point2i(dx, dy);
            int brightness = cfg.brightness.first +
                rng.uniform(0, cfg.brightness.second - cfg.brightness.first);

            cv::Mat temp = cv::Mat::zeros(height, width, CV_8UC1);
            cv::ellipse(temp, objCenter, cv::Size(ax1, ax2), angle, 0, 360,
                cv::Scalar(255), cv::FILLED, cv::LINE_AA);

            int sigma = rng.uniform(cfg.blur_rng.first, cfg.blur_rng.second);
            if (sigma > 0)
                cv::GaussianBlur(temp, temp, cv::Size(), sigma, sigma);

            img.setTo(brightness, temp);
            gt |= temp;
        }
    }

    cv::Mat noise(img.size(), CV_8SC1);
    rng.fill(noise, cv::RNG::NORMAL, cfg.noise_mean[0], cfg.noise_std[0]);

    cv::Mat finalImage;
    cv::add(img, noise, finalImage, cv::noArray(), CV_8UC1);

    cv::imwrite(imgPath, finalImage);
    cv::imwrite(gtPath, gt);

    std::cout << "Generated image: " << imgPath
        << "\nGround truth:   " << gtPath
        << "\nSeed:           " << cfg.seed << std::endl;

    return 0;
}
