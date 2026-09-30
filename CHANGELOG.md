# Changelog

本项目所有重要变更记录于此。格式参考 [Keep a Changelog](https://keepachangelog.com/zh-CN/1.1.0/)，版本号遵循 [语义化版本](https://semver.org/lang/zh-CN/)。

## [Unreleased]

### Added
- 单元测试与最小 CI 覆盖率
- 识别成功后自动写入考勤表（含上下班类型判定）
- 模型 / 数据库路径的运行时配置
- Linux 构建脚本与 Docker 开发环境

### Security
- 将字符串拼接 SQL 改为参数化绑定，消除注入风险

## [0.2.0] - 2026-09-30

### Added
- 顶层及子项目 CMake 构建脚本，统一输出到 `build/bin`，支持 CMake 自动查找 Qt / OpenCV / SeetaFace
- `common/apppaths.h`：集中管理可移植路径，自动创建可写数据目录
- GitHub Actions CI（Linux 构建）
- `docs/architecture.md` 架构设计文档
- MIT `LICENSE` 与本变更日志
- 模型 / 级联目录的多候选自动探测，支持 `-DSEETAFACE_MODEL_DIR`、`-DOPENCV_HAARCASCADE_DIR` 覆盖

### Changed
- 移除源码中全部硬编码绝对路径（模型、数据库、登录配置、注册照片、级联分类器）
- 重写 README，补充功能特性、技术栈、架构图、目录结构与 CMake 构建步骤
- 完善 `.gitignore`，覆盖 CMake / Qt 构建产物并明确忽略隐私数据与第三方库

## [0.1.0]

### Added
- 初始版本：Qt + OpenCV + SeetaFace2 + SQLite 的 C/S 人脸识别考勤系统
- 客户端：摄像头采集、级联分类器人脸框选、JPEG 编码、TCP 收发、结果展示、断线重连
- 服务端：多终端接入、自定义帧协议（8 字节帧头）、人脸注册与 1:N 识别、管理员登录
- 数据库：用户、部门、岗位、员工、考勤五张数据表及增删改查接口

[Unreleased]: #unreleased
[0.2.0]: #020---2026-09-30
[0.1.0]: #010
