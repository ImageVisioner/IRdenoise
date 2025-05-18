close all
clc
str='./noise/noise_1517.raw';
col=256;
row=192;
fid=fopen(str,'rb');
A=fread(fid,[col row],'uint16');
fclose(fid);
figure,imshow(A,[])
imwarp
disp("结束")
