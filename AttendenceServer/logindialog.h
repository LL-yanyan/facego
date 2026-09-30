#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include "database.h"
#include <QSettings>
#include <QTimer>

#define DEFAULT_ICON_PATH "://imgs/icon_default.jpeg"

namespace Ui {
class LoginDialog;
}

class LoginDialog : public QDialog
{
    Q_OBJECT

signals:
    void loginSuccess(QString id);  // 自定义信号：用户登录成功

public:
    explicit LoginDialog(QWidget *parent = nullptr);
    ~LoginDialog();
    // 初始化登录界面
    void uiInit();
    // 生成随机验证表达式
    void randVerification();
    // 加载配置信息
    void loadSettings();

private slots:
    void on_lineEdit_login_id_textChanged(const QString &arg1);

    void on_lineEdit_login_passwd_textChanged(const QString &arg1);

    void on_btn_login_register_clicked();

    void on_btn_login_retrieve_clicked();

    void on_btn_register_back_clicked();

    void on_btn_retrieve_back_clicked();

    void on_btn_register_register_clicked();

    void on_btn_register_select_icon_clicked();

    void on_btn_login_login_clicked();

    void on_btn_login_QR_clicked();

    void on_checkBox_auto_checkStateChanged(const Qt::CheckState &arg1);

    void on_checkBox_remember_checkStateChanged(const Qt::CheckState &arg1);

private:
    Ui::LoginDialog *ui;
    QString regist_icon;    // 用户注册时选择的头像
    QString user_icon;  // 用户的头像路径
    DataBase &db = DataBase::getInstance();  // 获取数据库对象
    QSettings *setting; // 配置对象指针
    QTimer *timer_auto;  // 定时器，用来延时自动登录
};

#endif // LOGINDIALOG_H
