function [D,Dt] = defDDt(w,a, image_size)
% defines finite difference operator D^a and its transpose operator Dt=conj(-1^a)div^a
% 定义有限差分算子D^a及其转置算子Dt=conj(-1^a)div^a
%
% This function creates the forward and backward finite difference operators
% for fractional-order derivatives in the FOM algorithm.
%
% 本函数为FOM算法创建分数阶导数的正向和反向有限差分算子。

%
% Input:
%   w: filter coefficients for fractional derivative / 分数阶导数的滤波器系数
%   a: order of the derivative / 导数阶数
%
% Output:
%   D: forward difference operator / 正向差分算子
%   Dt: transpose (backward) difference operator / 转置（反向）差分算子
%
% Note: Dt is designed to return results compatible with element-wise multiplication
% with the original image U in the ADMM algorithm.
% 注意：在ADMM算法中，Dt被设计为返回与原始图像U逐元素相乘兼容的结果。

D = @(U) ForwardD(U,w);
Dt = @(X,Y) Dive(X,Y,w,a, image_size);

    function [Dux,Duy] = ForwardD(U,w)
        % backward finite difference operator / 后向有限差分算子
        % Computes the fractional derivative in x and y directions
        % 计算x和y方向上的分数阶导数
        
        [m, n] = size(U);
        d = length(w);
        
        % Column differences (horizontal derivative) / 列差分（水平导数）
        Dux_full = cconv2(U, w); % Full convolution result / 完整卷积结果
        % Extract the part that matches Lam1 size: m × (n-1)
        % 提取匹配Lam1尺寸的部分：m × (n-1)
        % The full result has size (m+d-1) × (n+d-1), we need m × (n-1)
        % 完整结果尺寸为 (m+d-1) × (n+d-1)，我们需要 m × (n-1)
        Dux = Dux_full(1:m, 1:(n-1));  % Take first m rows and first (n-1) columns
        
        % Row differences (vertical derivative) / 行差分（垂直导数）
        Duy_full = cconv2(U, w'); % Full convolution result / 完整卷积结果
        % Extract the part that matches Lam2 size: (m-1) × n
        % 提取匹配Lam2尺寸的部分：(m-1) × n
        Duy = Duy_full(1:(m-1), 1:n);  % Take first (m-1) rows and first n columns
    end

    function DtXY = Dive(X,Y,w,a, image_size)
        % Transpose of the backward finite difference operator / 后向有限差分算子的转置
        % Computes the adjoint operator for the fractional derivative
        % 计算分数阶导数的伴随算子
        %
        % Returns result compatible with element-wise multiplication with original image
        % 返回与原始图像逐元素相乘兼容的结果

        d = length(w);
        w_flip = fliplr(w);

        % Initialize result with the correct size (same as original image)
        % 用正确尺寸初始化结果（与原始图像相同）
        DtXY = zeros(image_size);

        % Process X (horizontal differences) / 处理X（水平差分）
        if ~isempty(X) && any(X(:) ~= 0)
            DtX_full = cconv2(X, w_flip);
            % Extract the central part that corresponds to the original image size
            % 提取对应原始图像尺寸的中心部分
            [rows, cols] = size(DtX_full);
            start_row = floor((rows - image_size(1)) / 2) + 1;
            start_col = floor((cols - image_size(2)) / 2) + 1;
            DtX = DtX_full(start_row:start_row+image_size(1)-1, start_col:start_col+image_size(2)-1);
        else
            DtX = zeros(image_size);
        end

        % Process Y (vertical differences) / 处理Y（垂直差分）
        if ~isempty(Y) && any(Y(:) ~= 0)
            DtY_full = cconv2(Y, w_flip');
            % Extract the central part that corresponds to the original image size
            % 提取对应原始图像尺寸的中心部分
            [rows, cols] = size(DtY_full);
            start_row = floor((rows - image_size(1)) / 2) + 1;
            start_col = floor((cols - image_size(2)) / 2) + 1;
            DtY = DtY_full(start_row:start_row+image_size(1)-1, start_col:start_col+image_size(2)-1);
        else
            DtY = zeros(image_size);
        end

        % Combine results with phase correction / 用相位校正组合结果
        DtXY = conj((-1)^a) * (-1)^a * (DtX + DtY);
    end
end