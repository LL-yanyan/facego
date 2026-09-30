#ifndef ATTENDENCEWINDOW_H
#define ATTENDENCEWINDOW_H

#include <QMainWindow>
#include <opencv.hpp>
#include <QTimer>
#include <QTcpSocket>

// 定义两个宏：服务器的IP地址和服务器的端口号
#define SERVER_IP   "127.0.0.1"
#define SERVER_PORT 45678

QT_BEGIN_NAMESPACE
namespace Ui {
class AttendenceWindow;
}
QT_END_NAMESPACE

class AttendenceWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit AttendenceWindow(QWidget *parent = nullptr);
    ~AttendenceWindow() override;
    bool init();    // 初始化客户端

public slots:
    void slotGetSystime();
    void timerEvent(QTimerEvent *e);
    void slotConnectToServer();
    void slotDisconnected();
    void slotConnected();
    void slotReadyRead();
    void slotClearResult();

private:
    Ui::AttendenceWindow *ui;
    cv::VideoCapture *cap;  // 视频采集对象(摄像头)
    QTimer *timer_systime;  // 定时器：获取系统时间
    cv::CascadeClassifier cascade;  // 级联分类器对象
    QTcpSocket *socket; // 套接字对象
    QTimer *timer_connect_to_server;    // 定时连接服务器
    QTimer *timer_clear;    // 清空识别结果

private:
    bool recognition_finished;  // 用来记录服务器是否已经识别完成
    bool is_recognition;    // 判断服务器当前是不是识别模式

};
#endif // ATTENDENCEWINDOW_H
