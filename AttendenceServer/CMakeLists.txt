# ============================================================
#  考勤服务端 AttendenceServer
#  Qt Widgets/Network/Sql + OpenCV + SeetaFace
# ============================================================

# ---- Qt6 Sql（仅服务端使用；顶层只查找公共的 Widgets/Network）----
find_package(Qt6 6.5 REQUIRED COMPONENTS Sql)

# ---- SeetaFace（仅服务端需要）----
# 若找不到，配置时追加：-DCMAKE_PREFIX_PATH=<SeetaFace 安装根目录>（其下含 lib/cmake/SeetaFaceConfig.cmake）
find_package(SeetaFace REQUIRED COMPONENTS
    SeetaNet
    SeetaFaceDetector
    SeetaFaceLandmarker
    SeetaFaceRecognizer)

# ---- 源文件 ----
set(SERVER_SOURCES
    main.cpp
    database.cpp
    faceobject.cpp
    logindialog.cpp
    serverwindow.cpp)

set(SERVER_HEADERS
    database.h
    faceobject.h
    logindialog.h
    serverwindow.h)

set(SERVER_FORMS
    logindialog.ui
    serverwindow.ui)

set(SERVER_RESOURCES
    src.qrc)

add_executable(AttendenceServer
    ${SERVER_SOURCES}
    ${SERVER_HEADERS}
    ${SERVER_FORMS}
    ${SERVER_RESOURCES})

# ---- 头文件搜索路径 ----
# OpenCV 为老式布局（include/opencv2/opencv.hpp），需同时加入 include 与 include/opencv2
target_include_directories(AttendenceServer PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${OpenCV_INCLUDE_DIRS}
    "${OpenCV_INCLUDE_DIRS}/opencv2")

# ---- 链接 ----
target_link_libraries(AttendenceServer PRIVATE
    Qt6::Widgets
    Qt6::Network
    Qt6::Sql
    ${OpenCV_LIBS}
    SeetaFace::SeetaNet
    SeetaFace::SeetaFaceDetector
    SeetaFace::SeetaFaceLandmarker
    SeetaFace::SeetaFaceRecognizer)

# ---- SeetaFace 模型目录 ----
# 默认为 SeetaFace 安装目录下的 bin/model；可用 -DSEETAFACE_MODEL_DIR=... 覆盖
if(NOT SEETAFACE_MODEL_DIR)
    foreach(candidate "../../../bin/model" "../../bin/model" "../bin/model")
        get_filename_component(_candidate "${SeetaFace_DIR}/${candidate}" ABSOLUTE)
        if(EXISTS "${_candidate}/fd_2_00.dat")
            set(SEETAFACE_MODEL_DIR "${_candidate}")
            break()
        endif()
    endforeach()
endif()
set(SEETAFACE_MODEL_DIR "${SEETAFACE_MODEL_DIR}" CACHE PATH "SeetaFace 模型目录（含 fd/pd/fr .dat）")
if(NOT SEETAFACE_MODEL_DIR)
    message(WARNING "未自动定位到 SeetaFace 模型目录，运行时将回退到可执行文件旁的 model/ 子目录；可用 -DSEETAFACE_MODEL_DIR=... 指定。")
endif()
target_compile_definitions(AttendenceServer PRIVATE SEETAFACE_MODEL_DIR="${SEETAFACE_MODEL_DIR}")

# ---- IDE 逻辑分组（物理文件不移动，零破坏）----
source_group("Forms" FILES ${SERVER_FORMS})
source_group("Resources" FILES ${SERVER_RESOURCES})
