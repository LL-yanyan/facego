#ifndef SERVERWINDOW_H
#define SERVERWINDOW_H

#include <QMainWindow>
#include "logindialog.h"
#include "database.h"
#include "faceobject.h"
#include "database.h"
#include <QTcpServer>
#include <QList>
#include <QTcpSocket>
#include <QTimer>
#include <QLabel>
#include <QStandardItemModel>

// 工作模式
enum OPERATING_MODE
{
    MODE_MONITOR,        // 实时画面模式
    MODE_REGISTER,       // 注册模式
    MODE_RECOGNITION     // 人脸识别模式
};

#define SERVER_PORT 45678

QT_BEGIN_NAMESPACE
namespace Ui {
class ServerWindow;
}
QT_END_NAMESPACE

class ServerWindow : public QMainWindow
{
    Q_OBJECT

signals:
    void queryFace(cv::Mat &,QTcpSocket*);
    void registerFace(cv::Mat &);

public:
    explicit ServerWindow(QWidget *parent = nullptr);
    ~ServerWindow() override;
    bool init();    // 初始化服务器
    void closeServer(); // 关闭服务器

public slots:
    void slotLoginSuccess(QString id);
    void slotNewConnection();
    void slotReadyRead();
    void slotDisconnected();
    void slotGetSystime();
    void slotSendAttendanceResult(int64_t,QTcpSocket *);

private slots:
    void on_btn_home_clicked();

    void on_btn_employee_register_clicked();

    void on_btn_employees_management_clicked();

    void on_btn_department_management_clicked();

    void on_btn_attendance_record_clicked();

    void on_btn_home_show_clicked();

    void on_btn_home_close_clicked();

    void on_btn_regist_open_camera_clicked();

    void on_btn_regist_close_camera_clicked();

    void on_btn_regist_take_phtoto_clicked();

    void on_btn_regist_reset_clicked();

    void on_btn_regist_cancel_clicked();

    void on_btn_regist_ok_clicked();

    void on_btn_DM_add_department_clicked();

    void on_btn_DM_delete_department_clicked();

    void on_btn_DM_add_position_clicked();

    void on_btn_DM_delete_position_clicked();

    void on_comboBox_DM_department_currentTextChanged(const QString &arg1);

    void on_comboBox_regist_department_currentTextChanged(const QString &arg1);


    void on_btn_EM_refresh_clicked();

private:
    Ui::ServerWindow *ui;
    LoginDialog *login; // 登录界面对象指针
    DataBase &db = DataBase::getInstance();
    QTcpServer *server;
    QList<QTcpSocket*> list_client; // 所有客户端套接字列表
    FaceObject *face_object;    // 人脸对象指针
    QTimer *timer_systime;  // 定时器对象：获取系统时间
    QLabel *label_systime;  // 标签对象：用来显示系统时间
    QLabel *label_user_name;    // 标签对象：用来显示当前管理员姓名
    QStandardItemModel *t_employee_model;   // 员工信息表模型
    QStandardItemModel *t_attendance_model; // 考勤记录表模型
    QByteArray photo_data;  // 用来保存注册时的实时画面数据
    OPERATING_MODE curr_mode;
    OPERATING_MODE client_mode;

private:
    bool is_monitor;    // 判断是否需要显示实时画面
    bool is_show_register_pic;  // 判断是否需要将画面显示到注册窗口
    bool hase_take_photo;   // 用来记录用户是否拍了照
};
#endif // SERVERWINDOW_H
