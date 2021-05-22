#include "client.h"

#include <QApplication>
#include"register.h"
#include"userpanel.h"
#include"client.h"
#include"cachefile.h"
#include"md5.h"
#include<QDateTime>
#include<QVector>
#include<QStringList>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Client w;
    UserPanel u;
    Register r;
    w.setFixedSize(542,342);
    w.setWindowTitle("QTQQ-Client @copyright 2021 by DJH");
    w.setAttribute(Qt::WA_DeleteOnClose);

    u.setFixedSize(900,810);
    u.setWindowTitle("QTQQ-Client");

    r.setFixedSize(690,400);
    r.setWindowTitle("QTQQ-Client-Register");
    r.setAttribute(Qt::WA_DeleteOnClose);
    w.show();

    QObject::connect(&w,SIGNAL(regShow()),&r,SLOT(showReg()));
    QObject::connect(&w,SIGNAL(panShow(QString)),&u,SLOT(showPan(QString)));
    QObject::connect(&r,SIGNAL(showClient()),&w,SLOT(showClt()));

    QObject::connect(&u,SIGNAL(sendData(QString)),&w,SLOT(handle(QString)));
    QObject::connect(&w,SIGNAL(sendData(QString)),&u,SLOT(handle(QString)));


    return a.exec();
}
