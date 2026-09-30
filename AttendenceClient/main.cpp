#include "attendencewindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    AttendenceWindow w;
    w.show();
    return QApplication::exec();
}
