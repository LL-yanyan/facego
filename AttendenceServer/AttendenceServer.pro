QT += widgets sql network

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    database.cpp \
    faceobject.cpp \
    logindialog.cpp \
    main.cpp \
    serverwindow.cpp

HEADERS += \
    database.h \
    faceobject.h \
    logindialog.h \
    serverwindow.h

FORMS += \
    logindialog.ui \
    serverwindow.ui

# 添加opencv和SeetaFace的头文件搜索路径
INCLUDEPATH += D:\opencv\opencv452\include
INCLUDEPATH += D:\opencv\opencv452\include\opencv2
INCLUDEPATH += D:\opencv\SeetaFace\include
INCLUDEPATH += D:\opencv\SeetaFace\include\seeta
# 添加opencv和SeetaFace库文件搜索路径
LIBS += D:\opencv\opencv452\x64\mingw\lib\libopencv*
LIBS += D:\opencv\SeetaFace\lib\libSeeta*

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    src.qrc
