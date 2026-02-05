function x = cconv2(A,w)

% Input:
%   A: input image / 输入图像
%   w: convolution kernel / 卷积核
%
% Output:
%   x: convolved result / 卷积结果

x = imfilter(A,w,'circular','full');
end