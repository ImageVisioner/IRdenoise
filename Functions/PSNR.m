function psnr = PSNR(ud, u)
% Calculate Peak Signal-to-Noise Ratio (PSNR)
% 计算峰值信噪比(PSNR)
%
% Input:
%   ud: reference image (ground truth) / 参考图像（真值）
%   u:  test image (denoised result) / 测试图像（去噪结果）
%
% Output:
%   psnr: PSNR value in dB / PSNR值（单位：dB）

ud = im2double(ud);
u = im2double(u);
[x, y] = size(ud);
o = sum(sum((ud - u).^2));
t = o / (x * y);
maxu = max(ud(:));
psnr = 10 * log10(maxu^2 / t);
end