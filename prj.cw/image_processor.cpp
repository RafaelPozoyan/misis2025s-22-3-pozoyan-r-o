#include "image_processor.h"
#include <algorithm>
#include <numeric>

cv::Mat ImageProcessor::preprocess(const cv::Mat& gray)
{
    cv::Mat claheImg;
    cv::createCLAHE(3.0, { 8,8 })->apply(gray, claheImg);

    cv::Mat blur;
    cv::GaussianBlur(claheImg, blur, { 5,5 }, 0);

    cv::Mat bin;
    cv::adaptiveThreshold(blur, bin, 255,
        cv::ADAPTIVE_THRESH_GAUSSIAN_C,
        cv::THRESH_BINARY_INV, 31, 7);
    return bin;
}

// проверяем отношение площадей ~ (7^2 : 5^2 : 3^2) ≈ 49 : 25 : 9
bool ImageProcessor::checkNestedRatio(const std::vector<cv::Point>& out,
    const std::vector<cv::Point>& in)
{
    double aOut = std::fabs(cv::contourArea(out));
    double aIn = std::fabs(cv::contourArea(in));
    if (aIn < 10 || aOut < 100) return false;
    double ratio = aOut / aIn;
    return ratio > 2.0 && ratio < 8.0;          // допускаем разброс
}

// проверяем, что через центр outer квадрата проходит ч/б/ч/б/ч «тайминг» 1:1:3:1:1
bool ImageProcessor::hasTimingPattern(const cv::Mat& bin,
    const std::vector<cv::Point>& outer)
{
    cv::Rect R = cv::boundingRect(outer);
    int cx = R.x + R.width / 2, cy = R.y + R.height / 2;
    std::vector<int> horiz, vert;

    auto sampleLine = [&](int fixed, int start, int end, bool horizontal)
        {
            std::vector<int> runs;
            if (start >= end) return runs;

            uint8_t last = bin.at<uint8_t>(horizontal ? fixed : start, horizontal ? start : fixed);
            int cnt = 0;
            for (int t = start; t <= end; ++t) {
                if ((horizontal && fixed >= 0 && fixed < bin.rows && t >= 0 && t < bin.cols) ||
                    (!horizontal && t >= 0 && t < bin.rows && fixed >= 0 && fixed < bin.cols)) {
                    uint8_t v = bin.at<uint8_t>(horizontal ? fixed : t, horizontal ? t : fixed);
                    if (v == last) ++cnt;
                    else { runs.push_back(cnt); cnt = 1; last = v; }
                }
            }
            runs.push_back(cnt);
            return runs;
        };

    horiz = sampleLine(cy, R.x, R.x + R.width - 1, true);
    vert = sampleLine(cx, R.y, R.y + R.height - 1, false);

    auto good = [&](const std::vector<int>& r) {
        if (r.size() < 5) return false;
        for (size_t i = 0; i + 4 < r.size(); ++i) {
            float u = (r[i] + r[i + 1] + r[i + 3] + r[i + 4]) * 0.25f;
            if (u < 1) continue;
            float a = r[i] / u, b = r[i + 1] / u, c = r[i + 2] / u,
                d = r[i + 3] / u, e = r[i + 4] / u;
            if (a > 0.5 && a < 1.5 && b>0.5 && b < 1.5 && c>2.0 && c < 4.0 && d>0.5 && d < 1.5 && e>0.5 && e < 1.5)
                return true;
        }
        return false;
        };
    return good(horiz) && good(vert);
}

int ImageProcessor::countTimingModules(const cv::Mat& bin, cv::Point2f start, cv::Point2f end)
{
    float dist = cv::norm(end - start);
    int steps = (int)dist;
    if (steps < 2) return 0;

    std::vector<uint8_t> values;
    for (int i = 0; i <= steps; ++i) {
        float t = (float)i / steps;
        cv::Point2f pt = start + t * (end - start);
        int x = cvRound(pt.x), y = cvRound(pt.y);
        if (x >= 0 && y >= 0 && x < bin.cols && y < bin.rows) {
            values.push_back(bin.at<uint8_t>(y, x));
        }
    }

    // Считаем переходы (должно быть ~19 для QR версии 1)
    int transitions = 0;
    for (size_t i = 1; i < values.size(); ++i) {
        if (values[i] != values[i - 1]) transitions++;
    }

    return transitions;
}

bool ImageProcessor::findTimingPatterns(const cv::Mat& bin, const cv::Point2f& topLeft,
    const cv::Point2f& topRight, const cv::Point2f& bottomLeft,
    float& moduleSize)
{
    // Проверяем timing patterns
    int hTransitions = countTimingModules(bin, topLeft + 0.3f * (topRight - topLeft),
        topRight - 0.3f * (topRight - topLeft));
    int vTransitions = countTimingModules(bin, topLeft + 0.3f * (bottomLeft - topLeft),
        bottomLeft - 0.3f * (bottomLeft - topLeft));

    if (hTransitions >= 15 && hTransitions <= 25 && vTransitions >= 15 && vTransitions <= 25) {
        // Вычисляем размер модуля
        float hDist = cv::norm(topRight - topLeft);
        float vDist = cv::norm(bottomLeft - topLeft);
        moduleSize = (hDist + vDist) / (2.0f * 21.0f); // 21 модуль для версии 1
        return true;
    }

    return false;
}

std::vector<std::vector<cv::Point>>
ImageProcessor::findFinderPatterns(const cv::Mat& bin)
{
    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy;
    cv::findContours(bin, contours, hierarchy,
        cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);

    std::vector<std::vector<cv::Point>> patterns;
    for (size_t i = 0; i < contours.size(); ++i) {
        int child = hierarchy[i][2];
        if (child < 0) continue;                       // нет вложенного контура
        int grand = hierarchy[child][2];
        if (grand < 0) continue;                       // ожидаем ещё один уровень

        std::vector<cv::Point> approxOut, approxIn;
        cv::approxPolyDP(contours[i], approxOut, 0.05 * cv::arcLength(contours[i], true), true);
        cv::approxPolyDP(contours[grand], approxIn, 0.05 * cv::arcLength(contours[grand], true), true);

        if (approxOut.size() == 4 && approxIn.size() == 4 &&
            checkNestedRatio(approxOut, approxIn) &&
            hasTimingPattern(bin, approxOut))
        {
            patterns.push_back(approxOut);
        }
    }
    return patterns;
}
