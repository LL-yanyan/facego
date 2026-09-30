#ifndef FACEOBJECT_H
#define FACEOBJECT_H

#include <QObject>
#include <opencv.hpp>
#include <seeta/FaceEngine.h>
#include <QTcpSocket>

class FaceObject : public QObject
{
    Q_OBJECT

signals:
    void sendFaceID(int64_t face_id,QTcpSocket *socket);    // 注册或者识别完成以后，将人脸id通过信号发送出来

public:
    FaceObject(QObject *parent = nullptr);
    ~FaceObject();
    void init();

public slots:
    // 注册人脸(返回注册后的人脸ID)
    int64_t faceRegister(cv::Mat &face_img);
    // 查询人脸(返回匹配到的人脸ID)
    int64_t faceQuery(cv::Mat &face_img,QTcpSocket *socket);

private:
    seeta::FaceEngine *face_engine; // 人脸引擎对象指针
};

#endif // FACEOBJECT_H
