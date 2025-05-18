close all;
clc;

% 设置源文件夹和目标文件夹路径
sourceFolder = './groundtruth/';
targetFolder = './processed_groundtruth/';
if ~exist(targetFolder, 'dir')
    mkdir(targetFolder); % 如果目标文件夹不存在，则创建
end

% 获取所有符合命名规则的 .raw 文件
filePattern = fullfile(sourceFolder, 'gt_*.raw');
rawFiles = dir(filePattern);

% 图像的列和行数
col = 256;
row = 192;

% 遍历每个文件
for k = 1:length(rawFiles)
    % 获取文件路径
    str = fullfile(sourceFolder, rawFiles(k).name);
    
    % 打开文件读取数据
    fid = fopen(str, 'rb');
    A = fread(fid, [col, row], 'uint16');
    fclose(fid);
    
    % 缩放图像到 [0, 255] 范围
    A_scaled = mat2gray(A) * 255; % 将数据归一化并映射到[0, 255]
    
    % 将图像保存到目标文件夹
    targetFileName = fullfile(targetFolder, ['processed_' rawFiles(k).name(1:end-4) '.png']);
    imwrite(uint8(A_scaled), targetFileName); % 保存为 PNG 格式
    
    % 显示处理进度
    disp(['处理完成: ' rawFiles(k).name]);
end

disp("所有图像处理完毕！");
