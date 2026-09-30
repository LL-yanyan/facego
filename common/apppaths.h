#ifndef APPPATHS_H
#define APPPATHS_H

#include <QCoreApplication>
#include <QDir>
#include <QString>

// ============================================================
//  集中管理可移植路径，避免源码中出现任何硬编码绝对路径
//  规则：
//   - 可写的运行时数据（数据库 / 人脸库 / 配置 / 用户头像）
//     统一放在“可执行文件所在目录”，便于开发与部署；
//   - 第三方只读资源（SeetaFace 模型、OpenCV 级联分类器）
//     优先使用 CMake 注入的目录，未注入时回退到 exe 同级目录。
// ============================================================
namespace apppaths {

// 可执行文件所在目录（可写数据根目录）
inline QString appDir()
{
    return QCoreApplication::applicationDirPath();
}

// 运行时可写数据文件：database.db / face.db / login.ini
inline QString dataFile(const QString &fileName)
{
    return QDir(appDir()).filePath(fileName);
}

// 用户头像目录 user_imgs（不存在则自动创建）
inline QString userImageDir()
{
    const QString dir = QDir(appDir()).filePath(QStringLiteral("user_imgs"));
    QDir().mkpath(dir);
    return dir;
}

// 用户头像文件路径（按工号命名）
inline QString userImageFile(const QString &baseName)
{
    return QDir(userImageDir()).filePath(baseName + QStringLiteral(".jpg"));
}

// SeetaFace 模型目录：优先 CMake 注入的 SEETAFACE_MODEL_DIR，否则用 exe/model
inline QString modelDir()
{
#ifdef SEETAFACE_MODEL_DIR
    return QStringLiteral(SEETAFACE_MODEL_DIR);
#else
    return QDir(appDir()).filePath(QStringLiteral("model"));
#endif
}

// SeetaFace 模型文件路径
inline QString modelFile(const QString &fileName)
{
    return QDir(modelDir()).filePath(fileName);
}

// OpenCV 级联分类器目录：优先 CMake 注入的 OPENCV_HAARCASCADE_DIR，否则用 exe/haarcascades
inline QString cascadeDir()
{
#ifdef OPENCV_HAARCASCADE_DIR
    return QStringLiteral(OPENCV_HAARCASCADE_DIR);
#else
    return QDir(appDir()).filePath(QStringLiteral("haarcascades"));
#endif
}

// OpenCV 级联分类器文件路径
inline QString cascadeFile(const QString &fileName)
{
    return QDir(cascadeDir()).filePath(fileName);
}

} // namespace apppaths

#endif // APPPATHS_H
