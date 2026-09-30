#include "attendencewindow.h"
#include "apppaths.h"
#include "ui_attendencewindow.h"
#include <QDateTime>
#include <QJsonParseError>
#include <QJsonDocument>
#include <QJsonObject>
#include <QHostAddress>
AttendenceWindow::AttendenceWindow(QWidget *parent)
    : QMainWindow(parent),recognition_finished(true),is_recognition(true)
    , ui(new Ui::AttendenceWindow)
{
    ui->setupUi(this);
    // 实例化一个套接字对象
    socket = new QTcpSocket(this);
    // 初始化客户端
    if(!init())
    {
        exit(-1);
    }
}

AttendenceWindow::~AttendenceWindow()
{
    delete ui;
}

bool AttendenceWindow::init()
{
    // 设置窗口标题
    setWindowTitle("粤嵌考勤管理系统 V1.0");
    // 设置窗口图标
    setWindowIcon(QIcon("://imgs/window_icon.jpeg"));
    // 实例化一个视频采集对象
    cap = new cv::VideoCapture;
    // 打开摄像头，如果是在GEC6818开发板，设备文件路径名为/dev/video7
    if(!cap->open(0))
    {
        qDebug() << "打开摄像头失败！";
        return false;
    }
    // 初始化定时器，用来获取当前系统时间
    timer_systime = new QTimer(this);
    timer_systime->setInterval(1000);
    connect(timer_systime,SIGNAL(timeout()),this,SLOT(slotGetSystime()));
    timer_systime->start();
    slotGetSystime();
    // 初始化定时，用来获取摄像头画面数据(100ms刷新一次)
    startTimer(100);
    // 初始化级联分类器(绑定级联分类器对象与级联分类模型文件)
    cascade.load(apppaths::cascadeFile("haarcascade_frontalface_default.xml").toStdString());
    // 隐藏识别结果图标以及识别结果文本框
    ui->label_result_icon->hide();
    ui->label_result_text->hide();
    // 实例化定时器对象，用来清理识别结果
    timer_clear = new QTimer(this);
    timer_clear->setInterval(3000);
    // 连接信号与槽：当需要清理识别结果的时候，收到信号timerout，执行对应槽函数
    connect(timer_clear,SIGNAL(timeout()),this,SLOT(slotClearResult()));
    // 初始化定时器，用来在断开连接以后自动连接服务器
    timer_connect_to_server = new QTimer(this);
    timer_connect_to_server->setInterval(3000); // 每3秒钟尝试连接一次服务器
    // 连接信号与槽：当收到定时连接服务器的信号以后，就去连接服务器
    connect(timer_connect_to_server,SIGNAL(timeout()),this,SLOT(slotConnectToServer()));
    // 连接信号与槽：当断开连接的时候，会收到disconnected()信号，启动定时器，自动连接服务器
    connect(socket,SIGNAL(disconnected()),this,SLOT(slotDisconnected()));
    // 连接信号与槽：当连接成功的时候，会收到connected()信号，停止定时器
    connect(socket,SIGNAL(connected()),this,SLOT(slotConnected()));
    // 连接信号与槽：服务发送数据过来，会收到readyRead()信号，处理数据
    connect(socket,SIGNAL(readyRead()),this,SLOT(slotReadyRead()));
    //如果客户端没有连接到服务器，就启动定时器来自动连接服务器
    if(socket->state() != QAbstractSocket::ConnectedState)
    {
        slotConnectToServer();
    }
    return true;
}

void AttendenceWindow::slotGetSystime()
{
    // 获取系统时间并显示到界面上去
    ui->label_time->setText(QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss dddd"));
}

void AttendenceWindow::timerEvent(QTimerEvent *e)
{
    // 显示摄像头画面
    cv::Mat frame;  // 用来保存获取到的画面数据
    if(!cap->isOpened())
    {
        qDebug() << "摄像头没有打开！";
        return;
    }
    // 读取一帧画面
    if(!cap->read(frame))
    {
        qDebug() << "读取摄像头画面数据失败！";
        return;
    }
    // 判断画面有没有数据
    if(frame.data == nullptr)
    {
        qDebug() << "获取到的画面数据为空！";
        return;
    }
    // 检测画面中的人脸
    // 将画面转换为灰度图
    cv::Mat frame_gary; // 用来存储转换后的灰度图数据
    cv::cvtColor(frame,frame_gary,cv::COLOR_BGR2GRAY);
    std::vector<cv::Rect> faces;    // 用来保存检测到的人脸位置信息(矩形)，可以检测多个人脸
    cascade.detectMultiScale(frame_gary,faces);
    if(faces.size() > 0)
    {
        // 检测到人脸了，至少检测到一个人脸，遍历容器，标记每个人脸的位置
        for(int i = 0; i < faces.size(); i++)
        {
            // 获取一个人脸矩形区域数据
            cv::Rect rect = faces.at(i);
            // 在画面上将这个矩形画出来
            // cv::rectangle(frame,rect,cv::Scalar(0,0,255));
            // 将画面上的人脸框移动到人脸的位置
            ui->label_face_area->move(rect.x-20,rect.y+rect.height/2);
            // 对人脸框进行等比例缩放
            ui->label_face_area->resize(rect.width+40,rect.height+40);
        }
        // 将含有人脸数据的画面发送给服务器
        // 如果服务器处理完成了前面发送的数据，那么就发送新的数据，否则不发送
        if(recognition_finished or !is_recognition)
        {
            // 显示文本框：识别中...
            // 将当前画面发送给服务器
            // 将opencv::Mat数据转换为Qt::QByteArray数据，再发给服务器
            std::vector<uchar> buf; // 用来保存转换后的结果
            // 判断服务器模式，如果是人脸识别模式，那就发送灰度图，否则发送彩色图
            if(is_recognition)
            {
                // 人脸识别模式，发送灰度图数据
                ui->label_result_text->setText("识别中...");
                ui->label_result_text->show();
                // 将要发送的数据按照指定的格式进行编码
                cv::imencode(".jpg",frame_gary,buf);
            }
            else
            {
                // 注册模式或者监控模式，发送彩色图
                cv::imencode(".jpg",frame,buf);
            }
            // 将画面数据转换为二进制数据
            QByteArray byte_data((const char*)buf.data(),buf.size());
            // 计算数据包的大小
            quint64 data_size = byte_data.size();
            QByteArray send_data;   // 存放最终要发送的数据
            // 构建一个数据流对象(方便添加数据头等信息 -> 打包数据)，并绑定到字节数据对象
            QDataStream data_stream(&send_data,QIODevice::WriteOnly);
            // 考虑到服务器版本兼容性的问题，在打包数据的时候，指定Qt的版本
            data_stream.setVersion(QDataStream::Qt_6_8);
            // 打包数据：将数据大小和有效数据拼起来
            data_stream << data_size << byte_data;
            qDebug() << "send_data_size:" << data_size+8;
            // 发送数据
            socket->write(send_data);
            // 将服务器处理数据的状态设置为还没处理完
            recognition_finished = false;
        }
    }
    else
    {
        // 画面中没有检测到人脸
        // qDebug() << "画面中没有检测到人脸！";
        // 将画面中的人脸框恢复到原来的位置
        ui->label_face_area->move(210,180);
        ui->label_face_area->resize(220,220);
    }
    // 将画面显示到界面上去
    ui->label_movie->setPixmap(QPixmap::fromImage(QImage(frame.data,frame.cols,frame.rows,frame.step1(),QImage::Format_BGR888)));
}

void AttendenceWindow::slotConnectToServer()
{
    // 连接服务器
    socket->connectToHost(QHostAddress(SERVER_IP),SERVER_PORT);
    // 将连接状态显示到界面上去
    ui->label_result_icon->hide();  // 隐藏识别结果图标
    ui->label_result_text->setText("连接中...");
    ui->label_result_text->show();
    qDebug() << "ConnectToServer!";
}

void AttendenceWindow::slotDisconnected()
{
    // 与服务器断开连接：启动定时器，每3秒钟重连一次，直到连接成功
    slotConnectToServer();
    timer_connect_to_server->start();
    qDebug() << "Disconnected!";
}

void AttendenceWindow::slotConnected()
{
    // 与服务器连接成功
    // 隐藏识别结果文本框
    ui->label_result_text->hide();
    // 停止定时器
    timer_connect_to_server->stop();
    qDebug() << "Connected!";
}

void AttendenceWindow::slotReadyRead()
{
    // 服务器回数据了，就认为服务器已经处理完发送的数据了
    recognition_finished = true;
    // 获取服务器发送过来的数据
    QByteArray data = socket->readAll();
    qDebug() << "recived:" << data;
    // 判断服务器回复过来的指令类型
    if(QString(data).contains("REGISTER") or QString(data).contains("MONITOR"))
    {
        // 注册模式或者监控模式
        is_recognition = false;
        qDebug() << "注册模式或者监控模式！";
        return;
    }
    else if(QString(data).contains("RECOGNITION"))
    {
        // 识别模式
        is_recognition = true;
        qDebug() << "识别模式！";
        return;
    }
    // 判断识别结果，如果返回的用户ID为-1，表示没有识别到该用户的信息
    // 使用Json格式来解析数据
    QJsonParseError err;    // 用来保存转换失败的错误信息
    QJsonDocument doc = QJsonDocument::fromJson(data,&err);
    if(err.error != QJsonParseError::NoError)
    {
        qDebug() << "Json数据解析失败：" << err.errorString();
        qDebug() << QString(data);
        return;
    }
    // 启动定时器，清除识别结果
    timer_clear->start();
    // 提取员工信息
    QJsonObject json_obj = doc.object();
    // {\"id\":\"2612340139\",\"name\":\"张飞\",\"department\":\"技术部\",\"position\":\"软件开发工程师\"}
    QString id = json_obj.value("id").toString();
    // 判断是否识别成功
    if(id == "-1")
    {
        qDebug() << "认证失败！";
        // 显示认证失败的标签
        ui->label_result_icon->setStyleSheet("border-image: url(:/imgs/no.png);");
        ui->label_result_icon->show();
        ui->label_result_text->setText("认证失败");
        ui->label_result_text->show();
        return;
    }
    // 识别成功，显示员工信息
    ui->lineEdit_id->setText(id);
    ui->lineEdit_department->setText(json_obj.value("department").toString());
    ui->lineEdit_position->setText(json_obj.value("position").toString());
    ui->lineEdit_name->setText(json_obj.value("name").toString());
    ui->lineEdit_time->setText(QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss"));
    // 显示认证结果
    ui->label_result_icon->setStyleSheet("border-image: url(:/imgs/ok.png);");
    ui->label_result_icon->show();
    ui->label_result_text->setText("认证成功");
    ui->label_result_text->show();
    // 显示该员工的图片
    // ...
}

void AttendenceWindow::slotClearResult()
{
    // 认证完成了，清理认证结果
    ui->label_usr_pic->clear();
    ui->lineEdit_id->clear();
    ui->lineEdit_department->clear();
    ui->lineEdit_position->clear();
    ui->lineEdit_name->clear();
    ui->lineEdit_time->clear();
    // 隐藏识别结果图标和文本框
    ui->label_result_icon->hide();
    ui->label_result_text->hide();
    // 停止定时器
    timer_clear->stop();
}
