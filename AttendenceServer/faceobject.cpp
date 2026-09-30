#include "faceobject.h"
#include "apppaths.h"

FaceObject::FaceObject(QObject *parent)
    :QObject(parent)
{
    // 初始化人脸引擎
    init();
}

FaceObject::~FaceObject()
{
    delete face_engine;
}

void FaceObject::init()
{
    // 在实例化人脸引擎的时候需要提供3个模型：FD(人脸检测)，PD(人脸特征)，FR(人脸识别)
    // 这3个模型的类型是SeetaModelSetting，但是它是一个结构体类型，后续操作该对象不方便，使用它的派生类ModelSetting
    // 模型路径由 apppaths 统一管理（CMake 注入 SEETAFACE_MODEL_DIR，默认第三方库 bin/model）
    const QByteArray fd_model = apppaths::modelFile("fd_2_00.dat").toUtf8();
    const QByteArray pd_model = apppaths::modelFile("pd_2_00_pts5.dat").toUtf8();
    const QByteArray fr_model = apppaths::modelFile("fr_2_10.dat").toUtf8();
    seeta::ModelSetting FD_model(fd_model.constData());
    seeta::ModelSetting PD_model(pd_model.constData());
    seeta::ModelSetting FR_model(fr_model.constData());
    // 实例化一个人脸引擎对象
    face_engine = new seeta::FaceEngine(FD_model,PD_model,FR_model);
    // 加载人脸数据库
    // 加载人脸特征库（运行时数据，位于可执行文件目录）
    face_engine->Load(apppaths::dataFile("face.db").toUtf8().constData());
}

int64_t FaceObject::faceRegister(cv::Mat &face_img)
{
    // 将opencv中的Mat数据转换为seeta中的图像数据
    SeetaImageData seeta_img;
    seeta_img.data = face_img.data; // 图像数据
    seeta_img.width = face_img.cols;    // 图像宽度
    seeta_img.height = face_img.rows;   // 图像高度
    seeta_img.channels = face_img.channels();   // 颜色通道数
    // 注册人脸，如果注册成功，返回人脸ID值，否则返回-1
    int64_t face_id = face_engine->Register(seeta_img);
    if(face_id >= 0)
    {
        // 人脸注册成功了，将人脸数据保存到人脸数据库中去
        face_engine->Save(apppaths::dataFile("face.db").toUtf8().constData());
    }
    return face_id;
}

int64_t FaceObject::faceQuery(cv::Mat &face_img,QTcpSocket *socket)
{
    // 将opencv中的Mat数据转换为seeta中的图像数据
    SeetaImageData seeta_img;
    seeta_img.data = face_img.data; // 图像数据
    seeta_img.width = face_img.cols;    // 图像宽度
    seeta_img.height = face_img.rows;   // 图像高度
    seeta_img.channels = face_img.channels();   // 颜色通道数
    // 查询人脸，如果查询到了，返回这个人脸所对应的id，否则返回-1，可以通过第二个参数，来接收人脸匹配度
    float similarity = 0;   // 用来保存两个人脸的匹配度(相似度)
    int64_t face_id = face_engine->Query(seeta_img,&similarity);
    qDebug() << face_id << similarity;
    // 如果匹配度大于百分之80，则判定为匹配成功
    if(similarity < 0.8)
    {
        emit sendFaceID(-1,socket);
        return -1;
    }
    // 人脸匹配度大于百分之80，将与之对应的人脸id发送出去
    emit sendFaceID(face_id,socket);
    return face_id;
}
