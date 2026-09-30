# ============================================================
#  FaceGo 智勤 · 人脸识别考勤系统（C/S）—— 顶层 CMake
#  统一构建 AttendenceServer 与 AttendenceClient 两个目标
# ============================================================
cmake_minimum_required(VERSION 3.16)

project(FaceGo
    VERSION 1.0.0
    DESCRIPTION "Qt/OpenCV/SeetaFace face recognition attendance system (client-server)"
    LANGUAGES CXX)

# ---- C++ 标准 ----
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ---- 单配置生成器默认 Release ----
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS Debug Release RelWithDebInfo MinSizeRel)
endif()

# ---- 可执行文件统一输出到 build/bin，运行时数据都放这里 ----
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# ---- Qt 自动处理 MOC / UIC / RCC ----
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# ---- 依赖：Qt6（两个目标都用）----
find_package(Qt6 6.5 REQUIRED COMPONENTS Widgets Network Sql)

# ---- 依赖：OpenCV（两个目标都用）----
# 若 CMake 找不到，配置时追加：-DOpenCV_DIR=<OpenCVConfig.cmake 所在目录>
find_package(OpenCV REQUIRED)

# ---- 公共头目录（apppaths.h）----
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/common")

# ---- 两个子工程 ----
add_subdirectory(AttendenceServer)
add_subdirectory(AttendenceClient)

# ---- 汇总信息 ----
message(STATUS "FaceGo ${PROJECT_VERSION} | Qt ${Qt6_VERSION} | OpenCV ${OpenCV_VERSION} | build=${CMAKE_BUILD_TYPE}")
