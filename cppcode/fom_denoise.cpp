#include "fom_denoise.h"
#include <cmath>
#include <algorithm>
#include <iostream>
#include <complex>

// 计算gamma函数（简化版本）
double gamma_func(double x) {
    if (x < 0.5) {
        return M_PI / (sin(M_PI * x) * gamma_func(1.0 - x));
    }
    x -= 1.0;
    double result = 0.99999999999980993;
    double coeffs[] = {
        676.5203681218851, -1259.1392167224028, 771.32342877765313,
        -176.61502916214059, 12.507343278686905, -0.13857109526572012,
        9.9843695780195716e-6, 1.5056327351493116e-7
    };
    for (int i = 0; i < 8; i++) {
        result += coeffs[i] / (x + i + 1);
    }
    double t = x + 7.5;
    return sqrt(2.0 * M_PI) * pow(t, x + 0.5) * exp(-t) * result;
}

// 计算阶乘
double factorial(int n) {
    double result = 1.0;
    for (int i = 2; i <= n; i++) {
        result *= i;
    }
    return result;
}

// 循环卷积（简化的空间域实现，与MATLAB cconv2兼容）
cv::Mat cconv2(const cv::Mat& A, const cv::Mat& w) {
    cv::Mat kernel = w.clone();
    if (kernel.cols == 1 && kernel.rows > 1) {
        kernel = kernel.t();
    }

    cv::Mat result;
    // 使用BORDER_WRAP进行循环卷积
    cv::Mat padded;
    int pad_x = kernel.cols / 2;
    int pad_y = kernel.rows / 2;

    cv::copyMakeBorder(A, padded, pad_y, pad_y, pad_x, pad_x, cv::BORDER_WRAP);

    cv::filter2D(padded, result, -1, kernel, cv::Point(-1, -1), 0, cv::BORDER_CONSTANT);

    // 提取中心部分，与MATLAB行为匹配
    cv::Rect roi(pad_x, pad_y, A.cols, A.rows);
    return result(roi);
}

// 前向差分算子
void ForwardD(const cv::Mat& U, const cv::Mat& w, cv::Mat& Dux, cv::Mat& Duy) {
    int m = U.rows;
    int n = U.cols;
    
    // 水平方向导数
    cv::Mat Dux_full = cconv2(U, w);
    Dux = Dux_full(cv::Rect(0, 0, n - 1, m)).clone();
    
    // 垂直方向导数
    cv::Mat w_transpose = w.t();
    cv::Mat Duy_full = cconv2(U, w_transpose);
    Duy = Duy_full(cv::Rect(0, 0, n, m - 1)).clone();
}

// 反向差分算子（转置）
cv::Mat Dive(const cv::Mat& X, const cv::Mat& Y, const cv::Mat& w, double alpha, const cv::Size& image_size) {
    cv::Mat w_flip;
    cv::flip(w, w_flip, 1); // 水平翻转
    
    cv::Mat DtXY = cv::Mat::zeros(image_size, CV_64F);
    
    // 处理X（水平差分）
    if (!X.empty() && cv::countNonZero(X) > 0) {
        // 需要将X扩展到image_size进行卷积
        cv::Mat X_padded = cv::Mat::zeros(image_size, CV_64F);
        cv::Rect roi(0, 0, std::min(X.cols, image_size.width), std::min(X.rows, image_size.height));
        X(roi).copyTo(X_padded(roi));
        
        cv::Mat DtX = cconv2(X_padded, w_flip);
        DtXY += DtX;
    }
    
    // 处理Y（垂直差分）
    if (!Y.empty() && cv::countNonZero(Y) > 0) {
        // 需要将Y扩展到image_size进行卷积
        cv::Mat Y_padded = cv::Mat::zeros(image_size, CV_64F);
        cv::Rect roi(0, 0, std::min(Y.cols, image_size.width), std::min(Y.rows, image_size.height));
        Y(roi).copyTo(Y_padded(roi));
        
        cv::Mat w_flip_transpose = w_flip.t();
        cv::Mat DtY = cconv2(Y_padded, w_flip_transpose);
        DtXY += DtY;
    }
    
    // 相位校正
    std::complex<double> phase = std::conj(std::pow(std::complex<double>(-1.0, 0.0), alpha)) * std::pow(std::complex<double>(-1.0, 0.0), alpha);
    DtXY *= phase.real();
    
    return DtXY;
}

// FOM去噪核心算法
cv::Mat FOM(const cv::Mat& f, double beta, double mu, double alpha, double lambda_lr, FOMOutput& output) {
    cv::Mat U = f.clone();
    int itermax = 500;
    double tol = 1e-5;
    
    int m = f.rows;
    int n = f.cols;
    
    // 初始化
    cv::Mat P1 = cv::Mat::zeros(m, n - 1, CV_64F);
    cv::Mat Lam1 = cv::Mat::zeros(m, n - 1, CV_64F);
    cv::Mat P2 = cv::Mat::zeros(m - 1, n, CV_64F);
    cv::Mat Lam2 = cv::Mat::zeros(m - 1, n, CV_64F);
    cv::Mat u = f.clone(); // 使用输入图像作为初始化
    
    // 生成FOM导数算子权重
    int K = 20;
    cv::Mat w = cv::Mat::zeros(1, K, CV_64F);
    w.at<double>(0, K - 1) = 1.0;
    for (int i = 1; i < K; i++) {
        double coeff = std::pow(-1.0, i) * gamma_func(alpha + 1) / 
                      (gamma_func(alpha - i + 1) * factorial(i));
        w.at<double>(0, K - 1 - i) = coeff;
    }
    
    // 计算FFT下的分母（类似MATLAB的psf2otf）
    cv::Mat psf_kernel_h = cv::Mat::zeros(m, n, CV_64F);
    cv::Mat psf_kernel_v = cv::Mat::zeros(m, n, CV_64F);
    
    // 将权重放入PSF核（第一个元素在(0,0)位置）
    for (int i = 0; i < K && i < n; i++) {
        psf_kernel_h.at<double>(0, i) = w.at<double>(0, i);
    }
    for (int i = 0; i < K && i < m; i++) {
        psf_kernel_v.at<double>(i, 0) = w.at<double>(0, i);
    }
    
    cv::Mat D1, D2;
    cv::dft(psf_kernel_h, D1, cv::DFT_COMPLEX_OUTPUT);
    cv::dft(psf_kernel_v, D2, cv::DFT_COMPLEX_OUTPUT);
    
    cv::Mat D1_mag, D2_mag;
    std::vector<cv::Mat> D1_channels, D2_channels;
    cv::split(D1, D1_channels);
    cv::split(D2, D2_channels);
    cv::magnitude(D1_channels[0], D1_channels[1], D1_mag);
    cv::magnitude(D2_channels[0], D2_channels[1], D2_mag);

    D1_mag = D1_mag.mul(D1_mag);
    D2_mag = D2_mag.mul(D2_mag);
    cv::Mat C_freq = D1_mag + D2_mag;

    // 将C转换到空间域，因为在u子问题中需要与空间域的U相乘
    cv::Mat C;
    cv::idft(C_freq, C, cv::DFT_REAL_OUTPUT | cv::DFT_SCALE);
    
    // 主ADMM迭代循环
    auto tstart = cv::getTickCount();
    
    for (int iter = 0; iter < itermax; iter++) {
        // u子问题
        cv::Mat Dt_Lam = Dive(Lam1, Lam2, w, alpha, cv::Size(n, m));
        cv::Mat Dt_P = Dive(P1, P2, w, alpha, cv::Size(n, m));

        // 分子：(beta*f + U.*Dt(Lam1,Lam2))/mu + U.*Dt(P1,P2)
        cv::Mat numerator = (beta * f + U.mul(Dt_Lam)) / mu + U.mul(Dt_P);

        // 分母：(beta/mu) + U.*C
        // U和C都在空间域，C是通过ifft2(D1+D2)得到的
        cv::Mat denominator = (beta / mu) + U.mul(C);

        // 在频域求解：fft2(numerator) ./ fft2(denominator)
        cv::Mat numerator_fft, denominator_fft;
        cv::dft(numerator, numerator_fft, cv::DFT_COMPLEX_OUTPUT);
        cv::dft(denominator, denominator_fft, cv::DFT_COMPLEX_OUTPUT);

        cv::Mat u_fft_result;
        cv::divide(numerator_fft, denominator_fft, u_fft_result);

        cv::Mat u_ifft;
        cv::idft(u_fft_result, u_ifft, cv::DFT_REAL_OUTPUT | cv::DFT_SCALE);
        u = u_ifft;

        // 检查数值稳定性
        if (cv::countNonZero(cv::abs(u) > 1e10) > 0 || !cv::checkRange(u)) {
            std::cout << "Warning: Numerical instability detected in iteration " << iter << std::endl;
            u = U.clone(); // 使用上一轮的结果
        }
        
        // 低秩正则化
        if (lambda_lr > 0) {
            cv::SVD svd(u, cv::SVD::MODIFY_A | cv::SVD::FULL_UV);

            // 获取奇异值矩阵
            cv::Mat sigma = svd.w.clone();

            // 应用软阈值
            double threshold = lambda_lr;
            for (int i = 0; i < sigma.rows; i++) {
                double val = sigma.at<double>(i);
                if (i < 5) { // 保留前5个奇异值
                    continue;
                }
                sigma.at<double>(i) = std::max(val - threshold, 0.0);
            }

            // 重构矩阵，确保尺寸匹配
            cv::Mat S = cv::Mat::diag(sigma);
            cv::Mat U_part = svd.u(cv::Rect(0, 0, S.cols, svd.u.rows));
            cv::Mat Vt_part = svd.vt(cv::Rect(0, 0, S.cols, S.rows));
            u = U_part * S * Vt_part;
        }
        
        // z子问题（收缩步骤）
        cv::Mat D1U, D2U;
        ForwardD(u, w, D1U, D2U);
        
        // 处理Z1（软阈值收缩）
        cv::Mat Z1 = D1U - Lam1 / mu;
        cv::Mat W1 = cv::abs(Z1);
        cv::Mat Max1;
        cv::max(W1 - 1.0 / mu, 0.0, Max1);
        cv::Mat W1_safe;
        cv::max(W1, 1e-10, W1_safe); // 避免除零
        P1 = Max1.mul(Z1) / W1_safe;
        
        // 处理Z2（软阈值收缩）
        cv::Mat Z2 = D2U - Lam2 / mu;
        cv::Mat W2 = cv::abs(Z2);
        cv::Mat Max2;
        cv::max(W2 - 1.0 / mu, 0.0, Max2);
        cv::Mat W2_safe;
        cv::max(W2, 1e-10, W2_safe); // 避免除零
        P2 = Max2.mul(Z2) / W2_safe;
        
        // 更新拉格朗日乘子
        Lam1 = Lam1 + mu * (P1 - D1U);
        Lam2 = Lam2 + mu * (P2 - D2U);
        
        output.cpu.push_back((cv::getTickCount() - tstart) / cv::getTickFrequency());
        
        // 能量计算
        cv::Mat norm_D1U = cv::abs(D1U);
        cv::Mat norm_D2U = cv::abs(D2U);
        
        double energy = cv::sum(norm_D1U)[0] + cv::sum(norm_D2U)[0];
        cv::Mat log_term;
        cv::log(u + 1e-14, log_term);
        cv::Mat poisson_term = u - f.mul(log_term);
        energy += beta * cv::sum(poisson_term)[0];
        
        if (lambda_lr > 0) {
            cv::Mat u_sq = u.mul(u);
            energy += lambda_lr * cv::sum(u_sq)[0];
        }
        
        output.X.push_back(energy);
        
        // 相对误差
        cv::Mat diff = u - U;
        double norm_diff = cv::norm(diff);
        double norm_u = cv::norm(u);
        double z1 = norm_u > 0 ? norm_diff / norm_u : 0;
        output.Z.push_back(z1);
        
        U = u.clone();
        
        // 收敛检查
        if (z1 < tol) {
            break;
        }
    }
    
    return u;
}

// PSNR计算
double PSNR(const cv::Mat& ud, const cv::Mat& u) {
    cv::Mat ud_double, u_double;
    ud.convertTo(ud_double, CV_64F, 1.0 / 255.0);
    u.convertTo(u_double, CV_64F, 1.0 / 255.0);
    
    cv::Mat diff = ud_double - u_double;
    cv::Mat diff_sq = diff.mul(diff);
    double mse = cv::mean(diff_sq)[0];
    
    double max_val;
    cv::minMaxLoc(ud_double, nullptr, &max_val);
    
    if (mse == 0) {
        return 100.0; // 完美匹配
    }
    
    return 10.0 * log10(max_val * max_val / mse);
}
