#include "code_scanner.h"
#include "image_processor.h"
#include <algorithm>
#include <sstream>

using cv::Point2f;

bool CodeScanner::chooseTriple(const std::vector<std::vector<cv::Point>>& pats,
    std::vector<Point2f>& triple)
{
    if (pats.size() < 3) return false;
    std::vector<std::pair<double, size_t>> ar;
    for (size_t i = 0; i < pats.size(); ++i)
        ar.push_back({ cv::contourArea(pats[i]),i });
    std::sort(ar.begin(), ar.end(), std::greater<>());
    Point2f c[3];
    for (int i = 0; i < 3; ++i) {
        cv::Moments m = cv::moments(pats[ar[i].second]);
        c[i] = { float(m.m10 / m.m00),float(m.m01 / m.m00) };
    }
    triple.assign(c, c + 3);
    return true;
}

bool CodeScanner::findQRBoundingBox(const cv::Mat& bin,
    const std::vector<cv::Point2f>& tri, std::vector<cv::Point2f>& quad)
{
    if (tri.size() != 3) return false;
    auto dist = [](Point2f a, Point2f b) {return cv::norm(a - b); };
    double d01 = dist(tri[0], tri[1]),
        d02 = dist(tri[0], tri[2]),
        d12 = dist(tri[1], tri[2]);
    Point2f tl, tr, bl;
    if (d01 > d02 && d01 > d12) {
        tl = tri[2];
        tr = (tri[0].y < tri[1].y) ? tri[0] : tri[1];
        bl = (tri[0].y > tri[1].y) ? tri[0] : tri[1];
    }
    else if (d02 > d01 && d02 > d12) {
        tl = tri[1];
        tr = (tri[0].y < tri[2].y) ? tri[0] : tri[2];
        bl = (tri[0].y > tri[2].y) ? tri[0] : tri[2];
    }
    else {
        tl = tri[0];
        tr = (tri[1].y < tri[2].y) ? tri[1] : tri[2];
        bl = (tri[1].y > tri[2].y) ? tri[1] : tri[2];
    }
    if (tr.x < bl.x) std::swap(tr, bl);
    float mSize;
    if (!ImageProcessor::findTimingPatterns(bin, tl, tr, bl, mSize)) {
        float avg = (dist(tr, tl) + dist(bl, tl)) / 2;
        mSize = avg / 21.0f;
    }
    Point2f rv = Point2f((tr.x - tl.x) / 21.0f, (tr.y - tl.y) / 21.0f);
    Point2f dv = Point2f((bl.x - tl.x) / 21.0f, (bl.y - tl.y) / 21.0f);
    Point2f rzi = rv * 4.0f + dv * 4.0f;
    Point2f realTL = tl - rzi;
    Point2f realTR = tr + (rzi - dv * 8.0f);
    Point2f realBL = bl + (rzi - rv * 8.0f);
    Point2f realBR = realTL + (realTR - realTL) + (realBL - realTL);
    quad = { realTL,realTR,realBR,realBL };
    return true;
}

cv::Mat CodeScanner::cropSquare(const cv::Mat& src,
    const std::vector<Point2f>& quad)
{
    if (quad.size() != 4) return cv::Mat();
    float w = std::max(cv::norm(quad[0] - quad[1]), cv::norm(quad[2] - quad[3]));
    float h = std::max(cv::norm(quad[0] - quad[3]), cv::norm(quad[1] - quad[2]));
    float side = std::max(w, h);
    side = std::max(side, 210.0f);
    std::vector<Point2f> dst = { {0,0},{side - 1,0},{side - 1,side - 1},{0,side - 1} };
    cv::Mat H = cv::getPerspectiveTransform(quad, dst);
    cv::Mat out;
    cv::warpPerspective(src, out, H, cv::Size((int)side, (int)side));
    return out;
}

cv::Mat CodeScanner::modules21(const cv::Mat& qr)
{
    cv::Mat g, bin;
    cv::cvtColor(qr, g, cv::COLOR_BGR2GRAY);
    cv::threshold(g, bin, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);
    const int N = 21;
    int sx = bin.cols / N, sy = bin.rows / N;
    cv::Mat M(N, N, CV_8UC1, cv::Scalar(0));
    for (int y = 0; y < N; ++y)
        for (int x = 0; x < N; ++x) {
            int cx = x * sx + sx / 2, cy = y * sy + sy / 2;
            if (cx < bin.cols && cy < bin.rows)
                M.at<uint8_t>(y, x) = bin.at<uint8_t>(cy, cx) ? 1 : 0;
        }
    return M;
}

std::string CodeScanner::dummyDecode(const cv::Mat& mod)
{
    std::ostringstream s;
    s << "QR_" << mod.rows << "x" << mod.cols;
    return s.str();
}

QRResult CodeScanner::detectAndDecode(const cv::Mat& bgr)
{
    QRResult r;
    cv::Mat gray, bin;
    cv::cvtColor(bgr, gray, cv::COLOR_BGR2GRAY);
    bin = ImageProcessor::preprocess(gray);
    auto pats = ImageProcessor::findFinderPatterns(bin);
    std::vector<Point2f> tri;
    if (!chooseTriple(pats, tri)) return r;
    std::vector<Point2f> quad;
    if (!findQRBoundingBox(bin, tri, quad)) return r;
    cv::Mat qr = cropSquare(bgr, quad);
    if (qr.empty()) return r;
    cv::Mat m = modules21(qr);
    r.ok = true; r.text = dummyDecode(m); r.quad = quad;
    return r;
}
