% FOM Image Denoising Main Script
% FOM图像去噪主脚本
%
% This is the main entry point for FOM (Flexible Order Modulator) image denoising
% with low-rank regularization. The algorithm combines fractional-order total variation
% with low-rank constraints for effective noise removal.
%
% 这是FOM（柔性阶数调制器）图像去噪的主入口脚本。该算法结合分数阶全变分
% 和低秩约束，实现有效的噪声去除。
%
% Author: Chenhua Liu (liuchenhua023@163.com)
% (liuchenhua023@163.com)

% Main script entry point / 主脚本入口
clear; close all; clc;

% Add Functions path / 添加函数路径
addpath('Functions');

fprintf('=======================================\n');
fprintf('    FOM Image Denoising Algorithm     \n');
fprintf('        柔性去噪（FOM）算法          \n');
fprintf('=======================================\n');
fprintf('Author: Chenhua Liu (liuchenhua023@163.com)\n');


%% Configuration / 配置参数
% Default parameters / 默认参数
config = struct();
config.image_path = 'processed_noise_0005.png';  % Default test image / 默认测试图像
config.beta = 100;          % Regularization balancing parameter / 正则化平衡参数
config.mu = 2e-3;           % Penalty parameter / 惩罚参数
config.alpha_fom = 1.5;     % FOM order parameter / FOM阶数参数
config.alpha_tv = 2.0;      % TV order parameter / TV阶数参数
config.lambda_lr = 0.01;    % Low-rank regularization parameter / 低秩正则化参数

% You can manually change the image path here / 您可以在此处手动更改图像路径
% config.image_path = 'your_image.png';  % Uncomment and modify / 取消注释并修改

fprintf('Configuration / 配置:\n');
fprintf('  Image: %s\n', config.image_path);
fprintf('  Beta: %.1f\n', config.beta);
fprintf('  Mu: %.4f\n', config.mu);
fprintf('  FOM Alpha: %.1f\n', config.alpha_fom);
fprintf('  TV Alpha: %.1f\n', config.alpha_tv);
fprintf('  Low-rank Lambda: %.4f\n', config.lambda_lr);
fprintf('\n');

%% Load test image / 加载测试图像
fprintf('Loading image / 加载图像...\n');
try
    I = imread(config.image_path);
    I1 = im2double(I);
    u0 = I1;
    fprintf('Image loaded successfully. Size: %dx%d\n', size(u0,1), size(u0,2));
catch
    error('Failed to load image: %s\n加载图像失败: %s', config.image_path, config.image_path);
end

%% Test different denoising methods / 测试不同的去噪方法
results = struct();

% Test 1: Original FOM (without low-rank) / 测试1：原始FOM（无低秩）
fprintf('\n--- Test 1: Original FOM (alpha=%.1f, lambda_lr=0) ---\n', config.alpha_fom);
tic;
[results.u_fom, output_fom] = FOM(u0, config.beta, config.mu, config.alpha_fom, 0);
toc_time = toc;
fprintf('Completed in %.2f seconds\n', toc_time);
fprintf('Result range: [%.4f, %.4f], Mean: %.4f\n', min(results.u_fom(:)), max(results.u_fom(:)), mean(results.u_fom(:)));

% Test 2: FOM with low-rank regularization / 测试2：带低秩正则化的FOM
fprintf('\n--- Test 2: FOM with Low-rank (alpha=%.1f, lambda_lr=%.4f) ---\n', config.alpha_fom, config.lambda_lr);
tic;
[results.u_fom_lr, output_fom_lr] = FOM(u0, config.beta, config.mu, config.alpha_fom, config.lambda_lr);
toc_time = toc;
fprintf('Completed in %.2f seconds\n', toc_time);
fprintf('Result range: [%.4f, %.4f], Mean: %.4f\n', min(results.u_fom_lr(:)), max(results.u_fom_lr(:)), mean(results.u_fom_lr(:)));

% Test 3: TV with low-rank regularization / 测试3：带低秩正则化的TV
fprintf('\n--- Test 3: TV with Low-rank (alpha=%.1f, lambda_lr=%.4f) ---\n', config.alpha_tv, config.lambda_lr);
tic;
[results.u_tv_lr, output_tv_lr] = FOM(u0, config.beta, config.mu, config.alpha_tv, config.lambda_lr);
toc_time = toc;
fprintf('Completed in %.2f seconds\n', toc_time);
fprintf('Result range: [%.4f, %.4f], Mean: %.4f\n', min(results.u_tv_lr(:)), max(results.u_tv_lr(:)), mean(results.u_tv_lr(:)));

%% Calculate PSNR values / 计算PSNR值
fprintf('\n--- PSNR Results / PSNR结果 ---\n');
psnr_input = PSNR(I, u0);
psnr_fom = PSNR(I, results.u_fom);
psnr_fom_lr = PSNR(I, results.u_fom_lr);
psnr_tv_lr = PSNR(I, results.u_tv_lr);

fprintf('Input Image PSNR: %.2f dB\n', psnr_input);
fprintf('Original FOM PSNR: %.2f dB\n', psnr_fom);
fprintf('FOM + Low-rank PSNR: %.2f dB\n', psnr_fom_lr);
fprintf('TV + Low-rank PSNR: %.2f dB\n', psnr_tv_lr);

%% Display results / 显示结果
fprintf('\n--- Displaying Results / 显示结果 ---\n');
figure('Name', 'FOM Denoising Results', 'NumberTitle', 'off');

subplot(221);
imshow(u0);
title(sprintf('Input\nPSNR: %.2f dB', psnr_input));

subplot(222);
imshow(results.u_fom);
title(sprintf('Original FOM\nPSNR: %.2f dB', psnr_fom));

subplot(223);
imshow(results.u_fom_lr);
title(sprintf('FOM + Low-rank\nPSNR: %.2f dB', psnr_fom_lr));

subplot(224);
imshow(results.u_tv_lr);
title(sprintf('TV + Low-rank\nPSNR: %.2f dB', psnr_tv_lr));

%% Save results / 保存结果
fprintf('\n--- Saving Results / 保存结果 ---\n');
output_dir = 'Results';

% Create output directory with error handling / 创建输出目录并进行错误处理
try
    if ~exist(output_dir, 'dir')
        mkdir(output_dir);
        fprintf('Created output directory: %s\n', output_dir);
    end

    % Save results with error handling / 保存结果并进行错误处理
    output_files = {
        'FOM_result.png', results.u_fom;
        'FOM_lowrank_result.png', results.u_fom_lr;
        'TV_lowrank_result.png', results.u_tv_lr
    };

    saved_files = {};
    for i = 1:size(output_files, 1)
        filename = output_files{i, 1};
        image_data = output_files{i, 2};
        filepath = fullfile(output_dir, filename);

        try
            % Ensure image data is in valid range for imwrite / 确保图像数据在imwrite的有效范围内
            image_data = max(0, min(1, image_data));  % Clamp to [0, 1] / 限制到[0,1]范围

            imwrite(image_data, filepath);
            saved_files{end+1} = filename;
            fprintf('Saved: %s\n', filename);
        catch save_error
            fprintf('Warning: Failed to save %s - %s\n', filename, save_error.message);
        end
    end

    fprintf('Results saved to %s/\n', output_dir);
    if ~isempty(saved_files)
        fprintf('Files saved: %s\n', strjoin(saved_files, ', '));
    end

catch dir_error
    fprintf('Warning: Failed to create output directory - %s\n', dir_error.message);
    fprintf('Results will not be saved to disk.\n');
end

%% Performance summary / 性能总结
fprintf('\n=======================================\n');
fprintf('           Performance Summary         \n');
fprintf('               性能总结               \n');
fprintf('=======================================\n');
fprintf('Method              | PSNR (dB) | Improvement\n');
fprintf('-------------------|-----------|------------\n');
fprintf('Input              |   %6.2f  |     -\n', psnr_input);
fprintf('Original FOM       |   %6.2f  |  %+6.2f\n', psnr_fom, psnr_fom - psnr_input);
fprintf('FOM + Low-rank     |   %6.2f  |  %+6.2f\n', psnr_fom_lr, psnr_fom_lr - psnr_input);
fprintf('TV + Low-rank      |   %6.2f  |  %+6.2f\n', psnr_tv_lr, psnr_tv_lr - psnr_input);

fprintf('\nAlgorithm completed successfully!\n');
fprintf('算法执行成功！\n');
