#!/bin/bash

# FOM图像去噪算法构建脚本

echo "======================================="
echo "    FOM Denoising Build Script       "
echo "======================================="

# 创建build目录
if [ ! -d "build" ]; then
    mkdir build
    echo "Created build directory"
fi

cd build

# 运行CMake
echo "Running CMake..."
cmake ..

# 编译
echo "Building..."
make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

if [ $? -eq 0 ]; then
    echo ""
    echo "======================================="
    echo "Build completed successfully!"
    echo "可执行文件位置: build/fom_denoise"
    echo ""
    echo "使用方法:"
    echo "  cd build"
    echo "  ./fom_denoise [图像路径]"
    echo "======================================="
else
    echo ""
    echo "Build failed!"
    exit 1
fi
