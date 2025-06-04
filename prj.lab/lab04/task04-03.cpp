#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

struct Stats {
    double tp = 0;
    double fp = 0;
    double fn = 0;
    double tn = 0;
};

namespace metrics {

    double safeDiv(double a, double b) {
        return (b > 0.0) ? (a / b) : 0.0;
    }

    double iou(const cv::Mat& maskA, const cv::Mat& maskB) {
        double inter = static_cast<double>(cv::countNonZero(maskA & maskB));
        double uni = static_cast<double>(cv::countNonZero(maskA | maskB));
        return (uni > 0.0) ? (inter / uni) : 0.0;
    }

    Stats pixelwise(const cv::Mat& gtMask, const cv::Mat& predMask) {
        Stats s;
        s.tp = cv::countNonZero((gtMask == 255) & (predMask == 255));
        s.fp = cv::countNonZero((gtMask == 0) & (predMask == 255));
        s.fn = cv::countNonZero((gtMask == 255) & (predMask == 0));
        s.tn = cv::countNonZero((gtMask == 0) & (predMask == 0));
        return s;
    }

    void objectwise(const cv::Mat& gtMask, const cv::Mat& predMask,
        double& detTP, double& detFN, double& detFP)
    {
        cv::Mat labelGT, labelPR;
        int numGT = cv::connectedComponents(gtMask, labelGT, 8, CV_32S);
        int numPR = cv::connectedComponents(predMask, labelPR, 8, CV_32S);

        std::vector<char> matched(numPR, 0);
        detTP = detFN = detFP = 0;

        for (int i = 1; i < numGT; ++i) {
            cv::Mat maskGT = (labelGT == i);
            double bestIoU = 0.0;
            int bestMatch = -1;

            for (int j = 1; j < numPR; ++j) {
                cv::Mat maskPR = (labelPR == j);
                double score = iou(maskGT, maskPR);
                if (score > bestIoU) {
                    bestIoU = score;
                    bestMatch = j;
                }
            }

            if (bestIoU > 0.5) {
                detTP++;
                matched[bestMatch] = 1;
            }
            else {
                detFN++;
            }
        }

        for (int j = 1; j < numPR; ++j) {
            if (!matched[j]) {
                detFP++;
            }
        }
    }

    std::vector<std::string> readList(const std::string& filePath) {
        std::vector<std::string> list;
        std::ifstream file(filePath);
        std::string line;
        while (std::getline(file, line)) {
            if (!line.empty()) {
                list.push_back(line);
            }
        }
        return list;
    }

}

int main(int argc, char** argv) {
    if (argc != 4) {
        std::cerr << "Usage: task04-03 <gt.lst> <pred.lst> <lab_04.md>" << std::endl;
        return 1;
    }

    std::vector<std::string> gtList = metrics::readList(argv[1]);
    std::vector<std::string> predList = metrics::readList(argv[2]);

    if (gtList.size() != predList.size()) {
        std::cerr << "Error: list size mismatch." << std::endl;
        return 2;
    }

    Stats totalSeg;
    double totalTP = 0.0, totalFN = 0.0, totalFP = 0.0;

    for (size_t i = 0; i < gtList.size(); ++i) {
        cv::Mat gtMask = cv::imread(gtList[i], cv::IMREAD_GRAYSCALE);
        cv::Mat predMask = cv::imread(predList[i], cv::IMREAD_GRAYSCALE);

        if (gtMask.empty() || predMask.empty()) {
            std::cerr << "Warning: cannot read pair index " << i << std::endl;
            continue;
        }

        Stats seg = metrics::pixelwise(gtMask, predMask);
        totalSeg.tp += seg.tp;
        totalSeg.fp += seg.fp;
        totalSeg.fn += seg.fn;
        totalSeg.tn += seg.tn;

        double detTP, detFN, detFP;
        metrics::objectwise(gtMask, predMask, detTP, detFN, detFP);
        totalTP += detTP;
        totalFN += detFN;
        totalFP += detFP;
    }

    double segPrecision = metrics::safeDiv(totalSeg.tp, totalSeg.tp + totalSeg.fp);
    double segRecall = metrics::safeDiv(totalSeg.tp, totalSeg.tp + totalSeg.fn);
    double segF1 = metrics::safeDiv(2 * segPrecision * segRecall, segPrecision + segRecall);

    double detPrecision = metrics::safeDiv(totalTP, totalTP + totalFP);
    double detRecall = metrics::safeDiv(totalTP, totalTP + totalFN);
    double detF1 = metrics::safeDiv(2 * detPrecision * detRecall, detPrecision + detRecall);

    std::ofstream report(argv[3]);
    if (!report) {
        std::cerr << "Error: failed to write report to " << argv[3] << std::endl;
        return 3;
    }

    report << "[SEGMENTATION]\n";
    report << "Precision: " << segPrecision << "\n";
    report << "Recall   : " << segRecall << "\n";
    report << "F1-score : " << segF1 << "\n\n";

    report << "[DETECTION]\n";
    report << "Precision: " << detPrecision << "\n";
    report << "Recall   : " << detRecall << "\n";
    report << "F1-score : " << detF1 << "\n";

    std::cout << "Report written to " << argv[3] << std::endl;
    return 0;
}
