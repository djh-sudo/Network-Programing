#ifndef SERVER_H
#define SERVER_H

#include <QMainWindow>
#include<QTcpServer>
#include<QTcpSocket>
#include<QVector>
#include"database.h"
#include"mydatabase.h"
#include<QTimer>
#include<userdata.h>
#include<QUdpSocket>
QT_BEGIN_NAMESPACE
using namespace std;
namespace Ui { class Server; }
QT_END_NAMESPACE

class Server : public QMainWindow
{
    Q_OBJECT

public:
    explicit Server(QWidget *parent = nullptr);
    int userLogin(int id,QString password);
    bool userRegister(int id,QString userName,QString password,int friendNumber);
    QStringList readfromCache(QString idRev);//把文件数据读入到内存
    void writetoCache(QString messageHead,QString idRev,QString idSend);//把文件数据写入磁盘
    void assembleData(QVector<UserData> data,QString idRev,QString idSend);
    void handleMessage(UserData data,QString idRev,QString idSend);
    QString currentTime();
    void inicialUserTabel();
    void inicialUserGroup();
    void udpSend(int idRecv,QString s);
    void saveLocalCache(QString messageHead,int idRev,int idSend,QString name, QString content,QString time);
    void DISK();
    /*用户行为处理函数*/
    bool addFriend(QString buffer);
    bool agreementFriend(QString buffer,QTcpSocket* socket);
    bool SearchFriend(QString buffer,QTcpSocket*socket);
    bool UserLogin(QString buffer,QTcpSocket*socket);
    bool UserRegister(QString buffer,QTcpSocket*socket);
    bool UserCreateGroup(QString buffer,QTcpSocket*socket);
    void UserInicial(QString buffer,QTcpSocket*socket);
    bool SearchGroup(QString buffer,QTcpSocket*socket);
    bool addComponent(QString buffer);
    /*析构*/
    ~Server();
private slots:
    void updateProcess();
    void Handle(QString);
    void successfully();
    void successfully_1();
    void Brackets();
    void Brackets_1();
    void USERID();
    void USERID_1();
    void Fail();
    void Fail_1();

private:
    Ui::Server *ui;
    QTcpServer *tcpServer;
    DataBase* db;
    QUdpSocket *udpSocket;
    QTimer*timer;
    QTimer *t_cpu;

};
#endif // SERVER_H
