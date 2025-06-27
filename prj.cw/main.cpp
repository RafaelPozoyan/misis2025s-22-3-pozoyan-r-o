#include "code_scanner.h"
#include <iostream>

static void drawQuad(cv::Mat& img, const std::vector<cv::Point2f>& q)
{
    if (q.size() != 4) return;
    std::vector<std::vector<cv::Point>> poly(1);
    for (auto& p : q) poly[0].push_back(cv::Point(cvRound(p.x), cvRound(p.y)));
    cv::polylines(img, poly, true, { 0, 255, 0 }, 4);  // Увеличил толщину до 4
}

int main(int argc, char** argv)
{
    CodeScanner scanner;
    if (argc == 1) {
        cv::VideoCapture cap(0);
        if (!cap.isOpened()) {
            std::cerr << "No camera\n";
            return 1;
        }

        while (true) {
            cv::Mat f;
            cap >> f;
            if (f.empty()) break;

            auto r = scanner.detectAndDecode(f);
            if (r.ok) {
                drawQuad(f, r.quad);
                cv::putText(f, r.text, { 10, 30 }, cv::FONT_HERSHEY_SIMPLEX, 0.7, { 0, 255, 0 }, 2);
                std::cout << "QR detected: " << r.text << "\n";
            }
            cv::imshow("QR Scanner", f);
            if (cv::waitKey(1) == 'q') break;
        }
    }
    else {
        cv::Mat img = cv::imread(argv[1]);
        if (img.empty()) {
            std::cerr << "Can't open image: " << argv[1] << "\n";
            return 1;
        }

        std::cout << "Processing: " << argv[1] << "\n";
        auto r = scanner.detectAndDecode(img);
        if (r.ok) {
            drawQuad(img, r.quad);
            cv::putText(img, r.text, { 10, 50 }, cv::FONT_HERSHEY_SIMPLEX, 1.2, { 0, 255, 0 }, 3);
            std::cout << "QR detected: " << r.text << "\n";
        }
        else {
            std::cout << "QR not detected\n";
        }
        cv::imshow("Result", img);
        cv::waitKey(0);
    }
    return 0;
}
