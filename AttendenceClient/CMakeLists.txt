# ============================================================
#  考勤客户端 AttendenceClient
#  Qt Widgets/Network + OpenCV（摄像头采集 / 人脸框选）
# ============================================================

# ---- 源文件 ----
set(CLIENT_SOURCES
    main.cpp
    attendencewindow.cpp)

set(CLIENT_HEADERS
    attendencewindow.h)

set(CLIENT_FORMS
    attendencewindow.ui)

set(CLIENT_RESOURCES
    src.qrc)

add_executable(AttendenceClient
    ${CLIENT_SOURCES}
    ${CLIENT_HEADERS}
    ${CLIENT_FORMS}
    ${CLIENT_RESOURCES})

# ---- 头文件搜索路径 ----
# OpenCV 为老式布局（include/opencv2/opencv.hpp），需同时加入 include 与 include/opencv2
target_include_directories(AttendenceClient PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${OpenCV_INCLUDE_DIRS}
    "${OpenCV_INCLUDE_DIRS}/opencv2")

# ---- 链接 ----
target_link_libraries(AttendenceClient PRIVATE
    Qt6::Widgets
    Qt6::Network
    ${OpenCV_LIBS})

# ---- OpenCV 级联分类器目录 ----
# 默认为 OpenCV 安装目录下的 etc/haarcascades；可用 -DOPENCV_HAARCASCADE_DIR=... 覆盖
if(NOT OPENCV_HAARCASCADE_DIR)
    foreach(candidate
        "../../../../share/opencv4/haarcascades"
        "../../../../share/OpenCV/haarcascades"
        "../../../../etc/haarcascades"
        "../../../etc/haarcascades"
        "../../etc/haarcascades")
        get_filename_component(_candidate "${OpenCV_DIR}/${candidate}" ABSOLUTE)
        if(EXISTS "${_candidate}/haarcascade_frontalface_default.xml")
            set(OPENCV_HAARCASCADE_DIR "${_candidate}")
            break()
        endif()
    endforeach()
endif()
set(OPENCV_HAARCASCADE_DIR "${OPENCV_HAARCASCADE_DIR}" CACHE PATH "OpenCV haarcascades 目录")
if(NOT OPENCV_HAARCASCADE_DIR)
    message(WARNING "未自动定位到 OpenCV haarcascades 目录，运行时将回退到可执行文件旁的级联文件；可用 -DOPENCV_HAARCASCADE_DIR=... 指定。")
endif()
target_compile_definitions(AttendenceClient PRIVATE OPENCV_HAARCASCADE_DIR="${OPENCV_HAARCASCADE_DIR}")

# ---- IDE 逻辑分组（物理文件不移动，零破坏）----
source_group("Forms" FILES ${CLIENT_FORMS})
source_group("Resources" FILES ${CLIENT_RESOURCES})
