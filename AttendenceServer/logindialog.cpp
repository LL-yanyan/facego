#include "logindialog.h"
#include "apppaths.h"
#include "ui_logindialog.h"
#include <QDebug>
#include <QMovie>
#include <QMessageBox>
#include <time.h>
#include <QDate>
#include <QFileDialog>
#include <unistd.h>

LoginDialog::LoginDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::LoginDialog)
{
    ui->setupUi(this);
    ui->lineEdit_login_id->setFocus();
    // 初始化登录界面
    uiInit();
}

LoginDialog::~LoginDialog()
{
    delete timer_auto;
    delete ui;
}

void LoginDialog::uiInit()
{
    // 默认显示登录界面
    ui->stackedWidget->setCurrentWidget(ui->page_login);
    // 设置登录界面Logo
    ui->label_login_logo->setScaledContents(true);
    ui->label_login_logo->setPixmap(QPixmap("://imgs/logo.jpeg"));
    // 设置登录界面动画
    ui->label_login_movie->setScaledContents(true);
    QMovie *movie = new QMovie("://imgs/mov1.gif");
    ui->label_login_movie->setMovie(movie);
    movie->start();
    // 设置登录界面默认头像
    ui->label_login_icon->setScaledContents(true);
    ui->label_login_icon->setPixmap(QPixmap(DEFAULT_ICON_PATH));
    // 设置注册账号界面的标题
    ui->label_register_title->setScaledContents(true);
    ui->label_register_title->setPixmap(QPixmap("://imgs/regist_logo.png"));
    // 设置注册账号界面默认头像
    ui->label_register_icon->setScaledContents(true);
    ui->label_register_icon->setPixmap(QPixmap(DEFAULT_ICON_PATH));
    // 设置找回密码界面的标题
    ui->label_retrieve_title->setScaledContents(true);
    ui->label_retrieve_title->setPixmap(QPixmap("://imgs/retrieve_password_logo.png"));
    // 实例化一个配置对象
    setting = new QSettings(apppaths::dataFile("login.ini"),QSettings::IniFormat,this);
    // 实例化定时器对象
    timer_auto = new QTimer(this);
    timer_auto->setInterval(2000);  // 自动登录延时2秒
    connect(timer_auto,&QTimer::timeout,this,&LoginDialog::on_btn_login_login_clicked);
    // 加载配置信息
    loadSettings();
}

void LoginDialog::randVerification()
{
    // 设置随机种子
    srand(time(NULL));
    // 生成随机表达式
    ui->label_register_verification->setText(QString("%1%2%3").arg(rand()%100)
        .arg(rand()%2 ? '+' : '-').arg(rand()%100));
    ui->label_retrieve_verification->setText(QString("%1%2%3").arg(rand()%100)
        .arg(rand()%2 ? '+' : '-').arg(rand()%100));
}

void LoginDialog::loadSettings()
{
    // 加载配置信息
    if(setting->value("State/remember").toBool())
    {
        // 记住密码，勾选记住密码
        ui->checkBox_remember->setChecked(true);
        // 从配置文件中获取账号密码
        ui->lineEdit_login_id->setText(setting->value("User/id").toString());
        ui->lineEdit_login_passwd->setText(setting->value("User/passwd").toString());
    }
    if(setting->value("State/auto").toBool())
    {
        // 自动登录，勾选自动登录
        ui->checkBox_auto->setChecked(true);
        timer_auto->start();
    }
}

void LoginDialog::on_lineEdit_login_id_textChanged(const QString &text)
{
    if(text.isEmpty())
    {
        // 当输入框为空的时候，显示提示文本，设置对齐策略和字体
        ui->lineEdit_login_id->setAlignment(Qt::AlignHCenter);
        ui->lineEdit_login_id->setFont(QFont("Microsoft YaHei UI",12));
    }
    else
    {
        // 当用户开始输入以后，修改对齐策略以及字体
        ui->lineEdit_login_id->setAlignment(Qt::AlignLeft);
        ui->lineEdit_login_id->setFont(QFont("Microsoft YaHei UI",15,QFont::Weight::Bold));
        if(text.length() >= 6)
        {
            // 根据账号查询用户头像
            QString icon_path = db.getUserIconPath(text);
            if(icon_path.isEmpty())
            {
                // 使用默认头像
                ui->label_login_icon->setPixmap(QPixmap(DEFAULT_ICON_PATH));
            }
            else
            {
                // 使用用户头像
                ui->label_login_icon->setPixmap(QPixmap(icon_path));
            }
        }
        else
        {
            // 使用默认头像
            ui->label_login_icon->setPixmap(QPixmap(DEFAULT_ICON_PATH));
        }
    }
}

void LoginDialog::on_lineEdit_login_passwd_textChanged(const QString &text)
{
    if(text.isEmpty())
    {
        // 当输入框为空的时候，显示提示文本，设置对齐策略和字体
        ui->lineEdit_login_passwd->setAlignment(Qt::AlignHCenter);
        ui->lineEdit_login_passwd->setFont(QFont("Microsoft YaHei UI",12));
    }
    else
    {
        // 当用户开始输入以后，修改对齐策略以及字体
        ui->lineEdit_login_passwd->setAlignment(Qt::AlignLeft);
        ui->lineEdit_login_passwd->setFont(QFont("Microsoft YaHei UI",20,QFont::Weight::Bold));
    }
}


void LoginDialog::on_btn_login_register_clicked()
{
    // 跳转到注册页面
    ui->stackedWidget->setCurrentWidget(ui->page_register);
    // 生成随机验证表达式
    randVerification();
}


void LoginDialog::on_btn_login_retrieve_clicked()
{
    // 跳转到找回密码页面
    ui->stackedWidget->setCurrentWidget(ui->page_retrieve_passwd);
    // 生成随机验证表达式
    randVerification();
}


void LoginDialog::on_btn_register_back_clicked()
{
    // 返回登录界面
    ui->stackedWidget->setCurrentWidget(ui->page_login);
    // 清空注册界面中所有的信息
    ui->lineEdit_register_id->clear();
    ui->lineEdit_register_passwd->clear();
    ui->lineEdit_register_passwd_2->clear();
    ui->lineEdit_register_name->clear();
    ui->lineEdit_register_id_card->clear();
    ui->lineEdit_register_tel->clear();
    ui->lineEdit_register_verification->clear();
    regist_icon.clear();
    ui->label_register_icon->setPixmap(QPixmap(DEFAULT_ICON_PATH));
}


void LoginDialog::on_btn_retrieve_back_clicked()
{
    // 返回登录界面
    ui->stackedWidget->setCurrentWidget(ui->page_login);
    // 清空找回密码界面中所有的信息
    ui->lineEdit_retrieve_id->clear();
    ui->lineEdit_retrieve_name->clear();
    ui->lineEdit_retrieve_id_card->clear();
    ui->lineEdit_retrieve_tel->clear();
    ui->lineEdit_retrieve_verification->clear();
}


void LoginDialog::on_btn_register_register_clicked()
{
    // 注册
    // 获取用户输入的信息
    QString id = ui->lineEdit_register_id->text();
    QString passwd = ui->lineEdit_register_passwd->text();
    QString passwd2 = ui->lineEdit_register_passwd_2->text();
    QString name = ui->lineEdit_register_name->text();
    QString id_card = ui->lineEdit_register_id_card->text();
    QString tel = ui->lineEdit_register_tel->text();
    QString verification = ui->lineEdit_register_verification->text();
    // 判断数据是否为空
    if(id.isEmpty() or passwd.isEmpty() or passwd2.isEmpty() or name.isEmpty() or id_card.isEmpty()
        or tel.isEmpty() or verification.isEmpty())
    {
        QMessageBox::warning(this,"警告","数据不能为空，请输入完整的数据！");
        return;
    }
    if(passwd != passwd2)
    {
        QMessageBox::warning(this,"警告","两次输入的密码不一致，请重新输入！");
        ui->lineEdit_register_passwd_2->clear();
        return;
    }
    if(id_card.length() != 18)
    {
        QMessageBox::warning(this,"警告","请输入正确的身份证号码！");
        return;
    }
    if(tel.length() != 11)
    {
        QMessageBox::warning(this,"警告","请输入正确的手机号码！");
        return;
    }
    // 判断验证表达式结果是否正确
    QString express = ui->label_register_verification->text();
    int correct_res;
    if(express.contains('+'))
    {
        // 加法表达式，提取两个操作数
        int opt1 = express.split('+').at(0).toInt();
        int opt2 = express.split('+').at(1).toInt();
        correct_res = opt1+opt2;
    }
    else
    {
        // 减法表达式，提取两个操作数
        int opt1 = express.split('-').at(0).toInt();
        int opt2 = express.split('-').at(1).toInt();
        correct_res = opt1-opt2;
    }
    if(verification.toInt() != correct_res)
    {
        // 验证码错误
        QMessageBox::warning(this,"警告","验证表达式计算错误，请重新计算！");
        // 重新生成验证表达式
        randVerification();
        // 清空用户输入的错误验证结果
        ui->lineEdit_register_verification->clear();
        return;
    }
    // 根据身份证号码计算出用户的年龄和性别
    // 456123200705161234   [6]-[13]：出生年月日  [16]：如果是奇数 - 男，是偶数 - 女
    // 提取出用户的出生年月日
    bool ok;
    int u_year = id_card.mid(6,4).toInt(&ok);
    if(!ok)
    {
        QMessageBox::warning(this,"警告","请输入正确的身份证号码！");
        return;
    }
    int u_month = id_card.mid(10,2).toInt(&ok);
    if(!ok)
    {
        QMessageBox::warning(this,"警告","请输入正确的身份证号码！");
        return;
    }
    int u_day = id_card.mid(12,2).toInt(&ok);
    if(!ok)
    {
        QMessageBox::warning(this,"警告","请输入正确的身份证号码！");
        return;
    }
    // 获取系统日期
    QDate c_date = QDate::currentDate();
    int c_year = c_date.year();
    int c_month = c_date.month();
    int c_day = c_date.day();
    // 计算用户年龄
    int age = c_year - u_year;
    // 判断用户是否还没有满周岁，如果没有满周岁，年龄减1
    if(c_month < u_month)
    {
        // 还没有到满周岁的月份
        age--;
    }
    else if(c_month == u_month and c_day < u_day)
    {
        // 本月生日，还没到
        age--;
    }
    // 计算用户的性别
    if(id_card.at(16).digitValue() == -1 or age < 0 or age > 150)
    {
        // 身份证号码倒数第二位不是数字或者年龄不正常
        QMessageBox::warning(this,"警告","请输入正确的身份证号码！");
        return;
    }
    QString gender = id_card.at(16).digitValue()%2 ? "男" : "女";
    // 检查用户是否选择了头像
    if(regist_icon.isEmpty())
    {
        if(QMessageBox::question(this,"警告","您还没有选择头像，是否使用默认头像？") == QMessageBox::Yes)
        {
            // 使用默认路径
            regist_icon = DEFAULT_ICON_PATH;
        }
        else
        {
            // 让用户选择头像
            on_btn_register_select_icon_clicked();
            if(regist_icon.isEmpty())
            {
                // 如果用户还是没有选择头像。直接使用默认头像
                regist_icon = DEFAULT_ICON_PATH;
            }
        }
    }
    // 将数据写入数据库
    if(db.userRegist(id,passwd,name,id_card,gender,age,tel,regist_icon))
    {
        // 注册成功
        QMessageBox::information(this,"成功","注册成功，请妥善保管您的账号密码！");
        // 返回登录界面
        // 将账号填充到登录界面中去
        ui->lineEdit_login_id->setText(id);
        on_btn_register_back_clicked();
        ui->lineEdit_login_passwd->setFocus();
        return;
    }
    else
    {
        // 注册失败
        QMessageBox::warning(this,"失败","注册失败，请检查信息是否有误！");
        return;
    }
}


void LoginDialog::on_btn_register_select_icon_clicked()
{
    // 选择头像文件
    QString path = QFileDialog::getOpenFileName(this,"选择头像","../../../","*.jpg *.jpeg *.png *.bmp");
    if(!path.isEmpty())
    {
        // 保存用户选择的头像路径，并显示到界面上去
        regist_icon = path;
        ui->label_register_icon->setPixmap(QPixmap(regist_icon));
    }
}


void LoginDialog::on_btn_login_login_clicked()
{
    // 停止自动登录的定时器
    if(timer_auto->isActive())
    {
        timer_auto->stop();
    }
    // 获取用户输入的账号密码
    QString id = ui->lineEdit_login_id->text();
    QString passwd = ui->lineEdit_login_passwd->text();
    if(id.isEmpty() or passwd.isEmpty())
    {
        QMessageBox::warning(this,"警告","账号密码不能为空！");
        return;
    }
    // 从数据库中查询账号密码
    QString name = db.userLoing(id,passwd);
    qDebug() << name;
    if(name.isEmpty())
    {
        // 登录失败
        QMessageBox::information(this,"失败","登录失败，账号或密码错误！");
        ui->lineEdit_login_passwd->clear();
        return;
    }
    else
    {
        // 登录成功
        // 检查是否需要记住密码和自动登录
        if(ui->checkBox_remember->isChecked())
        {
            // 如果需要记住密码，就将用户的账号密码保存到配置文件中去
            setting->setValue("User/id",id);
            setting->setValue("User/passwd",passwd);
        }
        // 更新配置文件中记住密码状态
        setting->setValue("State/remember",ui->checkBox_remember->isChecked());
        // 更新配置文件中自动登录状态
        setting->setValue("State/auto",ui->checkBox_auto->isChecked());
        QMessageBox::information(this,"成功","登录成功，祝您使用愉快！");
        ui->lineEdit_login_passwd->clear();
        // 发信号，让主界面显示出来
        emit loginSuccess(id);
        // 隐藏登录界面
        this->hide();
    }
}


void LoginDialog::on_btn_login_QR_clicked()
{

}


void LoginDialog::on_checkBox_auto_checkStateChanged(const Qt::CheckState &checked)
{
    if(checked)
    {
        // 选择了自动登录，那么一定要选择记住密码
        ui->checkBox_remember->setChecked(true);
    }
    else
    {
        // 取消自动登录，如果是正处于自动登录过程中，那么就要终止自动登录(停止定时器)
        if(timer_auto->isActive())
        {
            timer_auto->stop();
        }
    }
}


void LoginDialog::on_checkBox_remember_checkStateChanged(const Qt::CheckState &checked)
{
    if(!checked)
    {
        // 取消了记住密码，那么就不能进行自动登录
        ui->checkBox_auto->setChecked(false);
    }
}

