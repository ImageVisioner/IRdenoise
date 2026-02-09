cmake_minimum_required(VERSION 3.10)
project(FOMDenoise)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找OpenCV
find_package(OpenCV REQUIRED)

# 包含目录
include_directories(${OpenCV_INCLUDE_DIRS})

# 源文件
set(SOURCES
    fom_denoise.cpp
    main.cpp
)

# 可执行文件
add_executable(fom_denoise ${SOURCES})

# 链接OpenCV库
target_link_libraries(fom_denoise ${OpenCV_LIBS})

# 输出信息
message(STATUS "OpenCV version: ${OpenCV_VERSION}")
message(STATUS "OpenCV libraries: ${OpenCV_LIBS}")
