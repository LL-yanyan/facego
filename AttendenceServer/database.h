#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <QSqlDatabase>

// 员工信息结构体
// 员工ID 部门 职位 姓名 身份证号码 性别 年龄 生日 民族 政治面貌 学历 毕业院校 手机号码 入职日期 人脸ID
struct EMPLOYEE_INFO
{
    EMPLOYEE_INFO() {}
    EMPLOYEE_INFO(QString id,QString department,QString position,QString name,QString id_card,
                  QString gender,int age,QString birthday,QString nation,QString political,
                  QString education,QString college,QString tel,QString join_date,int face_id)
        :id(id),department(department),position(position),name(name),id_card(id_card),gender(gender),
        age(age),birthday(birthday),nation(nation),political(political),education(education),college(college),
        tel(tel),join_date(join_date),face_id(face_id){}
    QString id;
    QString department;
    QString position;
    QString name;
    QString id_card;
    QString gender;
    int age;
    QString birthday;
    QString nation;
    QString political;
    QString education;
    QString college;
    QString tel;
    QString join_date;
    int face_id;
};

// 考勤记录结构体
struct ATTENDANCE_INFO
{
    ATTENDANCE_INFO() {}
    ATTENDANCE_INFO(QString num,QString id,QString name,QString department,QString position,QString time,QString type,QString state)
        :num(num),id(id),name(name),department(department),position(position),time(time),type(type),state(state){}
    QString num;
    QString id;
    QString name;
    QString department;
    QString position;
    QString time;
    QString type;
    QString state;
};

// 数据库类：单例模式
class DataBase : public QObject
{
    Q_OBJECT
public:
    // 析构函数
    ~DataBase();
    // 提供一个获取唯一示例的方法
    static DataBase& getInstance();
    // 用户注册
    // 账号 密码 姓名 身份证号码 性别 年龄 手机号码 头像路径
    // id passwd name id_card gender age tel icon_path
    bool userRegist(QString id,QString passwd,QString name,QString id_card,QString gender,
            int age,QString tel,QString icon_path);
    // 获取用户的头像路径
    QString getUserIconPath(QString user_id);
    // 用户登录
    QString userLoing(QString id,QString passwd);
    // 获取用户的姓名
    QString getUserName(QString id);
    // 获取所有部门信息
    QStringList getAllDepartment();
    // 添加部门
    bool addDepartment(QString department);
    // 删除部门
    bool deleteDepartment(QString department);
    // 获取指定部门的岗位信息
    QStringList getPosition(QString department);
    // 添加岗位
    bool addPosition(QString department,QString position);
    // 删除岗位
    bool deletePosition(QString department,QString position);
    // 注册员工
    bool employeeRegister(QString id,QString department,QString position,QString name,QString id_card,
                          QString gender,int age,QString birthday,QString nation,QString political,
                          QString education,QString college,QString tel,QString join_date,int face_id);
    // 获取所有员工信息
    QList<EMPLOYEE_INFO> getAllEmployeeInfo();
    // 获取一个员工的信息
    EMPLOYEE_INFO getEmployeeInfo(QString id);
    // 通过人脸ID来获取员工信息
    EMPLOYEE_INFO getEmployeeInfoByFaceID(int64_t face_id);
    // 获取所有的考勤记录
    QList<ATTENDANCE_INFO> getAllAttendanceRecord();

private:
    // 初始化数据库
    bool dataBaseInit();

private:
    // 隐藏构造函数
    explicit DataBase(QObject *parent = nullptr);
    // 删除拷贝构造函数
    DataBase(const DataBase&) = delete;
    // 删除赋值运算符函数
    DataBase& operator=(const DataBase&) = delete;

private:
    QSqlDatabase *m_db; // 数据库连接对象
};

#endif // DATABASE_H
