#include "register.h"
#include "ui_register.h"
#include<QRegExp>
#include<QHostAddress>
#include<QTcpSocket>
#include<QDateTime>
#include<QClipboard>
#include"md5.h"
int flag = 0;
int check = 0;
static bool index = true;
extern QString __IP__;
Register::Register(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Register)
{
    ui->setupUi(this);
    socket = NULL;
    socket = new QTcpSocket(this);
    time = new QTimer;
    ui->name->setStyleSheet("border:3px solid gray;border-radius:12px;padding:1px 4px");
    ui->password1->setStyleSheet("border:3px solid gray;border-radius:12px;padding:1px 4px");
    ui->password2->setStyleSheet("border:3px solid gray;border-radius:12px;padding:1px 4px");
    ui->register_2->setStyleSheet("border:3px;border-radius:12px;padding:1px 4px;background-color:rgba(0,201,87,1)");
    ui->pushButton_7->setStyleSheet("border:3px;border-radius:12px;padding:1px 4px;background-color:rgba(0,201,87,1)");
    ui->pushButton_9->setStyleSheet("border:3px;border-radius:12px;padding:1px 4px;background-color:rgba(0,201,87,1)");
    ui->info->hide();
    ui->info_2->hide();
    ui->pushButton_7->hide();
    ui->copy->hide();
    QIcon error;
    error.addFile(tr(":/error.png"));
    ui->connectServer->setIcon(error);

    connect(time,&QTimer::timeout,this,[=]{
        QString ip = __IP__;
        qint16 port = 8888;
        socket->connectToHost(QHostAddress(ip),port);
    });

    connect(socket,&QTcpSocket::connected,[=](){
        time->stop();
        check = 1;
        QIcon right;
        right.addFile(tr(":/right.png"));
        ui->connectServer->setIcon(right);
        ui->info_3->clear();
        connect(socket,&QTcpSocket::disconnected,[=](){
            check = 0;
            QIcon error;
            error.addFile(tr(":/error.png"));
            ui->connectServer->setIcon(error);
            time->start(1000);
            ui->info_3->setText("网络异常");
        });

        connect(socket,&QTcpSocket::readyRead,[=](){
            QByteArray buffer = socket->readAll();
            if(QString(buffer).section("##",0,0) == "regis"){
                QString res = QString(buffer).section("##",1,1);
                if (res == "ok"){//注册成功
                    ui->info->show();
                    ui->info_2->show();
                    ui->pushButton_7->show();
                    ui->info_3->clear();
                    ui->pushButton_9->hide();
                    ui->name->setEnabled(false);
                    ui->password1->setEnabled(false);
                    ui->password2->setEnabled(false);
                    ui->userid->setHidden(false);
                    ui->register_2->setEnabled(false);
                    ui->copy->show();
                }
                else if(res == "no"){
                    check = 0;
                }
            }
        });
    });
    //

}

Register::~Register()
{
    qDebug()<<"~Register";
    if(time->isActive())
        time->stop();
    delete time;
    if(socket!=NULL){
        socket->disconnectFromHost();
        socket->close();
    }
    delete socket;
    delete ui;
}

void Register::on_name_textChanged(const QString &arg1)
{
    if(arg1.size()<3){ //
        ui->label_2->setText("用户名太短");
        ui->name->setStyleSheet("QLineEdit{border:3px solid rgb(255,0,0);border-radius:12px;padding:1px 4px}");
        flag = 1;
    }else if(arg1.size()>=10){
        ui->label_2->setText("用户名太长");
        ui->name->setStyleSheet("QLineEdit{border:3px solid rgb(255,0,0);border-radius:12px;padding:1px 4px}");
        flag = 1;
    }
    else if(arg1.contains("#")||arg1.contains("$")||arg1.contains("@")){
        ui->label_2->setText("用户名包含非法字符");
        ui->name->setStyleSheet("QLineEdit{border:3px solid rgb(255,0,0);border-radius:12px;padding:1px 4px}");
        flag = 1;
    }
    else{
        ui->label_2->clear();
        ui->name->setStyleSheet("border:3px solid rgb(0,238,0);border-radius:12px;padding:1px 4px");
        flag = 0;
    }
}

void Register::on_register_2_clicked()//注册按钮
{
    if(flag == 1 && check == 1){
        QString res = "regis##";
        QString id = produceId();
        ui->userid->setText(id);
        ui->userid->setHidden(true);
        QString name = ui->name->text();
        QString psw = ui->password2->text();
        res = res+id+"##"+name+"##"+psw+"##";
        socket->write(res.toUtf8().data());//发送消息
    }
    else if(check == 0){
        ui->info_3->setText("网络异常!");
    }
    else if(flag == 0){
        ui->info_3->setText("账号/密码有误!");
    }
    else return;
    return;
}

void Register::on_password1_textChanged(const QString &arg1)
{
    QRegExp rx;
    rx.setPattern(QString("^(?![^\\da-zA-Z]+$).{6,16}$"));
    if(arg1.size()<6){ //
        ui->label_3->setText("密码太短");
        ui->password1->setStyleSheet("QLineEdit{border:3px solid rgb(255,0,0);border-radius:12px;padding:1px 4px}");
        if(!rx.exactMatch(arg1))
        {
            ui->password1->setStyleSheet("QLineEdit{border:3px solid rgb(255,0,0);border-radius:12px;padding:1px 4px}");
            ui->label_3->setText("密码需包含字母和数字");

        }
        flag = 0;
    }
    if(arg1!="" && rx.exactMatch(arg1)){
        ui->label_3->clear();
        ui->password1->setStyleSheet("border:3px solid rgb(0,238,0);border-radius:12px;padding:1px 4px");
        flag = 1;
    }
}

void Register::on_password2_textChanged(const QString &arg1)
{
    QString psw = ui->password1->text();
    if(arg1!=psw && arg1!=""){
        ui->password2->setStyleSheet("QLineEdit{border:3px solid rgb(255,0,0);border-radius:12px;padding:1px 4px}");
        ui->label_4->setText("两次密码不一致");
        flag = 0;
    }
    else{
        ui->label_4->clear();
        ui->password2->setStyleSheet("border:3px solid rgb(0,238,0);border-radius:12px;padding:1px 4px");
        flag = 1;
    }
}

void Register::on_toolButton_pressed()
{
    ui->password2->setEchoMode(QLineEdit::Normal);
}

void Register::on_toolButton_released()
{
    ui->password2->setEchoMode(QLineEdit::Password);
}

void Register::on_pushButton_7_clicked()
{
    if(socket!=NULL){
        socket->disconnectFromHost();
        socket->close();
    }
    if(time->isActive()){
        time->stop();
    }
    this->hide();
    emit showClient();
    this->close();
}
void Register::showReg(){
    if(time!=nullptr){
        time->start(1000);
    }
    this->show();
}

void Register::on_connectServer_clicked()
{
    index = !index;
    if(!index){
        time->stop();
        QIcon stop;
        stop.addFile(tr(":/stop.png"));
        ui->connectServer->setIcon(stop);
        ui->info_3->setText("停止尝试");
    }
    if(index){
        time->start(1000);
        QIcon error;
        error.addFile(tr(":/error.png"));
        ui->connectServer->setIcon(error);
        ui->info_3->setText("未连接请确认");
    }
}

void Register::on_pushButton_9_clicked()
{
    if(socket!=NULL){
        socket->disconnectFromHost();
        socket->close();
    }
    if(time->isActive()){
        time->stop();
    }
    this->hide();
    emit showClient();
    this->close();
}

QString Register::produceId(){
    QDateTime time = QDateTime::currentDateTime();
    QString s = time.toString("yyyy-MM-dd hh:mm:ss");
    s = MD5::Md5(s);
    s = "0x" + s.left(6);
    int value = s.toInt(NULL,16);
    return QString::number(value);
}

void Register::on_copy_clicked()
{
    QClipboard *clip = QApplication::clipboard();
    clip->setText(ui->userid->text());
}
void Register::Close(){
    this->close();
}
