# FOM图像去噪算法 - C++ OpenCV版本

这是FOM（Flexible Order Modulator）图像去噪算法的C++ OpenCV实现版本。

## 依赖要求

- CMake 3.10 或更高版本
- OpenCV 3.0 或更高版本
- C++11 或更高版本的编译器

## 编译方法

### 方法1：使用build.sh脚本（推荐）

```bash
cd cppcode
./build.sh
```

### 方法2：手动编译

```bash
cd cppcode
mkdir build
cd build
cmake ..
make -j4
```

## 使用方法

```bash
cd build
./fom_denoise [图像路径] [--no-display]
```

参数说明：
- `图像路径`: 输入图像路径（可选，默认使用 `processed_noise_0005.png`）
- `--no-display`: 禁用图像显示窗口（可选，用于无GUI环境）

## 输出结果

程序会在 `Results/` 目录下保存以下结果：
- `FOM_result.png` - 原始FOM去噪结果
- `FOM_lowrank_result.png` - FOM + 低秩正则化结果
- `TV_lowrank_result.png` - TV + 低秩正则化结果
- `comparison.png` - 2x2 subplot对比视图（包含所有结果和PSNR值）

如果不使用 `--no-display` 选项，还会在窗口中显示对比视图。

## 代码结构

- `fom_denoise.h` - 头文件，包含所有函数声明
- `fom_denoise.cpp` - 核心算法实现
- `main.cpp` - 主程序入口
- `CMakeLists.txt` - CMake构建配置
- `build.sh` - 构建脚本

## 主要功能

1. **cconv2** - 循环卷积函数（使用FFT实现）
2. **ForwardD** - 前向差分算子
3. **Dive** - 反向差分算子（转置）
4. **FOM** - FOM去噪核心算法（ADMM迭代求解）
5. **PSNR** - PSNR计算函数

## 参数说明

- `beta`: 正则化平衡参数（默认：100）
- `mu`: 惩罚参数（默认：2e-3）
- `alpha_fom`: FOM阶数参数（默认：1.5）
- `alpha_tv`: TV阶数参数（默认：2.0）
- `lambda_lr`: 低秩正则化参数（默认：0.01）

## 注意事项

- 输入图像应为灰度图像
- 算法使用ADMM迭代求解，最大迭代次数为500次
- 收敛容差为1e-5
