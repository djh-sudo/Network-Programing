#include "server.h"
#include <QApplication>
#include<QDebug>
#include"database.h"
#include"md5.h"
#include<QMap>
#include<QDebug>
#include<QDateTime>
#include<windows.h>
#include<QStorageInfo>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Server w;
    w.show();
    w.setFixedSize(1523,762);
    w.setWindowTitle("Server @copytight by DJH 2021");
    return a.exec();
}
