#include "serverwindow.h"
#include "apppaths.h"
#include "ui_serverwindow.h"
#include <QThread>
#include <QDateTime>
#include <QMessageBox>

ServerWindow::ServerWindow(QWidget *parent)
    : QMainWindow(parent),face_object(new FaceObject()),is_monitor(false),
    is_show_register_pic(false),hase_take_photo(false),curr_mode(MODE_RECOGNITION)
    , ui(new Ui::ServerWindow)
{
    ui->setupUi(this);
    // 实例化一个登录界面对象
    login = new LoginDialog();
    // 连接信号与槽，当登录成功以后显示主界面
    connect(login,&LoginDialog::loginSuccess,this,&ServerWindow::slotLoginSuccess);
    // 显示登录界面
    login->show();
    // 初始化服务器
    if(!init())
    {
        exit(-1);
    }
}

ServerWindow::~ServerWindow()
{
    delete ui;
}

bool ServerWindow::init()
{
    // 设置窗口名称
    this->setWindowTitle("粤嵌考勤管理系统 V1.0");
    // 设置窗口图标
    this->setWindowIcon(QIcon("://imgs/window_icon.jpeg"));
    // 实例化一个服务器套接字对象
    server = new QTcpServer(this);
    // 连接信号与槽：如果有客户端连接服务器，就去处理器
    connect(server,SIGNAL(newConnection()),this,SLOT(slotNewConnection()));
    // 服务器开启监听
    if(!server->listen(QHostAddress::Any,SERVER_PORT))
    {
        qDebug() << "服务端监听失败：" + server->errorString();
        return false;
    }
    // 创建一个线程，用来处理人脸对象相关功能
    QThread *thread = new QThread(this);
    // 将FaceObject对象转移到新线程中去执行
    face_object->moveToThread(thread);
    // 如果需要进行人脸检测，就发出信号queryFace，让人脸对象去执行人脸检测功能
    // 默认的连接方式是自动连接：当发送者和接收者都是同线程中的对象时，采用直连方式，否则使用队列连接方式
    // 队列连接方式需要等到接收者所属线程的事件循环取得控制权时才能获得该信号，调用相应的槽函数
    // 所以，将它的连接方式修改直连方式
    // 连接信号与槽：当需要进行人脸查询的时候，就让人脸对象去处理
    connect(this,SIGNAL(queryFace(cv::Mat&,QTcpSocket*)),face_object,SLOT(faceQuery(cv::Mat&,QTcpSocket*)),Qt::DirectConnection);
    // 民族数据有点多，使用代码的方式将他们显示到界面的控件中去
    QString nation = "汉族、蒙古族、回族、藏族、维吾尔族、苗族、彝族、壮族、布依族、朝鲜族、满族、侗族、瑶族、白族、"
                     "土家族、哈尼族、哈萨克族、傣族、黎族、傈僳族、佤族、畲族、高山族、拉祜族、水族、东乡族、纳西族、"
                     "景颇族、柯尔克孜族、土族、达斡尔族、仫佬族、羌族、布朗族、撒拉族、毛南族、仡佬族、锡伯族、阿昌族、"
                     "普米族、塔吉克族、怒族、乌孜别克族、俄罗斯族、鄂温克族、德昂族、保安族、裕固族、京族、塔塔尔族、独龙族、"
                     "鄂伦春族、赫哲族、门巴族、珞巴族、基诺族";
    ui->comboBox_regist_nation->addItems(nation.split("、"));
    // 将入职日期默认设置为当前日期
    ui->dateEdit_regist_date->setDate(QDate::currentDate());
    // 显示用户信息
    label_user_name = new QLabel(this);
    ui->statusbar->addPermanentWidget(label_user_name);
    // 显示系统时间
    label_systime = new QLabel(this);
    ui->statusbar->addPermanentWidget(label_systime);
    // 实例化定时器对象
    timer_systime = new QTimer(this);
    timer_systime->setInterval(1000);
    connect(timer_systime,SIGNAL(timeout()),this,SLOT(slotGetSystime()));
    timer_systime->start();
    slotGetSystime();
    // 获取已有的部门
    ui->comboBox_DM_department->addItems(db.getAllDepartment());
    ui->comboBox_regist_department->addItems(db.getAllDepartment());
    // 实例化表格模型
    t_employee_model = new QStandardItemModel;
    t_attendance_model = new QStandardItemModel;
    // 设置表头
    t_employee_model->setHorizontalHeaderLabels({"工号","所属部门","职位","姓名","身份证号码","性别","年龄","出生年月日","民族","政治面貌","学历","毕业院校","联系方式","入职日期","人脸ID"});;
    t_attendance_model->setHorizontalHeaderLabels({"编号","工号","姓名","所属部门","职位","考勤时间","类型","状态"});
    // 启用表格排序功能
    ui->tableView_employees_info->setSortingEnabled(true);
    ui->tableView_attendance_info->setSortingEnabled(true);
    // 禁用表格编辑功能
    ui->tableView_employees_info->setEditTriggers(QAbstractItemView::QAbstractItemView::NoEditTriggers);
    ui->tableView_attendance_info->setEditTriggers(QAbstractItemView::QAbstractItemView::NoEditTriggers);
    // 将表格模型和表格控件绑定
    ui->tableView_employees_info->setModel(t_employee_model);
    ui->tableView_attendance_info->setModel(t_attendance_model);
    // 设置注册页面视频框自适应大小
    ui->label_regist_photo->setScaledContents(true);
    // 禁用关闭画面按钮
    ui->btn_home_close->setDisabled(true);
    // 连接信号与槽：当人脸识别完成以后，人脸对象会发出sendFaceID的信号，将识别结果整合完成以后发给客户端
    connect(face_object,SIGNAL(sendFaceID(int64_t,QTcpSocket*)),this,SLOT(slotSendAttendanceResult(int64_t,QTcpSocket*)));
    return true;
}

void ServerWindow::closeServer()
{
    server->close();
    // 断开所有的客户端
    for(auto socket : list_client)
    {
        socket->disconnectFromHost();   // 断开客户端
        socket->deleteLater();  // 延时断开
    }
    // 清空客户端套接字列表
    list_client.clear();
}

void ServerWindow::slotLoginSuccess(QString id)
{
    // 将当前管理员的姓名显示到界面上去
    QString user_name = db.getUserName(id);
    user_name = user_name.isEmpty() ? "未知" : user_name;
    label_user_name->setText("当前管理员："+user_name);
    // 默认显示主页
    ui->stackedWidget->setCurrentWidget(ui->page_home);
    // 显示主界面
    this->show();
}

void ServerWindow::slotNewConnection()
{
    // 接收客户端的连接请求，并与之通信
    QTcpSocket *client = server->nextPendingConnection();
    // 将客户端的套接字对象指针添加到客户端连接列表中去
    list_client.append(client);
    // 默认为人脸识别模式，发送人脸识别指令给客户端
    client->write("RECOGNITION");
    client_mode = MODE_RECOGNITION;
    // 通过套接字对象来获取客户端的IP地址
    QString client_ip = client->peerAddress().toString();
    qDebug() << QString("客户端[%1]连接成功！").arg(client_ip);
    // 连接信号与槽：当该客户端发送数据过来的时候去处理数据
    connect(client,SIGNAL(readyRead()),this,SLOT(slotReadyRead()));
    // 连接信号与槽：当该客户端断开连接的时候进行处理
    connect(client,SIGNAL(disconnected()),this,SLOT(slotDisconnected()));
}

void ServerWindow::slotReadyRead()
{
    // 读取客户端发送过来的数据
    // 获取是哪个客户端发送过来的数据
    QTcpSocket *socket = (QTcpSocket*)sender();
    if(!socket)
    {
        return;
    }
    // 构造一个数据量对象(绑定套接字)，用来保存读到的数据，方便还原数据
    QDataStream data_stream(socket);
    // 考虑到兼容性问题，用和客户端版本相同的方式来解析数据
    data_stream.setVersion(QDataStream::Qt_6_0);
    // 判断客户端的数据是否发送完成，如果发送完了就处理数据，如果没有发送完，就等待它发送完成
    static quint64 data_size = 0;  // 用来保存有效数据的长度，如果这个值为0，就表示还没有读到数据
    if(data_size == 0)
    {
        // 这一帧画面的数据还没有开始接收，判断待接收的数据长度是否大于最小数据长度(8字节)
        if(socket->bytesAvailable() < (qint64)sizeof(data_size))
        {
            qDebug() << "数据总长度没有超过8字节。数据存在问题！";
            return;
        }
        // 数据长度大于等于8字节，获取有效字节数据长度值
        data_stream >> data_size;
        qDebug() << "data_size=" << data_size;
    }
    // 继续接收后续的画面数据
    if(socket->bytesAvailable() < data_size)
    {
        // 客户端的数据还没发完，等客户端发送完成以后，再一起获取
        qDebug() << "客户端的数据还没发送完成！";
        return;
    }
    // 数据完整了，读取数据
    QByteArray frame_data;  // 用来保存客户端发送过来的画面数据
    data_stream >> frame_data;
    // 将数据长度值设置为0,等待获取下一帧画面
    data_size = 0;
    // 判断获取到的数据是不是为空
    if(frame_data.size() == 0)
    {
        qDebug() << "图片数据为空！";
        return;
    }
    // 根据当前页面以及状态来确定对画面数据如何处理
    // 如果当前所在界面是首页，并且需要显示实时画面，就将画面显示到实时画面框中去
    if(ui->stackedWidget->currentWidget() == ui->page_home and is_monitor)
    {
        qDebug() << "MODE_MONITOR";
        curr_mode = MODE_MONITOR;
        QPixmap pixmap;
        if(!pixmap.loadFromData(frame_data,"jpg"))
        {
            qDebug() << "加载实时画面失败";
            return;
        }
        // 将画面显示到主页中的实时画面窗口中去
        ui->label_home_monitor->setPixmap(pixmap);
        // return;
    }
    else if(ui->stackedWidget->currentWidget() == ui->page_employee_register and is_show_register_pic)
    {
        qDebug() << "MODE_REGISTER";
        curr_mode = MODE_REGISTER;
        QPixmap pixmap;
        if(!pixmap.loadFromData(frame_data,"jpg"))
        {
            qDebug() << "加载实时注册头像失败";
            return;
        }
        // 保存当前画面数据，方便拍照时显示并保存图片
        photo_data = frame_data;
        // 将画面显示到注册页面中的实时窗口中去
        ui->label_regist_photo->setPixmap(pixmap);
        // return;
    }
    else
    {
        qDebug() << "MODE_RECOGNITION";
        curr_mode = MODE_RECOGNITION;
        // 识别画面中的人脸信息
        // 将QByteArray数据转换为opencv的Mat数据对象
        std::vector<uchar> decode;  // 用来保存QByteArray中的数据
        decode.resize(frame_data.size());
        // 将QByteArray对象中的数据拷贝到容器对象中去
        memcpy(decode.data(),frame_data.data(),frame_data.size());
        // 对数据进行解码
        cv::Mat face_img = cv::imdecode(decode,cv::IMREAD_COLOR);
        // 发送信号，让人脸对象去执行人脸查询操作
        emit queryFace(face_img,socket);
    }
    // 如果当前模式和客户端模式不匹配，发送指令让客户端切换模式
    if(curr_mode != client_mode)
    {
        switch (curr_mode)
        {
        case MODE_MONITOR:
            socket->write("MONITOR");
            break;
        case MODE_REGISTER:
            socket->write("REGISTER");
            break;
        case MODE_RECOGNITION:
            socket->write("RECOGNITION");
            break;
        default:
            break;
        }
        client_mode = curr_mode;
    }
}

void ServerWindow::slotDisconnected()
{
    // 先获取是哪个客户端发送过来的信号
    QTcpSocket *socket = (QTcpSocket*)sender();
    if(!socket)
    {
        return;
    }
    // 获取客户端的IP地址
    QString client_ip = socket->peerAddress().toString();
    qDebug() << QString("客户端[%1]断开连接！").arg(client_ip);
    // 从客户端连接列表中将该客户端删除
    list_client.removeOne(socket);
    // 将客户端断开连接(延迟断开)
    socket->deleteLater();
}

void ServerWindow::slotGetSystime()
{
    label_systime->setText(QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss dddd"));
}

void ServerWindow::slotSendAttendanceResult(int64_t face_id,QTcpSocket *socket)
{
    // 将识别结果整理好以后发送给客户端
    // 如果没有匹配到人脸信息，返回识别失败的结果给客户端
    QString send_data;
    if(-1 == face_id)
    {
        // 准备空结果按照Json格式给客户端发送数据
        send_data = QString(R"({"id":"-1","name":"","department":"","position":""})");
        // 发送给客户端
        socket->write(send_data.toUtf8());
        return;
    }
    // 成功识别到人脸信息了，就去数据库中查询该员工的信息，并发送给客户端
    EMPLOYEE_INFO e_info = db.getEmployeeInfoByFaceID(face_id);
    // 判断是否识别到身份信息了
    if(e_info.id != "-1")
    {
        // 查询到了记录，将员工信息按照Json格式打包发送给客户端
        send_data = QString(R"({"id":"%1","name":"%2","department":"%3","position":"%4"})")
                        .arg(e_info.id).arg(e_info.name).arg(e_info.department).arg(e_info.position);
        // 发送给客户端
        socket->write(send_data.toUtf8());
        // 将考勤记录添加到考勤表中去
        // ...
    }
    else
    {
        // 没有查询到记录
        send_data = QString(R"({"id":"-1","name":"","department":"","position":""})");
        // 发送给客户端
        socket->write(send_data.toUtf8());
    }
}

void ServerWindow::on_btn_home_clicked()
{
    // 跳转到主页
    ui->stackedWidget->setCurrentWidget(ui->page_home);
}


void ServerWindow::on_btn_employee_register_clicked()
{
    // 跳转到员工注册界面
    ui->stackedWidget->setCurrentWidget(ui->page_employee_register);
}


void ServerWindow::on_btn_employees_management_clicked()
{
    // 跳转到员工管理界面
    ui->stackedWidget->setCurrentWidget(ui->page_employees_management);
}


void ServerWindow::on_btn_department_management_clicked()
{
    // 跳转到部门管理界面
    ui->stackedWidget->setCurrentWidget(ui->page_department_management);
}


void ServerWindow::on_btn_attendance_record_clicked()
{
    // 跳转到考勤记录界面
    ui->stackedWidget->setCurrentWidget(ui->page_attendance_record);
}


void ServerWindow::on_btn_home_show_clicked()
{
    is_monitor = true;
    // 使能关闭画面按钮
    ui->btn_home_close->setEnabled(true);
    // 禁用显示画面按钮
    ui->btn_home_show->setDisabled(true);
}


void ServerWindow::on_btn_home_close_clicked()
{
    is_monitor = false;
    // 禁用关闭画面按钮
    ui->btn_home_close->setDisabled(true);
    // 使能显示画面按钮
    ui->btn_home_show->setEnabled(true);
}


void ServerWindow::on_btn_regist_open_camera_clicked()
{
    // 让画面显示到注册页面
    is_show_register_pic = true;
    // 禁用打开摄像头按钮
    ui->btn_regist_open_camera->setDisabled(true);
    // 使能拍照按钮
    ui->btn_regist_take_phtoto->setEnabled(true);
    // 使能关闭摄像头按钮
    ui->btn_regist_close_camera->setEnabled(true);
}


void ServerWindow::on_btn_regist_close_camera_clicked()
{
    is_show_register_pic = false;
    // 使能打开摄像头按钮
    ui->btn_regist_open_camera->setEnabled(true);
    // 禁用使能拍照按钮
    ui->btn_regist_take_phtoto->setDisabled(true);
    // 禁用关闭摄像头按钮
    ui->btn_regist_close_camera->setDisabled(true);
}


void ServerWindow::on_btn_regist_take_phtoto_clicked()
{
    if(photo_data.isEmpty())
    {
        qDebug() << "拍照失败：图片数据为空";
        return;
    }
    on_btn_regist_close_camera_clicked();
    hase_take_photo = true;
    // 将照片显示到控件中去
    QPixmap pixmap;
    if(!pixmap.loadFromData(photo_data,"jpg"))
    {
        qDebug() << "加载照片失败";
        return;
    }
    ui->label_regist_photo->setPixmap(pixmap);
}


void ServerWindow::on_btn_regist_reset_clicked()
{
    on_btn_regist_close_camera_clicked();
    ui->lineEdit_regist_id->clear();
    if(ui->comboBox_regist_department->count())
    {
        ui->comboBox_regist_department->setCurrentIndex(0);
    }
    if(ui->comboBox_regist_position->count())
    {
        ui->comboBox_regist_position->setCurrentIndex(0);
    }
    ui->lineEdit_regist_name->clear();
    ui->lineEdit_regist_id_card->clear();
    ui->lineEdit_regist_tel->clear();
    ui->lineEdit_regist_college->clear();
    ui->comboBox_regist_gender->setCurrentIndex(0);
    ui->spinBox_regist_age->setValue(22);
    ui->dateEdit_regist_birthday->setDate(QDate(2000,1,1));
    ui->dateEdit_regist_date->setDate(QDate::currentDate());
    ui->comboBox_regist_nation->setCurrentIndex(0);
    ui->comboBox_regist_political->setCurrentIndex(0);
    ui->comboBox_regist_education->setCurrentIndex(0);
    ui->label_regist_photo->clear();
    hase_take_photo = false;
}


void ServerWindow::on_btn_regist_cancel_clicked()
{
    on_btn_regist_reset_clicked();
}


void ServerWindow::on_btn_regist_ok_clicked()
{
    // 获取用户输入的信息
    QString id = ui->lineEdit_regist_id->text();
    QString department = ui->comboBox_regist_department->currentText();
    QString position = ui->comboBox_regist_position->currentText();
    QString name = ui->lineEdit_regist_name->text();
    QString id_card = ui->lineEdit_regist_id_card->text();
    QString tel = ui->lineEdit_regist_tel->text();
    QString college = ui->lineEdit_regist_college->text();
    QString register_date = ui->dateEdit_regist_date->date().toString("yyyy/MM/dd");
    QString nation = ui->comboBox_regist_nation->currentText();
    QString political = ui->comboBox_regist_political->currentText();
    QString education = ui->comboBox_regist_education->currentText();
    // 判断信息是否为空
    if(id.isEmpty() or department.isEmpty() or position.isEmpty() or name.isEmpty() or id_card.isEmpty()
        or tel.isEmpty() or college.isEmpty())
    {
        QMessageBox::information(this,"警告","请输入完整的信息！");
        return;
    }
    if(id.length() != 10)
    {
        QMessageBox::information(this,"警告","请输入10位的工号！");
        return;
    }
    if(id_card.length() != 18)
    {
        QMessageBox::information(this,"警告","请输入正确的身份证号码！");
        return;
    }
    if(tel.length() != 11)
    {
        QMessageBox::information(this,"警告","请输入正确的手机号码！");
        return;
    }
    // 根据身份证号码计算用户的年龄和性别
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
    QString birthday = QString("%1-%2-%3").arg(u_year).arg(u_month).arg(u_day);
    // 将用户的年龄、性别、生日设置到界面上去
    ui->spinBox_regist_age->setValue(age);
    ui->comboBox_regist_gender->setCurrentText(gender);
    ui->dateEdit_regist_birthday->setDate(QDate(u_year,u_month,u_day));
    // 判断用户是否拍了照
    if(!hase_take_photo)
    {
        // 用户还没有拍照，提示用户拍照
        QMessageBox::information(this,"警告","请拍照，完成人脸录入！");
        return;
    }
    // 注册人脸
    // 将QByteArray数据转换为opencv的Mat数据发送给人脸对象去进行注册
    std::vector<uchar> decode;  // 用来保存QByteArray中的数据
    decode.resize(photo_data.size());
    // 将QByteArray对象中的数据拷贝到容器对象中去
    memcpy(decode.data(),photo_data.data(),photo_data.size());
    // 对数据进行解码
    cv::Mat face_img = cv::imdecode(decode,cv::IMREAD_COLOR);
    // 通过人脸对象来注册人脸
    int64_t face_id = face_object->faceRegister(face_img);
    if(-1 == face_id)
    {
        // 注册人脸失败
        QMessageBox::information(this,"警告","人脸注册失败，请重新拍照！");
        hase_take_photo = false;
        return;
    }
    // 保存员工注册时的照片
    cv::imwrite(apppaths::userImageFile(id).toUtf8().constData(),face_img);
    // 添加员工
    ok = db.employeeRegister(id,department,position,name,id_card,gender,age,birthday,nation,
                        political,education,college,tel,register_date,face_id);
    if(!ok)
    {
        QMessageBox::information(this,"失败","员工注册失败，请检查后再试！");
        return;
    }
    QMessageBox::information(this,"成功","员工注册成功！");
    on_btn_regist_close_camera_clicked();
    on_btn_regist_reset_clicked();
}


void ServerWindow::on_btn_DM_add_department_clicked()
{
    // 添加部门
    QString department = ui->lineEdit_DM_input->text();
    if(department.isEmpty())
    {
        QMessageBox::information(this,"警告","请输入要添加的部门名字!");
        return;
    }
    // 检查该部门是否已经存在
    if(db.getAllDepartment().contains(department))
    {
        QMessageBox::information(this,"警告","该部门已经存在,请不要重复添加!");
        return;
    }
    if(QMessageBox::question(this,"添加部门",QString("是否要添加部门:%1").arg(department)) == QMessageBox::Yes)
    {
        // 添加部门
        if(db.addDepartment(department))
        {
            QMessageBox::information(this,"成功","添加部门成功!");
            ui->lineEdit_DM_input->clear();
            return;
        }
        else
        {
            QMessageBox::information(this,"失败","添加部门失败!");
            return;
        }
    }
}


void ServerWindow::on_btn_DM_delete_department_clicked()
{
    // 删除部门
    QString department = ui->lineEdit_DM_input->text();
    if(department.isEmpty())
    {
        QMessageBox::information(this,"警告","请输入要删除的部门名字!");
        return;
    }
    if(QMessageBox::question(this,"删除部门",QString("是否要删除部门:%1").arg(department)) == QMessageBox::Yes)
    {
        // 删除部门
        if(db.deleteDepartment(department))
        {
            QMessageBox::information(this,"成功","删除部门成功!");
            ui->lineEdit_DM_input->clear();
            return;
        }
        else
        {
            QMessageBox::information(this,"失败","删除部门失败!");
            return;
        }
    }
}


void ServerWindow::on_btn_DM_add_position_clicked()
{
    // 添加岗位
    QString position = ui->lineEdit_DM_input->text();
    if(position.isEmpty())
    {
        QMessageBox::information(this,"警告","请输入要添加的岗位名字!");
        return;
    }
    QString department = ui->comboBox_DM_department->currentText();
    if(department.isEmpty())
    {
        QMessageBox::information(this,"警告","请先选择要在哪个部门中添加岗位!");
        return;
    }
    // 检查该岗位是否已经存在
    if(db.getPosition(department).contains(department))
    {
        QMessageBox::information(this,"警告","该岗位已经存在,请不要重复添加!");
        return;
    }
    if(QMessageBox::question(this,"添加岗位",QString("是否要在%1中添加岗位:%2").arg(department).arg(position)) == QMessageBox::Yes)
    {
        // 添加岗位
        if(db.addPosition(department,position))
        {
            QMessageBox::information(this,"成功","添加岗位成功!");
            ui->lineEdit_DM_input->clear();
            return;
        }
        else
        {
            QMessageBox::information(this,"失败","添加岗位失败!");
            return;
        }
    }
}


void ServerWindow::on_btn_DM_delete_position_clicked()
{
    // 删除岗位
    QString position = ui->lineEdit_DM_input->text();
    if(position.isEmpty())
    {
        QMessageBox::information(this,"警告","请输入要删除的岗位名字!");
        return;
    }
    QString department = ui->comboBox_DM_department->currentText();
    if(department.isEmpty())
    {
        QMessageBox::information(this,"警告","请先选择要从哪个部门中删除岗位!");
        return;
    }
    if(QMessageBox::question(this,"删除岗位",QString("是否要删除%1中的岗位:%2").arg(department).arg(position)) == QMessageBox::Yes)
    {
        // 删除岗位
        if(db.deletePosition(department,position))
        {
            QMessageBox::information(this,"成功","删除岗位成功!");
            ui->lineEdit_DM_input->clear();
            return;
        }
        else
        {
            QMessageBox::information(this,"失败","删除岗位失败!");
            return;
        }
    }
}


void ServerWindow::on_comboBox_DM_department_currentTextChanged(const QString &text)
{
    ui->comboBox_DM_position->clear();
    ui->comboBox_DM_position->addItems(db.getPosition(text));
}


void ServerWindow::on_comboBox_regist_department_currentTextChanged(const QString &text)
{
    ui->comboBox_regist_position->clear();
    ui->comboBox_regist_position->addItems(db.getPosition(text));
}


void ServerWindow::on_btn_EM_refresh_clicked()
{
    // 先清空表格中的所有信息
    t_employee_model->removeRows(0,t_employee_model->rowCount());
    // 获取所有员工信息
    QList<EMPLOYEE_INFO> info_list = db.getAllEmployeeInfo();
    // 将数据显示到界面上去
    for(int i = 0; i < info_list.length(); i++)
    {
        QList<QStandardItem*> row_data;
        row_data << new QStandardItem(info_list.at(i).id)
                 << new QStandardItem(info_list.at(i).department)
                 << new QStandardItem(info_list.at(i).position)
                 << new QStandardItem(info_list.at(i).name)
                 << new QStandardItem(info_list.at(i).id_card)
                 << new QStandardItem(info_list.at(i).gender)
                 << new QStandardItem(QString::number(info_list.at(i).age))
                 << new QStandardItem(info_list.at(i).birthday)
                 << new QStandardItem(info_list.at(i).nation)
                 << new QStandardItem(info_list.at(i).political)
                 << new QStandardItem(info_list.at(i).education)
                 << new QStandardItem(info_list.at(i).college)
                 << new QStandardItem(info_list.at(i).tel)
                 << new QStandardItem(info_list.at(i).join_date)
                 << new QStandardItem(QString::number(info_list.at(i).face_id));
        t_employee_model->appendRow(row_data);
    }
}

