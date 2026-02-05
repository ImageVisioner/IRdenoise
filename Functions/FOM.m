function [u,output] = FOM(f,beta,mu,alpha,lambda_lr)
% Flexible Order Modulator (FOM) denoising with low-rank regularization
% 柔性去噪（FOM）算法，带低秩正则化
%
% This function implements image denoising using Flexible Order Modulator (FOM)
% with optional low-rank regularization for improved performance.
%
% 本函数实现基于柔性阶数调制器（FOM）的图像去噪算法，
% 可选低秩正则化以提升性能。

% If you use this code or part of this code please cite the following references:
% 如果您使用此代码或部分代码，请引用以下文献：
%
% REFERENCES: Chowdhury, M. R. and Zhang, J. and Qin, J. and Lou, Y.; Poisson image denoising based on fractional-order total variation
%                Inverse Problem and Imaging Volume 14, No. 1, 2020, 77-96
%                doi: 10.3934/ipi.2019064
%
%     Input / 输入参数:
%             f : noisy image / 含噪图像
%             beta: balancing parameter for regularization and data fitting term / 正则化与数据拟合项平衡参数
%             mu: penalty parameter / 惩罚参数
%             alpha: order of the derivative (flexible order) / 导数阶数（柔性阶数）
%             lambda_lr: low-rank regularization parameter (optional, default=0) / 低秩正则化参数（可选，默认=0）
%
%     Output / 输出参数:
%             u: recovered/denoised image / 恢复/去噪图像
%             output.X: energy at each iteration / 每次迭代的能量
%             output.Z: relative error at each iteration / 每次迭代的相对误差
%
%   The proposed model / 优化模型:
%
%              min_{u,z} || z ||_1 + beta * sum ( u - f * log(u) )
%                                + lambda_lr * ||u||_{S_{1/2}}
%                                + 0.5 * mu || z - D(u) + Lam/mu ||^2
%
% Mujibur R. Chowdhury, 12/8/2019.
% Modified to include low-rank regularization / 修改以包含低秩正则化

% Default low-rank parameter / 默认低秩参数
if nargin < 5 || isempty(lambda_lr)
    lambda_lr = 0;
end

U = f;

itermax = 500; % Maximum number of iterations / 最大迭代次数
tol = 1e-5;     % Error tolerance / 误差容限

[m,n] = size(f); % Read the image size / 读取图像尺寸

% Initialization / 初始化
% Note: P1 and Lam1 should match D1U size (m x (n-1))
% 注意：P1和Lam1应该与D1U尺寸匹配 (m x (n-1))
P1 = zeros(m,n-1);
Lam1 = P1;

% Note: P2 and Lam2 should match D2U size ((m-1) x n)
% 注意：P2和Lam2应该与D2U尺寸匹配 ((m-1) x n)
P2 = zeros(m-1,n);
Lam2 = P2;

u = zeros(m,n);

% Low-rank regularization uses direct approximation / 低秩正则化使用直接近似
% No additional variables needed / 不需要额外变量

% FOM derivative operator / FOM导数算子
K = 20;
w = zeros(1,K);
w(K) = 1;
for i = 1:(K-1)
    w(K-i) = (-1)^i*gamma(alpha+1)/gamma(alpha-i+1)/factorial(i);
end
[D,Dt] = defDDt(w,alpha, size(f));

% Define the denominator for the LS subproblem under FFT / 在FFT下定义最小二乘子问题的分母
D1 = abs(psf2otf(w, [m,n])).^2; % backward, column / 后向，列
D2 = abs(psf2otf(w',[m,n])).^2; % row / 行
C = D1 + D2;

% Main ADMM iteration loop / 主要ADMM迭代循环
tstart = tic;
for iter=1:itermax
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    % ==================
    %   u-subproblem / u子问题
    % ==================

    % Solve u using Poisson noise model / 使用泊松噪声模型求解u
    u = (beta*f + U.*Dt(Lam1,Lam2))/mu + U.*Dt(P1,P2); %for Poisson noise / 泊松噪声
    u = fft2(u)./((beta/mu)+ U.*C); %for Poisson noise,not ./ / 泊松噪声
    u = real(ifft2(u));

    % Apply low-rank regularization if enabled / 如果启用则应用低秩正则化
    if lambda_lr > 0
        % Gentle low-rank regularization using nuclear norm approximation
        % 使用温和的低秩正则化（核范数近似）
        [U_lr, S_lr, V_lr] = svd(u, 'econ');
        sigma = diag(S_lr);

        % Apply soft-thresholding to singular values / 对奇异值应用软阈值
        threshold = lambda_lr;  % Fixed small threshold / 固定小阈值
        sigma_filtered = max(sigma - threshold, 0);

        % Keep at least the first few singular values to preserve structure
        % 至少保留前几个奇异值以保持结构
        min_singular_values = min(5, length(sigma));  % Keep at least 5 singular values / 至少保留5个奇异值
        sigma_filtered(1:min_singular_values) = sigma(1:min_singular_values);

        u = U_lr * diag(sigma_filtered) * V_lr';
    end

    % ==================
    %   z-subproblem (Shrinkage Step) / z子问题（收缩步骤）
    % ==================

    [D1U,D2U] = D(u);

    % Process Z1 (horizontal differences) / 处理Z1（水平差分）
    Z1 = D1U - Lam1/mu;
    W1 = sqrt(Z1.^2);
    Max1 = max(W1 - 1/mu, 0);
    W1(W1==0) = 1;
    P1 = Max1.*Z1./W1;

    % Process Z2 (vertical differences) / 处理Z2（垂直差分）
    Z2 = D2U - Lam2/mu;
    W2 = sqrt(Z2.^2);
    Max2 = max(W2 - 1/mu, 0);
    W2(W2==0) = 1;
    P2 = Max2.*Z2./W2;

    % ==================
    %    Update Lagrange multipliers / 更新拉格朗日乘子
    % ==================

    Lam1 = Lam1 + mu*(P1 - D1U);
    Lam2 = Lam2 + mu*(P2 - D2U);

    output.cpu(iter) = toc(tstart);

    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    % Energy computation / 能量计算
    % Compute norms separately since D1U and D2U have different sizes
    % 分别计算范数，因为D1U和D2U有不同的尺寸
    norm_D1U = sqrt(D1U.^2);
    norm_D2U = sqrt(D2U.^2);
    x1 = sum(norm_D1U(:)) + sum(norm_D2U(:)) + sum(sum((u-f.*log(u+1e-14))*beta));

    % Add low-rank regularization energy / 添加低秩正则化能量
    if lambda_lr > 0
        % Approximate low-rank energy using trace / 使用迹近似低秩能量
        lowrank_term = lambda_lr * sum(sum(u.^2));  % Simplified regularization / 简化的正则化
        x1 = x1 + lowrank_term;
    end

    output.X(iter) = x1;

    z1 = norm(u(:)-U(:))/norm(u(:));
    output.Z(iter) = z1;

    U = u;

    % Convergence check / 收敛检查
    if z1 < tol
        break;
    end

end

end