#include "client.h"
#include "ui_client.h"
#include<QDebug>
#include<QHostAddress>
#include<QByteArray>
#include<QPixmap>
#include<md5.h>
#include"user.h"
#include<register.h>
#include<QRegExp>
#include<QNetworkProxy>
#include<QAbstractSocket>
#include<QMessageBox>
extern User user;
static bool index = true;
extern QString __IP__ = "127.0.0.1";
Client::Client(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Client)
{
    ui->setupUi(this);
    socket = NULL;
    ui->login->setDefault(true);
    socket = new QTcpSocket(this);
//    time = new QTimer();
    //time->start(1000);
    QRegExp rx("\\b(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\b");  //IP的正则表达式
    ui->lineEdit->setValidator(new QRegExpValidator(rx,this)); //设置
    socket->setProxy(QNetworkProxy::NoProxy);
    socket->abort();

//    connect(time,&QTimer::timeout,this,[&](){

//    });

    ui->userId->setStyleSheet("border:3px solid gray;border-radius:12px;padding:1px 4px");
    ui->login->setStyleSheet("border:3px;border-radius:12px;padding:1px 4px;background-color:rgba(0,201,87,1)");
    ui->password->setStyleSheet("border:3px solid gray;border-radius:12px;padding:1px 4px");
    ui->label_3->setText("未连接请确认");
    ui->login->setFocus();
    QIcon error;
    error.addFile(tr(":/error.png"));
    ui->connectServer->setIcon(error);
    QIcon head;
    head.addFile(tr(":/QQ.png"));
    ui->head->setIcon(head);
    //
    connect(socket,&QTcpSocket::connected,[=](){
        ui->label_3->setText("连接成功");
//        time->stop();
        ui->connectServer->setEnabled(false);
        QIcon right;
        right.addFile(tr(":/right.png"));
        ui->connectServer->setIcon(right);

        //修改相关标识信息

        connect(socket,&QTcpSocket::readyRead,[&](){
            //对收到的文件进行处理
            QByteArray buffer = socket->readAll();
            if(QString(buffer).section("##",0,0) == "login"){
                if(QString(buffer).section("##",1,1) == "ok"){
                    int userId = QString(buffer).section("##",2,2).toUtf8().toInt();
                    QString userName = QString(buffer).section("##",3,3);
                    QString userPassword =QString(buffer).section("##",4,4);
                    bool state = QString(buffer).section("##",5,5).toUtf8().toInt();
                    int number = QString(buffer).section("##",6,6).toUtf8().toInt();

                    user.setName(userName);
                    user.setPassword(userPassword);
                    user.setId(userId);
                    user.setState(state);
                    user.setFriendNumber(number);
                    user.setIpPort(userName,socket->localAddress().toString(),socket->localPort());


                    this->hide();
                    emit panShow(ui->lineEdit->text());
                }else if(QString(buffer).section("##",1,1) == "fail"){
                    ui->password->clear();
                    ui->password->setStyleSheet("border:3px solid red;border-radius:12px;padding:1px 4px");
                    ui->message->setText("账号/密码不匹配!");
                }
                else if(QString(buffer).section("##",1,1) == "regis"){
                    ui->password->clear();
                    ui->login->click();
                    ui->login->setStyleSheet("border:3px solid red;border-radius:12px;padding:1px 4px");
                    ui->message->setText("请先注册账号!");
                }
            }
            else{
                emit sendData(buffer);
            }
        });

        connect(socket,&QTcpSocket::disconnected,[=]{
            QIcon error;
            error.addFile(tr(":/error.png"));
            ui->connectServer->setIcon(error);
            ui->label_3->setText("未连接请确认");
//            time->start(1000);
            ui->connectServer->setEnabled(true);
            user.setState(false);
            if(socket!= NULL){
                socket->disconnectFromHost();
                socket->close();
            }
            emit sendData("exit");
        });
    });
}

Client::~Client()
{
//    if(time->isActive()){
//        time->stop();
//        delete time;
//    }
    if(socket!=NULL){
        socket->disconnectFromHost();
        socket->close();
    }
    delete socket;
    delete ui;
}

void Client::on_connectServer_clicked()
{
    __IP__ = ui->lineEdit->text();
//    index = !index;
//    if(!index){
//        time->stop();
//        QIcon stop;
//        stop.addFile(tr(":/stop.png"));
//        ui->connectServer->setIcon(stop);
//        ui->label_3->setText("停止尝试");
//    }
//    if(index){
//        time->start(1000);
        QIcon error;
        error.addFile(tr(":/error.png"));
        ui->connectServer->setIcon(error);
        ui->label_3->setText("未连接请确认");
        QString ip = __IP__;
        qDebug()<<ip;
        quint16 port = 8888;
        socket->abort();
        socket->connectToHost(QHostAddress(ip),port);
//    }
}

void Client::on_login_clicked()//发送账户和密码验证
{
    ui->message->setText("");
    QString password = ui->password->text();
    QString userId = ui->userId->text();
    if(password == "" || userId == ""){
        ui->message->setText("账号/密码不可以为空!");
        return;
    }
    QString res = "login##"+userId+"##"+MD5::Md5(password)+"##";
    socket->write(res.toUtf8().data());
    user.setId(ui->userId->text().toUtf8().toInt());
    return;
}

void Client::on_register_2_clicked()//
{
    if(socket!= NULL){
        socket->disconnectFromHost();
        socket->close();
    }
//    if(time->isActive())
//        time->stop();
    this->hide();
    emit regShow();
    return;
}

void Client::showClt(){
//    if(time->isActive())
//        time->stop();
    this->show();
}
void Client::handle(QString s){//收到用户请求
    socket->write(s.toUtf8().data());
}

//========================关闭窗口事件========================
void Client::closeEvent(QCloseEvent *event){
    emit quit();
    exit(0);
}

