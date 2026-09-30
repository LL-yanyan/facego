<div align="center">

# FaceGo 智勤 · Face Recognition Attendance System

**Qt / C++ 实现的客户端-服务器（C/S）架构人脸识别考勤系统**

A client-server face recognition attendance system built with C++17, Qt 6, OpenCV and SeetaFace2.

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://isocpp.org/)
[![Qt 6](https://img.shields.io/badge/Qt-6.8-41CD52.svg)](https://www.qt.io/)
[![OpenCV](https://img.shields.io/badge/OpenCV-4.5-darkred.svg)](https://opencv.org/)
[![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux-lightgrey.svg)](#)

</div>

---

## 项目截图

> 把运行截图放到 `docs/screenshots/` 后，替换下面的占位路径。建议放：服务端主界面、员工注册、客户端识别成功，各一张；有条件再录一段 GIF。

| 服务端主界面 | 员工人脸注册 | 客户端识别 |
| :---: | :---: | :---: |
| ![server](docs/screenshots/server.png) | ![register](docs/screenshots/register.png) | ![client](docs/screenshots/client.png) |

---

## 功能特性 Features

- **人脸注册与 1:N 识别**：基于 SeetaFace2 提取人脸特征，OpenCV 完成摄像头采集与预处理，支持 1:N 比对，相似度阈值可配
- **多终端并发接入**：服务端基于 `QTcpServer / QTcpSocket`，同时接入多台考勤客户端
- **自定义应用层协议**：8 字节固定帧头（长度字段），“先读长度、后收数据”的分包机制，解决 TCP 粘包 / 半包
- **多线程架构**：人脸处理对象 `FaceObject` 设计为可挂载到独立工作线程（`moveToThread`），识别结果通过信号回传 UI（当前默认在主线程同步执行，工作线程的启用与改进见架构文档“已知局限”）
- **数据持久化**：SQLite 设计用户、部门、岗位、员工、考勤多张数据表，数据库模块以单例模式封装，提供完整增删改查接口
- **登录与权限管理**：管理员登录、账号校验、记住密码 / 自动登录、密码找回
- **跨平台构建**：CMake 统一构建，支持 Windows（MinGW）与 Linux

## 技术栈 Tech Stack

| 分类 | 技术 |
| --- | --- |
| 语言 / 标准 | C++17 |
| GUI 框架 | Qt 6（Widgets、Network、Sql） |
| 计算机视觉 | OpenCV 4.5.x（图像采集 / 预处理 / 级联分类器） |
| 人脸识别 | SeetaFace2（FaceDetector / FaceLandmarker / FaceDatabase） |
| 网络通信 | TCP，`QTcpServer` / `QTcpSocket`，自定义帧协议 |
| 数据库 | SQLite（Qt Sql 模块） |
| 构建 | CMake（推荐）/ qmake（保留 `.pro`） |

## 系统架构 Architecture

```
 考勤客户端 A ┐
 考勤客户端 B ├── TCP（8字节帧头 / 解决粘包）──▶ 考勤服务端
 考勤客户端 C ┘                    ├── 网络模块  QTcpServer（多终端并发 + 协议解析）
                                   ├── 人脸模块  SeetaFace2 + OpenCV（子线程）
                                   └── 数据模块  SQLite（单例封装，多表关联）
```

- **AttendenceClient**：摄像头采集人脸 → 级联分类器框选 → 压缩为 JPEG → 按帧协议发送；接收并展示识别结果
- **AttendenceServer**：监听并管理多个客户端 → 子线程完成人脸注册 / 识别 → 查询数据库 → 以 JSON 返回员工信息

详见 [docs/architecture.md](docs/architecture.md)。

## 目录结构 Project Layout

```
facego/
├── CMakeLists.txt              # 顶层 CMake（统一构建两个目标）
├── common/
│   └── apppaths.h              # 集中管理可移植路径（消除硬编码）
├── AttendenceClient/           # 考勤客户端
│   ├── CMakeLists.txt
│   ├── main.cpp
│   ├── attendencewindow.*      # 考勤主窗口（采集 / 识别 / 通信）
│   ├── attendencewindow.ui
│   ├── src.qrc
│   └── imgs/
├── AttendenceServer/           # 考勤服务端
│   ├── CMakeLists.txt
│   ├── main.cpp
│   ├── logindialog.*           # 登录 / 注册 / 找回密码
│   ├── serverwindow.*          # 服务端主窗口（终端管理 / 记录）
│   ├── database.*              # SQLite 封装（单例）
│   ├── faceobject.*            # SeetaFace2 检测 / 特征 / 比对封装
│   ├── *.ui
│   ├── src.qrc
│   └── imgs/
├── docs/
│   ├── architecture.md
│   └── screenshots/
├── .github/workflows/build.yml  # CI：Linux + Windows 构建
├── CHANGELOG.md
├── LICENSE
└── .gitignore
```

## 环境依赖 Prerequisites

| 依赖 | 版本 | 说明 |
| --- | --- | --- |
| Qt | 6.5+（开发于 6.8） | 需包含 Widgets、Network、Sql 模块 |
| OpenCV | 4.5.x | 需提供 CMake 配置（`OpenCVConfig.cmake`） |
| SeetaFace2 | 2.x | 需提供 CMake 配置与模型文件（仅服务端） |
| CMake | 3.16+ | 构建系统 |
| 编译器 | MinGW 13+（Windows）/ GCC（Linux） | 与 Qt / OpenCV ABI 一致 |

> OpenCV 与 SeetaFace2 的第三方库、DLL 及模型文件体积较大，**不随仓库提供**，需自行下载配置。

## 快速开始 Quick Start

### 1. 配置（Configure）

**Windows（MinGW，PowerShell）**

```powershell
# 让 CMake 找到 Qt / OpenCV / SeetaFace
cmake -S . -B build -G "MinGW Makefiles" `
  -DCMAKE_PREFIX_PATH="D:/Qt2/6.8.3/mingw_64;D:/opencv/SeetaFace" `
  -DOpenCV_DIR="D:/opencv/opencv452/x64/mingw/lib" `
  -DCMAKE_BUILD_TYPE=Release
```

**Linux**

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
```

### 2. 编译（Build）

```bash
cmake --build build -j
```

产物输出在 `build/bin/`。

### 3. 运行（Run）

1. 先运行 **AttendenceServer**（首次启动会在 exe 旁自动创建 `database.db`），完成管理员登录；
2. 再运行一个或多个 **AttendenceClient**，客户端会自动连接服务端（默认 `127.0.0.1:45678`）；
3. Windows 下若提示找不到 DLL，把 Qt、OpenCV、SeetaFace 的 `bin` 目录加入 `PATH`。

## 路径与配置说明

为保证可移植性，源码中**没有任何硬编码绝对路径**，统一由 `common/apppaths.h` 管理：

- **可写运行时数据**（`database.db`、`face.db`、`login.ini`、`user_imgs/`）默认放在可执行文件所在目录；
- **SeetaFace 模型目录**由 CMake 自动从 SeetaFace 安装位置推导（`<SeetaFace>/bin/model`），也可用 `-DSEETAFACE_MODEL_DIR=...` 指定；
- **OpenCV 级联分类器目录**自动推导（`<OpenCV>/etc/haarcascades`），也可用 `-DOPENCV_HAARCASCADE_DIR=...` 指定。

## 网络协议（简述）

- 帧格式：`[8 字节数据长度（quint64, 大端序）] + [JPEG 图像负载]`
- 模式指令（服务端 → 客户端）：`RECOGNITION` / `REGISTER` / `MONITOR`
- 识别结果（服务端 → 客户端，JSON）：

```json
{ "id": "2612340139", "name": "张三", "department": "技术部", "position": "软件工程师" }
```

## Roadmap

- [ ] 补充单元测试与最小 CI 覆盖率
- [ ] 服务端识别结果自动写入考勤表（含上下班类型判定）
- [ ] 模型 / 数据库路径支持运行时配置文件
- [ ] 提供 Linux 下完整构建脚本与 Docker 开发环境

## Changelog

见 [CHANGELOG.md](CHANGELOG.md)。

## License

基于 [MIT License](LICENSE) 开源。

## 致谢 Acknowledgements

- [SeetaFace2](https://github.com/seetafaceengine/SeetaFace2)
- [OpenCV](https://opencv.org/)
- [Qt](https://www.qt.io/)
