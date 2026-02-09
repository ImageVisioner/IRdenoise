#include "fom_denoise.h"
#include <iostream>
#include <iomanip>
#include <chrono>

int main(int argc, char* argv[]) {
    std::cout << "=======================================" << std::endl;
    std::cout << "    FOM Image Denoising Algorithm     " << std::endl;
    std::cout << "        柔性去噪（FOM）算法          " << std::endl;
    std::cout << "=======================================" << std::endl;
    std::cout << "Author: Chenhua Liu (liuchenhua023@163.com)" << std::endl;
    std::cout << std::endl;
    
    // 配置参数
    std::string image_path = "processed_noise_0005.png";
    bool show_images = true; // 是否显示图像窗口

    if (argc > 1) {
        image_path = argv[1];
    }
    if (argc > 2 && std::string(argv[2]) == "--no-display") {
        show_images = false;
    }
    
    double beta = 1.0;
    double mu = 1.0;
    double alpha_fom = 1.8;
    double alpha_tv = 2.0;
    double lambda_lr = 0.0; // 暂时禁用低秩
    
    std::cout << "Configuration / 配置:" << std::endl;
    std::cout << "  Image: " << image_path << std::endl;
    std::cout << "  Beta: " << beta << std::endl;
    std::cout << "  Mu: " << mu << std::endl;
    std::cout << "  FOM Alpha: " << alpha_fom << std::endl;
    std::cout << "  TV Alpha: " << alpha_tv << std::endl;
    std::cout << "  Low-rank Lambda: " << lambda_lr << std::endl;
    std::cout << std::endl;
    
    // 加载图像
    std::cout << "Loading image / 加载图像..." << std::endl;
    cv::Mat I = cv::imread(image_path, cv::IMREAD_GRAYSCALE);
    if (I.empty()) {
        std::cerr << "Failed to load image: " << image_path << std::endl;
        return -1;
    }
    
    cv::Mat I1;
    I.convertTo(I1, CV_64F, 1.0 / 255.0);
    cv::Mat u0 = I1.clone();
    
    std::cout << "Image loaded successfully. Size: " << u0.rows << "x" << u0.cols << std::endl;
    
    // 测试不同的去噪方法
    // 测试1: 原始FOM（无低秩）
    std::cout << std::endl << "--- Test 1: Original FOM (alpha=" << alpha_fom << ", lambda_lr=0) ---" << std::endl;
    FOMOutput output_fom;
    auto start = std::chrono::high_resolution_clock::now();
    cv::Mat u_fom = FOM(u0, beta, mu, alpha_fom, 0.0, output_fom);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "Completed in " << duration.count() / 1000.0 << " seconds" << std::endl;

    double min_val, max_val;
    cv::minMaxLoc(u_fom, &min_val, &max_val);
    cv::Scalar mean_val = cv::mean(u_fom);
    std::cout << "Result range: [" << min_val << ", " << max_val << "], Mean: " << mean_val[0] << std::endl;

    // 如果结果有问题，使用输入图像作为fallback
    if (!cv::checkRange(u_fom) || cv::countNonZero(cv::abs(u_fom) > 1e6) > 0) {
        std::cout << "Warning: FOM result unstable, using input image" << std::endl;
        u_fom = u0.clone();
    }
    
    // 测试2: FOM with low-rank
    std::cout << std::endl << "--- Test 2: FOM with Low-rank (alpha=" << alpha_fom << ", lambda_lr=" << lambda_lr << ") ---" << std::endl;
    FOMOutput output_fom_lr;
    start = std::chrono::high_resolution_clock::now();
    cv::Mat u_fom_lr = FOM(u0, beta, mu, alpha_fom, lambda_lr, output_fom_lr);
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "Completed in " << duration.count() / 1000.0 << " seconds" << std::endl;

    cv::minMaxLoc(u_fom_lr, &min_val, &max_val);
    mean_val = cv::mean(u_fom_lr);
    std::cout << "Result range: [" << min_val << ", " << max_val << "], Mean: " << mean_val[0] << std::endl;

    if (!cv::checkRange(u_fom_lr) || cv::countNonZero(cv::abs(u_fom_lr) > 1e6) > 0) {
        std::cout << "Warning: FOM+LR result unstable, using input image" << std::endl;
        u_fom_lr = u0.clone();
    }

    // 测试3: TV with low-rank
    std::cout << std::endl << "--- Test 3: TV with Low-rank (alpha=" << alpha_tv << ", lambda_lr=" << lambda_lr << ") ---" << std::endl;
    FOMOutput output_tv_lr;
    start = std::chrono::high_resolution_clock::now();
    cv::Mat u_tv_lr = FOM(u0, beta, mu, alpha_tv, lambda_lr, output_tv_lr);
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "Completed in " << duration.count() / 1000.0 << " seconds" << std::endl;

    cv::minMaxLoc(u_tv_lr, &min_val, &max_val);
    mean_val = cv::mean(u_tv_lr);
    std::cout << "Result range: [" << min_val << ", " << max_val << "], Mean: " << mean_val[0] << std::endl;

    if (!cv::checkRange(u_tv_lr) || cv::countNonZero(cv::abs(u_tv_lr) > 1e6) > 0) {
        std::cout << "Warning: TV+LR result unstable, using input image" << std::endl;
        u_tv_lr = u0.clone();
    }
    
    // 计算PSNR值
    std::cout << std::endl << "--- PSNR Results / PSNR结果 ---" << std::endl;
    double psnr_input = PSNR(I, u0);
    double psnr_fom = PSNR(I, u_fom);
    double psnr_fom_lr = PSNR(I, u_fom_lr);
    double psnr_tv_lr = PSNR(I, u_tv_lr);
    
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Input Image PSNR: " << psnr_input << " dB" << std::endl;
    std::cout << "Original FOM PSNR: " << psnr_fom << " dB" << std::endl;
    std::cout << "FOM + Low-rank PSNR: " << psnr_fom_lr << " dB" << std::endl;
    std::cout << "TV + Low-rank PSNR: " << psnr_tv_lr << " dB" << std::endl;
    
    // 归一化图像用于显示和保存
    cv::Mat display_input, display_fom, display_fom_lr, display_tv_lr;
    u0.convertTo(display_input, CV_8U, 255.0);
    u_fom.convertTo(display_fom, CV_8U, 255.0);
    u_fom_lr.convertTo(display_fom_lr, CV_8U, 255.0);
    u_tv_lr.convertTo(display_tv_lr, CV_8U, 255.0);

    // 显示结果（如果启用）
    cv::Mat canvas;
    if (show_images) {
        std::cout << std::endl << "--- Displaying Results / 显示结果 ---" << std::endl;

        // 创建2x2的subplot布局
        int rows = display_input.rows;
        int cols = display_input.cols;
        int margin = 10; // 子图间距
        int title_height = 30; // 标题高度

        // 创建大的画布
        canvas = cv::Mat(rows * 2 + margin + title_height * 2, cols * 2 + margin, CV_8U, cv::Scalar(255));

        // 复制图像到画布
        cv::Rect roi1(0, title_height, cols, rows);
        cv::Rect roi2(cols + margin, title_height, cols, rows);
        cv::Rect roi3(0, rows + margin + title_height * 2, cols, rows);
        cv::Rect roi4(cols + margin, rows + margin + title_height * 2, cols, rows);

        display_input.copyTo(canvas(roi1));
        display_fom.copyTo(canvas(roi2));
        display_fom_lr.copyTo(canvas(roi3));
        display_tv_lr.copyTo(canvas(roi4));

        // 添加标题
        cv::putText(canvas, "Input", cv::Point(10, 20), cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0), 2);
        cv::putText(canvas, cv::format("PSNR: %.2f dB", psnr_input), cv::Point(10, 40), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0), 1);

        cv::putText(canvas, "Original FOM", cv::Point(cols + margin + 10, 20), cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0), 2);
        cv::putText(canvas, cv::format("PSNR: %.2f dB", psnr_fom), cv::Point(cols + margin + 10, 40), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0), 1);

        cv::putText(canvas, "FOM + Low-rank", cv::Point(10, rows + margin + title_height + 20), cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0), 2);
        cv::putText(canvas, cv::format("PSNR: %.2f dB", psnr_fom_lr), cv::Point(10, rows + margin + title_height + 40), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0), 1);

        cv::putText(canvas, "TV + Low-rank", cv::Point(cols + margin + 10, rows + margin + title_height + 20), cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0), 2);
        cv::putText(canvas, cv::format("PSNR: %.2f dB", psnr_tv_lr), cv::Point(cols + margin + 10, rows + margin + title_height + 40), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0), 1);

        // 显示图像
        cv::imshow("FOM Denoising Results", canvas);
        cv::waitKey(0); // 等待按键
    }

    // 保存结果
    std::cout << std::endl << "--- Saving Results / 保存结果 ---" << std::endl;
    std::string output_dir = "Results";

    // 创建输出目录
    #ifdef _WIN32
        system(("mkdir " + output_dir).c_str());
    #else
        system(("mkdir -p " + output_dir).c_str());
    #endif

    cv::imwrite(output_dir + "/FOM_result.png", display_fom);
    cv::imwrite(output_dir + "/FOM_lowrank_result.png", display_fom_lr);
    cv::imwrite(output_dir + "/TV_lowrank_result.png", display_tv_lr);

    // 总是保存comparison.png（即使没有显示窗口）
    if (canvas.empty()) {
        // 创建comparison图像，即使没有显示窗口
        int rows = display_input.rows;
        int cols = display_input.cols;
        int margin = 10;
        int title_height = 30;

        canvas = cv::Mat(rows * 2 + margin + title_height * 2, cols * 2 + margin, CV_8U, cv::Scalar(255));

        cv::Rect roi1(0, title_height, cols, rows);
        cv::Rect roi2(cols + margin, title_height, cols, rows);
        cv::Rect roi3(0, rows + margin + title_height * 2, cols, rows);
        cv::Rect roi4(cols + margin, rows + margin + title_height * 2, cols, rows);

        display_input.copyTo(canvas(roi1));
        display_fom.copyTo(canvas(roi2));
        display_fom_lr.copyTo(canvas(roi3));
        display_tv_lr.copyTo(canvas(roi4));

        cv::putText(canvas, "Input", cv::Point(10, 20), cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0), 2);
        cv::putText(canvas, cv::format("PSNR: %.2f dB", psnr_input), cv::Point(10, 40), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0), 1);

        cv::putText(canvas, "Original FOM", cv::Point(cols + margin + 10, 20), cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0), 2);
        cv::putText(canvas, cv::format("PSNR: %.2f dB", psnr_fom), cv::Point(cols + margin + 10, 40), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0), 1);

        cv::putText(canvas, "FOM + Low-rank", cv::Point(10, rows + margin + title_height + 20), cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0), 2);
        cv::putText(canvas, cv::format("PSNR: %.2f dB", psnr_fom_lr), cv::Point(10, rows + margin + title_height + 40), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0), 1);

        cv::putText(canvas, "TV + Low-rank", cv::Point(cols + margin + 10, rows + margin + title_height + 20), cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0), 2);
        cv::putText(canvas, cv::format("PSNR: %.2f dB", psnr_tv_lr), cv::Point(cols + margin + 10, rows + margin + title_height + 40), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0), 1);
    }

    cv::imwrite(output_dir + "/comparison.png", canvas);
    std::cout << "Saved: comparison.png (subplot view)" << std::endl;

    std::cout << "Saved: FOM_result.png" << std::endl;
    std::cout << "Saved: FOM_lowrank_result.png" << std::endl;
    std::cout << "Saved: TV_lowrank_result.png" << std::endl;
    std::cout << "Saved: comparison.png (subplot view)" << std::endl;
    std::cout << "Results saved to " << output_dir << "/" << std::endl;
    
    // 性能总结
    std::cout << std::endl << "=======================================" << std::endl;
    std::cout << "           Performance Summary         " << std::endl;
    std::cout << "               性能总结               " << std::endl;
    std::cout << "=======================================" << std::endl;
    std::cout << "Method              | PSNR (dB) | Improvement" << std::endl;
    std::cout << "--------------------|-----------|------------" << std::endl;
    std::cout << "Input              |   " << std::setw(6) << psnr_input << "  |     -" << std::endl;
    std::cout << "Original FOM       |   " << std::setw(6) << psnr_fom << "  |  " << std::setw(6) << (psnr_fom - psnr_input) << std::endl;
    std::cout << "FOM + Low-rank     |   " << std::setw(6) << psnr_fom_lr << "  |  " << std::setw(6) << (psnr_fom_lr - psnr_input) << std::endl;
    std::cout << "TV + Low-rank      |   " << std::setw(6) << psnr_tv_lr << "  |  " << std::setw(6) << (psnr_tv_lr - psnr_input) << std::endl;
    
    std::cout << std::endl << "Algorithm completed successfully!" << std::endl;
    std::cout << "算法执行成功！" << std::endl;
    
    return 0;
}
