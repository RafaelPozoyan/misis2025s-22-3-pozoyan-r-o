#include "code_scanner.h"
#include "image_processor.h"
#include <numeric>
#include <iomanip>
#include <sstream>

using cv::Point2f;

bool CodeScanner::chooseTriple(const std::vector<std::vector<cv::Point>>& pats,
    std::vector<Point2f>& triple)
{
    if (pats.size() < 3) return false;
    /* берём три самых больших */
    std::vector<std::pair<double, size_t>> ar;
    for (size_t i = 0; i < pats.size(); ++i)
        ar.push_back({ cv::contourArea(pats[i]), i });
    std::sort(ar.begin(), ar.end(), std::greater<>());

    Point2f c[3];
    for (int i = 0; i < 3; ++i) {
        cv::Moments m = cv::moments(pats[ar[i].second]);
        c[i] = { float(m.m10 / m.m00), float(m.m01 / m.m00) };
    }
    triple.assign(c, c + 3);
    return true;
}

bool CodeScanner::findQRBoundingBox(const cv::Mat& bin, const std::vector<cv::Point2f>& triple,
    std::vector<cv::Point2f>& quad)
{
    if (triple.size() != 3) return false;

    auto dist = [](Point2f a, Point2f b) { return cv::norm(a - b); };
    double d01 = dist(triple[0], triple[1]),
        d02 = dist(triple[0], triple[2]),
        d12 = dist(triple[1], triple[2]);

    Point2f tl, tr, bl;
    if (d01 > d02 && d01 > d12) {
        tl = triple[2];
        tr = (triple[0].y < triple[1].y) ? triple[0] : triple[1];
        bl = (triple[0].y > triple[1].y) ? triple[0] : triple[1];
    }
    else if (d02 > d01 && d02 > d12) {
        tl = triple[1];
        tr = (triple[0].y < triple[2].y) ? triple[0] : triple[2];
        bl = (triple[0].y > triple[2].y) ? triple[0] : triple[2];
    }
    else {
        tl = triple[0];
        tr = (triple[1].y < triple[2].y) ? triple[1] : triple[2];
        bl = (triple[1].y > triple[2].y) ? triple[1] : triple[2];
    }

    if (tr.x < bl.x) std::swap(tr, bl);

    float moduleSize;
    if (!ImageProcessor::findTimingPatterns(bin, tl, tr, bl, moduleSize)) {
        float avgDist = (cv::norm(tr - tl) + cv::norm(bl - tl)) / 2.0f;
        moduleSize = avgDist / 21.0f;
    }

    Point2f rightVec = (tr - tl) / 21.0f;
    Point2f downVec = (bl - tl) / 21.0f; 
    Point2f quietZone = rightVec * 4.0f + downVec * 4.0f;

    Point2f realTL = tl - rightVec * 4.0f - downVec * 4.0f;        
    Point2f realTR = tr + rightVec * 4.0f - downVec * 4.0f;          
    Point2f realBL = bl - rightVec * 4.0f + downVec * 4.0f;        
    Point2f realBR = tr + rightVec * 4.0f + bl - tl + downVec * 4.0f; 

    realBR = realTL + (realTR - realTL) + (realBL - realTL);

    quad = { realTL, realTR, realBR, realBL };
    return true;
}

cv::Mat CodeScanner::cropSquare(const cv::Mat& src, const std::vector<Point2f>& quad)
{
    if (quad.size() != 4) return cv::Mat();

    float width = std::max(cv::norm(quad[0] - quad[1]), cv::norm(quad[2] - quad[3]));
    float height = std::max(cv::norm(quad[0] - quad[3]), cv::norm(quad[1] - quad[2]));
    float size = std::max(width, height);
    size = std::max(size, 210.0f);

    std::vector<Point2f> dst = { {0,0}, {size - 1,0}, {size - 1,size - 1}, {0,size - 1} };
    cv::Mat H = cv::getPerspectiveTransform(quad, dst);

    cv::Mat out;
    cv::warpPerspective(src, out, H, { int(size), int(size) });
    return out;
}

cv::Mat CodeScanner::modules21(const cv::Mat& qr)
{
    cv::Mat g;
    if (qr.channels() == 3) {
        cv::cvtColor(qr, g, cv::COLOR_BGR2GRAY);
    }
    else {
        g = qr.clone();
    }

    cv::Mat bin;
    cv::threshold(g, bin, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);

    const int N = 21;
    int stepX = bin.cols / N, stepY = bin.rows / N;
    cv::Mat M(N, N, CV_8UC1, cv::Scalar(0));

    for (int y = 0; y < N; ++y) {
        for (int x = 0; x < N; ++x) {
            int cx = x * stepX + stepX / 2;
            int cy = y * stepY + stepY / 2;
            if (cx >= bin.cols || cy >= bin.rows) continue;
            M.at<uint8_t>(y, x) = bin.at<uint8_t>(cy, cx) ? 1 : 0;
        }
    }
    return M;
}

std::string CodeScanner::dummyDecode(const cv::Mat& mod)
{
    std::ostringstream s;
    s << "QR_DETECTED_" << mod.rows << "x" << mod.cols;
    return s.str();
}

QRResult CodeScanner::detectAndDecode(const cv::Mat& bgr)
{
    QRResult res;
    cv::Mat gray;
    cv::cvtColor(bgr, gray, cv::COLOR_BGR2GRAY);
    cv::Mat bin = ImageProcessor::preprocess(gray);

    auto patterns = ImageProcessor::findFinderPatterns(bin);
    std::vector<Point2f> triple;
    if (!chooseTriple(patterns, triple)) return res;

    std::vector<Point2f> quad;
    if (!findQRBoundingBox(bin, triple, quad)) return res;

    cv::Mat qr = cropSquare(bgr, quad);
    if (qr.empty()) return res;

    cv::Mat mod = modules21(qr);
    res.ok = true;
    res.text = dummyDecode(mod);
    res.quad = quad;
    return res;
}
