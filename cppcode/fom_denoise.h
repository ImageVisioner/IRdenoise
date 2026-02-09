#ifndef FOM_DENOISE_H
#define FOM_DENOISE_H

#include <opencv2/opencv.hpp>
#include <vector>

// 循环卷积函数
cv::Mat cconv2(const cv::Mat& A, const cv::Mat& w);

// 前向差分算子
void ForwardD(const cv::Mat& U, const cv::Mat& w, cv::Mat& Dux, cv::Mat& Duy);

// 反向差分算子（转置）
cv::Mat Dive(const cv::Mat& X, const cv::Mat& Y, const cv::Mat& w, double alpha, const cv::Size& image_size);

// FOM去噪核心算法
struct FOMOutput {
    std::vector<double> X;  // 能量
    std::vector<double> Z;  // 相对误差
    std::vector<double> cpu; // CPU时间
};

cv::Mat FOM(const cv::Mat& f, double beta, double mu, double alpha, double lambda_lr, FOMOutput& output);

// PSNR计算
double PSNR(const cv::Mat& ud, const cv::Mat& u);

#endif // FOM_DENOISE_H
