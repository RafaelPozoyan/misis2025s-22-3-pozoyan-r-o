#include "code_scanner.h"
#include <iostream>

static void drawQuad(cv::Mat& img, const std::vector<cv::Point2f>& q)
{
    if (q.size() != 4) return;
    std::vector<std::vector<cv::Point>> poly(1);
    for (auto& p : q) poly[0].push_back(cv::Point(int(p.x), int(p.y)));
    cv::polylines(img, poly, true, { 0,255,0 }, 4);
}

int main(int argc, char** argv)
{
    CodeScanner scanner;
    if (argc == 1) {
        cv::VideoCapture cap(0);
        if (!cap.isOpened())return 1;
        while (true) {
            cv::Mat f; cap >> f; if (f.empty())break;
            auto res = scanner.detectAndDecode(f);
            if (res.ok) {
                drawQuad(f, res.quad);
                cv::putText(f, res.text, { 10,30 }, cv::FONT_HERSHEY_SIMPLEX, 0.7, { 0,255,0 }, 2);
            }
            cv::imshow("QR Scanner", f);
            if (cv::waitKey(1) == 'q')break;
        }
    }
    else {
        cv::Mat img = cv::imread(argv[1]);
        if (img.empty())return 1;
        auto res = scanner.detectAndDecode(img);
        if (res.ok) {
            drawQuad(img, res.quad);
            cv::putText(img, res.text, { 10,50 }, cv::FONT_HERSHEY_SIMPLEX, 1.0, { 0,255,0 }, 2);
        }
        cv::imshow("Result", img); cv::waitKey(0);
    }
    return 0;
}
