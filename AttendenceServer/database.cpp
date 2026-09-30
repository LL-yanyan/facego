#include "database.h"
#include "apppaths.h"
#include <QSqlError>
#include <QSqlQuery>
#include <QDebug>
DataBase::DataBase(QObject *parent)
    : QObject{parent}
{
    // 初始化数据库
    if(!dataBaseInit())
    {
        // 初始化数据库失败
        exit(-1);
    }
}

DataBase::~DataBase()
{
    // 关闭数据库连接
}

DataBase &DataBase::getInstance()
{
    // 返回一个静态数据库对象
    static DataBase db;
    return db;
}

bool DataBase::dataBaseInit()
{
    // 初始化数据库
    // 实例化一个数据库连接对象
    m_db = new QSqlDatabase;
    // 设置数据库类型
    *m_db = QSqlDatabase::addDatabase("QSQLITE");
    // 设置数据库文件的路径名
    m_db->setDatabaseName(apppaths::dataFile("database.db"));
    // 打开数据库
    if(!m_db->open())
    {
        qDebug() << "open database error:" + m_db->lastError().text();
        return false;
    }
    // 开启外键功能
    QString sql = "pragma foreign_keys = ON;";
    QSqlQuery query(*m_db);
    if(!query.exec(sql))
    {
        qDebug() << "open foreign_keys error:" + query.lastError().text();
        return false;
    }
    // 创建所有的关系表
    // 用户表
    // 账号 密码 姓名 身份证号码 性别 年龄 手机号码 头像路径
    // id passwd name id_card gender age tel icon_path
    sql = "create table if not exists t_user("
                  "id varchar(12) primary key,"
                  "passwd varchar(12) not null,"
                  "name varchar(30) not null,"
                  "id_card char(18) not null unique,"
                  "gender char(3) not null,"
                  "age smallint(2) not null,"
                  "tel char(11) not null unique,"
                  "icon_path varchar not null);";
    if(!query.exec(sql))
    {
        qDebug() << "create t_user error:" + query.lastError().text();
        return false;
    }
    // 部门表
    // 序号 部门名称
    // num department
    sql = "create table if not exists t_department("
                  "num integer primary key autoincrement,"
                  "department varchar unique not null);";
    if(!query.exec(sql))
    {
        qDebug() << "create t_department error:" + query.lastError().text();
        return false;
    }
    // 岗位表
    // 序号 部门名称 岗位名称
    // num department position
    sql = "create table if not exists t_position("
          "num integer primary key autoincrement,"
          "department_id integer not null,"
          "department varchar not null,"
          "position varchar not null,"
          "foreign key (department_id) references t_department(num)"
          "on delete cascade "
          "on update cascade);";
    if(!query.exec(sql))
    {
        qDebug() << "create t_position error:" + query.lastError().text();
        return false;
    }
    // 员工表
    // 员工ID 部门 职位 姓名 身份证号码 性别 年龄 生日 民族 政治面貌 学历 毕业院校 手机号码 入职日期 人脸ID
    // id department position name id_card gender age birthday nation political education college tel join_date face_id
    sql = "create table if not exists t_employees("
          "id char(10) primary key,"
          "department varchar not null,"
          "position varchar not null,"
          "name varchar(30) not null,"
          "id_card char(18) unique not null,"
          "gender char(3) not null,"
          "age smallint(2) not null,"
          "birthday date not null,"
          "nation varchar not null,"
          "political varchar not null,"
          "education varchar not null,"
          "college varchar not null,"
          "tel char(11) unique not null,"
          "join_date date not null,"
          "face_id integer not null);";
    if(!query.exec(sql))
    {
        qDebug() << "create t_employees error:" + query.lastError().text();
        return false;
    }
    // 考勤表
    // 序号 员工ID 考勤时间 类型 状态
    // num employee_id time type state
    sql = "create table if not exists t_attendance("
          "num integer primary key autoincrement,"
          "employee_id char(11) not null,"
          "time datetime not null,"
          "type varchar not null,"
          "state varchar not null, "
          "foreign key (employee_id) references t_employees(id)"
          "on delete cascade "
          "on update cascade);";
    if(!query.exec(sql))
    {
        qDebug() << "create t_attendance error:" + query.lastError().text();
        return false;
    }
    qDebug() << "database init success!";
    return true;
}

bool DataBase::userRegist(QString id, QString passwd, QString name, QString id_card, QString gender, int age, QString tel, QString icon_path)
{
    QString sql = QString("insert into t_user(id,passwd,name,id_card,gender,age,tel,icon_path) values("
                          "'%1','%2','%3','%4','%5',%6,'%7','%8');").arg(id).arg(passwd).arg(name).arg(id_card)
                      .arg(gender).arg(age).arg(tel).arg(icon_path);
    QSqlQuery query(*m_db);
    if(!query.exec(sql))
    {
        qDebug() << "insert into t_user error:" + query.lastError().text();
        return false;
    }
    qDebug() << "insert into t_user success!";
    return true;
}

QString DataBase::getUserIconPath(QString user_id)
{
    QString sql = QString("select icon_path from t_user where id = '%1';").arg(user_id);
    QSqlQuery query(*m_db);
    if(!query.exec(sql))
    {
        qDebug() << "select icon_path error:" + query.lastError().text();
        return "";
    }
    if(query.next())
    {
        return query.value(0).toString();
    }
    else
    {
        return "";
    }
}

QString DataBase::userLoing(QString id, QString passwd)
{
    QString sql = QString("select name from t_user where id = '%1' and passwd = '%2';").arg(id).arg(passwd);
    QSqlQuery query(*m_db);
    if(!query.exec(sql))
    {
        qDebug() << "select user name error:" + query.lastError().text();
        return "";
    }
    if(query.next())
    {
        return query.value(0).toString();
    }
    else
    {
        return "";
    }
}

QString DataBase::getUserName(QString id)
{
    QString sql = QString("select name from t_user where id = '%1';").arg(id);
    QSqlQuery query(*m_db);
    if(!query.exec(sql))
    {
        qDebug() << "select user name error:" + query.lastError().text();
        return "";
    }
    if(query.next())
    {
        return query.value(0).toString();
    }
    else
    {
        return "";
    }
}

QStringList DataBase::getAllDepartment()
{
    QString sql = "select department from t_department;";
    QSqlQuery query(*m_db);
    QStringList departmens;
    if(!query.exec(sql))
    {
        qDebug() << "select department error:" + query.lastError().text();
        return departmens;
    }
    while(query.next())
    {
        departmens.append(query.value(0).toString());
    }
    return departmens;
}

bool DataBase::addDepartment(QString department)
{
    QString sql = QString("insert into t_department(department) values('%1');").arg(department);
    QSqlQuery query(*m_db);
    if(!query.exec(sql))
    {
        qDebug() << "insert into t_department(department) error:" + query.lastError().text();
        return false;
    }
    return true;
}

bool DataBase::deleteDepartment(QString department)
{
    QString sql = QString("delete from t_department where department='%1';").arg(department);
    QSqlQuery query(*m_db);
    if(!query.exec(sql))
    {
        qDebug() << "delete from t_department error:" + query.lastError().text();
        return false;
    }
    return true;
}

QStringList DataBase::getPosition(QString department)
{
    QString sql = QString("select position from t_position where department = '%1';").arg(department);
    QSqlQuery query(*m_db);
    QStringList positions;
    if(!query.exec(sql))
    {
        qDebug() << "select position error:" + query.lastError().text();
        return positions;
    }
    while(query.next())
    {
        positions.append(query.value(0).toString());
    }
    return positions;
}

bool DataBase::addPosition(QString department, QString position)
{
    QString sql = QString("select num from t_department where department = '%1';").arg(department);
    QSqlQuery query(*m_db);
    if(!query.exec(sql))
    {
        qDebug() << "select num from t_department error:" + query.lastError().text();
        return false;
    }
    if(!query.next())
    {
        return false;
    }
    int department_id = query.value(0).toInt();
    sql = QString("insert into t_position(department_id,department,position) values(%1,'%2','%3');")
              .arg(department_id).arg(department).arg(position);
    if(!query.exec(sql))
    {
        qDebug() << "insert into t_position error:" + query.lastError().text();
        return false;
    }
    return true;
}

bool DataBase::deletePosition(QString department,QString position)
{
    QString sql = QString("delete from t_position where department = '%1' and position='%2';").arg(department).arg(position);
    QSqlQuery query(*m_db);
    if(!query.exec(sql))
    {
        qDebug() << "delete from t_position error:" + query.lastError().text();
        return false;
    }
    return true;
}

bool DataBase::employeeRegister(QString id, QString department, QString position, QString name, QString id_card,
                                QString gender, int age, QString birthday, QString nation, QString political,
                                QString education, QString college, QString tel, QString join_date, int face_id)
{
    QString sql = QString("insert into t_employees values("
                          "'%1','%2','%3','%4','%5','%6',%7,'%8','%9','%10','%11','%12','%13','%14',%15);")
                      .arg(id).arg(department).arg(position).arg(name).arg(id_card).arg(gender).arg(age).arg(birthday)
                      .arg(nation).arg(political).arg(education).arg(college).arg(tel).arg(join_date).arg(face_id);
    QSqlQuery query(*m_db);
    if(!query.exec(sql))
    {
        qDebug() << "insert into t_employees error:" + query.lastError().text();
        return false;
    }
    qDebug() << "insert into t_employees success!";
    return true;
}

QList<EMPLOYEE_INFO> DataBase::getAllEmployeeInfo()
{
    QString sql = "select * from t_employees;";
    QSqlQuery query(*m_db);
    QList<EMPLOYEE_INFO> info_list;
    if(!query.exec(sql))
    {
        qDebug() << "select all t_employees error:" + query.lastError().text();
        return info_list;
    }
    while(query.next())
    {
        info_list.append(EMPLOYEE_INFO{query.value(0).toString(),query.value(1).toString(),query.value(2).toString(),
            query.value(3).toString(),query.value(4).toString(),query.value(5).toString(),query.value(6).toInt(),
            query.value(7).toString(),query.value(8).toString(),query.value(9).toString(),query.value(10).toString(),
            query.value(11).toString(),query.value(12).toString(),query.value(13).toString(),query.value(14).toInt()});
    }
    return info_list;
}

EMPLOYEE_INFO DataBase::getEmployeeInfo(QString id)
{
    QString sql = QString("select * from t_employees where id = '%1';").arg(id);
    QSqlQuery query(*m_db);
    EMPLOYEE_INFO info;
    if(!query.exec(sql))
    {
        qDebug() << "select employee info error:" + query.lastError().text();
        info.id = "-1";
        return info;
    }
    if(query.next())
    {
        info = EMPLOYEE_INFO{query.value(0).toString(),query.value(1).toString(),query.value(2).toString(),
                query.value(3).toString(),query.value(4).toString(),query.value(5).toString(),query.value(6).toInt(),
                query.value(7).toString(),query.value(8).toString(),query.value(9).toString(),query.value(10).toString(),
                query.value(11).toString(),query.value(12).toString(),query.value(13).toString(),query.value(14).toInt()};
        return info;
    }
    else
    {
        info.id = "-1";
        return info;
    }
}

EMPLOYEE_INFO DataBase::getEmployeeInfoByFaceID(int64_t face_id)
{
    QString sql = QString("select * from t_employees where face_id = '%1';").arg(face_id);
    QSqlQuery query(*m_db);
    EMPLOYEE_INFO info;
    if(!query.exec(sql))
    {
        qDebug() << "select employee info by face_id error:" + query.lastError().text();
        info.id = "-1";
        return info;
    }
    if(query.next())
    {
        info = EMPLOYEE_INFO{query.value(0).toString(),query.value(1).toString(),query.value(2).toString(),
                             query.value(3).toString(),query.value(4).toString(),query.value(5).toString(),query.value(6).toInt(),
                             query.value(7).toString(),query.value(8).toString(),query.value(9).toString(),query.value(10).toString(),
                             query.value(11).toString(),query.value(12).toString(),query.value(13).toString(),query.value(14).toInt()};
        return info;
    }
    else
    {
        info.id = "-1";
        return info;
    }
}

QList<ATTENDANCE_INFO> DataBase::getAllAttendanceRecord()
{
    QString sql = QString("select t_attendance.num,t_attendance.employee_id,t_employees.name,t_employees.department,"
                            "t_employees.position,t_attendance.time,t_attendance.type,t_attendance.state "
                            "from t_attendance "
                            "join t_employees on t_attendance.employee_id = t_employees.id;");
    QSqlQuery query(*m_db);
    QList<ATTENDANCE_INFO> info_list;
    if(!query.exec(sql))
    {
      qDebug() << "select all t_attendance error:" + query.lastError().text();
      return info_list;
    }
    while(query.next())
    {
      info_list.append(ATTENDANCE_INFO{query.value(0).toString(),query.value(1).toString(),query.value(2).toString(),query.value(3).toString(),
                            query.value(4).toString(),query.value(5).toString(),query.value(6).toString(),query.value(7).toString()});
    }
    return info_list;
}
