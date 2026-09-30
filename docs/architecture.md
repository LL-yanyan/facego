# FaceGo 架构设计文档

本文描述 FaceGo 智勤人脸识别考勤系统的整体架构、模块职责、网络协议、人脸处理流程、数据库设计与线程模型，并如实记录当前实现的已知局限与改进方向。

- 语言 / 标准：C++17
- GUI / 网络 / 数据库：Qt 6（Widgets、Network、Sql）
- 视觉 / 识别：OpenCV 4.5、SeetaFace2
- 形态：客户端-服务器（C/S），TCP 通信

## 1. 总体架构

```
 AttendenceClient A ┐
 AttendenceClient B ├──(TCP, 端口 45678)──▶ AttendenceServer
 AttendenceClient C ┘
       │                                        │
       │ 摄像头采集 / 级联框选 / JPEG 编码        ├─ 网络模块：QTcpServer，多连接管理 + 帧解析
       │                                        ├─ 人脸模块：FaceObject（SeetaFace2 + OpenCV）
       │                                        └─ 数据模块：DataBase（SQLite，单例）
```

- 客户端只负责**采集、框选、编码、发送、展示**，不做特征比对与数据存储；
- 服务端集中处理**终端管理、人脸注册 / 识别、业务数据持久化**，并以 JSON 返回结果。

## 2. 模块划分

### 2.1 客户端 AttendenceClient

| 类 / 文件 | 职责 |
| --- | --- |
| `AttendenceWindow` | 考勤主窗口，聚合摄像头、定时器、TCP socket 与界面 |
| `main.cpp` | 客户端程序入口 |

关键行为：

1. `cv::VideoCapture` 打开默认摄像头（`open(0)`）；
2. 通过 `startTimer(100)` 每 100ms 在 `timerEvent` 中取一帧；
3. 转灰度后用 OpenCV 级联分类器 `detectMultiScale` 检测人脸，移动 / 缩放界面上的人脸框；
4. 将画面 `imencode(".jpg")` 编码，按帧协议发送；
5. 通过模式标志 `is_recognition` 决定发送灰度图（识别）还是彩色图（注册 / 监控）；
6. 接收服务端指令切换模式，接收 JSON 结果并展示，断线后每 3s 自动重连。

### 2.2 服务端 AttendenceServer

| 类 / 文件 | 职责 |
| --- | --- |
| `LoginDialog` | 管理员登录、账号注册、密码找回、记住密码 / 自动登录 |
| `ServerWindow` | 服务端主窗口：监听端口、管理客户端连接、模式切换、员工 / 部门 / 岗位 / 考勤界面 |
| `FaceObject` | 封装 SeetaFace2：人脸检测、特征注册、1:N 比对、特征库读写 |
| `DataBase` | SQLite 单例封装：建表与各业务表的增删改查 |
| `main.cpp` | 服务端程序入口 |

## 3. 网络通信设计

### 3.1 帧格式（解决 TCP 粘包 / 半包）

```
┌──────────────────────────┬─────────────────────────────┐
│ 帧头：8 字节数据长度      │ 负载：JPEG 图像数据           │
│ quint64，QDataStream 大端 │ 长度 = 帧头给出的字节数        │
└──────────────────────────┴─────────────────────────────┘
```

- 发送端：`QDataStream << data_size << byte_data`，并 `setVersion(QDataStream::Qt_6_8)` 保证跨版本一致；
- 接收端：**先读 8 字节长度，再按长度收满负载**，收不满则等待下一次 `readyRead`。

### 3.2 服务端接收状态机

`ServerWindow::slotReadyRead` 中用一个（当前为 `static`）变量 `data_size` 记录进度：

1. `data_size == 0`：若可读字节不足 8，返回等待；否则读出长度；
2. 若 `bytesAvailable() < data_size`，负载未收全，返回等待；
3. 收满后读出整帧 `frame_data`，将 `data_size` 复位为 0，进入业务处理。

### 3.3 模式切换与消息类型

| 方向 | 消息 | 含义 |
| --- | --- | --- |
| 服务端 → 客户端 | `RECOGNITION` | 切换到人脸识别模式（发灰度图） |
| 服务端 → 客户端 | `REGISTER` | 切换到注册模式（发彩色图） |
| 服务端 → 客户端 | `MONITOR` | 切换到实时监控模式（发彩色图） |
| 服务端 → 客户端 | JSON | 识别结果 |

服务端根据当前所在页面 / 状态确定 `curr_mode`，与客户端当前模式不一致时下发对应指令。

识别结果 JSON 示例：

```json
{ "id": "2612340139", "name": "张三", "department": "技术部", "position": "软件工程师" }
```

识别失败时 `id` 为 `"-1"`。

> 已知局限：`data_size` 使用 `static` 局部变量，在**多客户端同时接入**时会被多个连接共享，造成长度状态串扰。正确做法是把“每连接的接收状态”绑定到对应 socket（如用 `QObject` 子类或 `QMap<QTcpSocket*, RecvState>`）。

## 4. 人脸处理流程

### 4.1 SeetaFace2 组件

`FaceObject` 组合使用：

- **FaceDetector**：人脸检测，输出人脸位置；
- **FaceLandmarker**：关键点定位（5 点 / 81 点模型）；
- **FaceRecognizer / FaceDatabase**：特征提取、特征注册与 1:N 比对。

模型文件：`fd_2_00.dat`（检测）、`pd_2_00_pts5.dat`（关键点）、`fr_2_10.dat`（识别）。

### 4.2 注册流程

1. 服务端处于注册页并开启摄像头，客户端以彩色图持续上报；
2. 拍照得到 `face_img`，调用 `FaceObject::faceRegister`；
3. SeetaFace 检测 / 定位 / 提取特征，并将特征注册进 FaceDatabase，返回 `face_id`；
4. 注册照片保存到 `user_imgs/<工号>.jpg`；
5. 将员工业务信息连同 `face_id` 写入 `t_employees`。

### 4.3 识别流程（1:N）

1. 识别模式下客户端上报灰度 JPEG；
2. 服务端解码为 `cv::Mat`，触发 `queryFace`；
3. `FaceObject::faceQuery` 提取当前人脸特征，与特征库中已注册特征逐一计算相似度；
4. 取最高相似度，**阈值为 0.8**：达到阈值则返回对应 `face_id`，否则返回 -1；
5. 服务端据 `face_id` 查 `t_employees`，以 JSON 返回工号 / 姓名 / 部门 / 岗位。

### 4.4 特征库持久化

注册得到的人脸特征通过 SeetaFace 的 `Save` 持久化到运行时数据文件 `face.db`，启动时 `Load` 恢复，避免每次重启重新录入。

## 5. 数据库设计

### 5.1 单例封装

`DataBase` 通过 `getInstance()` 返回静态单例，统一持有数据库连接与所有业务接口。

### 5.2 数据表

**t_user（系统用户 / 管理员）**

| 字段 | 类型 | 约束 |
| --- | --- | --- |
| id | varchar(12) | 主键 |
| passwd | varchar(12) | 非空 |
| name | varchar(30) | 非空 |
| id_card | char(18) | 唯一、非空 |
| gender | char(3) | 非空 |
| age | smallint | 非空 |
| tel | char(11) | 唯一、非空 |
| icon_path | varchar | - |

**t_department（部门）**：`num` integer 主键自增；`department` varchar 唯一非空。

**t_position（岗位）**

| 字段 | 类型 | 约束 |
| --- | --- | --- |
| num | integer | 主键自增 |
| department_id | integer | 外键 → t_department(num)，级联更新 / 删除 |
| department | varchar | 非空 |
| position | varchar | 非空 |

**t_employees（员工）**

| 字段 | 类型 | 约束 |
| --- | --- | --- |
| id | char(10) | 主键 |
| department / position / name | varchar | 非空 |
| id_card | char(18) | 唯一、非空 |
| gender | char(3) | 非空 |
| age | smallint | 非空 |
| birthday | date | 非空 |
| nation / political / education / college | varchar | 非空 |
| tel | char(11) | 唯一、非空 |
| join_date | date | 非空 |
| face_id | integer | 非空 |

**t_attendance（考勤记录）**

| 字段 | 类型 | 约束 |
| --- | --- | --- |
| num | integer | 主键自增 |
| employee_id | char(11) | 外键 → t_employees(id)，级联更新 / 删除 |
| time | datetime | 非空 |
| type | varchar | 非空 |
| state | varchar | 非空 |

### 5.3 关系

```
t_department(1) ──< t_position(N)
t_employees (N) ──< t_attendance(N)
```

建表前通过 `pragma foreign_keys = ON;` 开启外键，岗位 / 考勤表配置级联更新与删除。

## 6. 线程模型（含已知局限）

设计意图：把耗时的人脸计算从 GUI 线程剥离。`init()` 中创建了 `QThread` 并执行 `face_object->moveToThread(thread)`。

**当前实际实现存在两处问题，人脸计算并未真正进入子线程：**

1. 创建的 `QThread` 没有调用 `start()`，工作线程未运行事件循环；
2. `queryFace` 信号以 `Qt::DirectConnection` 连接，强制槽函数在**发射信号的 GUI 线程**同步执行。

因此 `faceQuery` 当前在主线程同步运行，画面帧率较高时会造成可感知的卡顿；其“好处”是识别结果可立即在同一上下文中通过 socket 返回。

**改进方向：**

- 调用 `thread->start()` 启动工作线程；
- 将 `queryFace` 改为默认 / 队列连接（或 `BlockingQueuedConnection` 配合流控），使人脸计算在 worker 线程执行；
- 计算完成后通过已有信号 `sendFaceID`（跨线程时为队列连接）回传 GUI 线程发送结果；
- 同时用“每连接接收状态”替换 `static data_size`。

## 7. 路径与可移植性

历史代码在源码与 `.pro` 中硬编码了开发机绝对路径，他人克隆后无法编译 / 运行。现做两层处理：

- 构建层：CMake 自动 `find_package` Qt / OpenCV / SeetaFace，并推导模型与级联目录，支持 `-D` 覆盖；
- 运行层：`common/apppaths.h` 集中管理路径，源码不再出现任何绝对路径。
  - 可写数据（`database.db`、`face.db`、`login.ini`、`user_imgs/`）位于可执行文件目录；
  - 模型 / 级联目录优先使用 CMake 编译定义，缺失时回退到可执行文件旁子目录。

## 8. 构建与部署

见根目录 [README.md](../README.md) 的“快速开始”。简述：

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release   # 配置
cmake --build build -j                            # 编译，产物在 build/bin
```

部署时需保证 Qt / OpenCV / SeetaFace 的运行时 DLL（或 `.so`）与模型文件可被找到。

## 9. 已知局限汇总

1. 多客户端共享 `static data_size`，存在接收状态串扰；
2. 工作线程未 `start()` 且 `queryFace` 为 `DirectConnection`，人脸计算实际在主线程；
3. 识别成功后的考勤记录写入逻辑尚未完成（`slotSendAttendanceResult` 中为 TODO）；
4. 客户端服务器地址 `SERVER_IP` 仍为编译期常量，暂不支持运行时配置；
5. SQL 全部采用字符串拼接，存在注入风险，应改为参数化查询（占位符绑定）。
