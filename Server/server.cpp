#include "server.h"
#include "ui_server.h"
#include"user.h"
#include"md5.h"
#include<QString>
#include<vector>
#include<QDebug>
#include<QtSql/QSqlDatabase>
#include"userdata.h"
#include"cachefile.h"
#include<QMap>
#include<QDateTime>
#include<QVector>
#include<QDir>
#include<QFileInfoList>
#include<QMessageBox>
#include<QSettings>
#include<windows.h>
#include"func.h"
#include<QStandardItemModel>
#include<QStorageInfo>
#include<QProgressBar>
using namespace std;

/********************* Gloable Var *********************/

QMap<int,User>USER;//user information
QMap<QString,int>SocketMap;//<hash(socket)->user id>
QMap<int,QMap<int,QVector<UserData>>>message; //user message
QMap<int,int>userlist;//<user id,user location in userlist>
QMap<int,int>grouplist;//<group id,group location in grouplist>

/********************* Constructure function *********************/
Server::Server(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Server)
{
    ui->setupUi(this);
    //inicial the partial widget

    ui->online->clear();
    ui->data->clear();
    ui->listWidget->clear();
    ui->listWidget_2->clear();

    //inicial the widget color
    ui->data->setTextColor(QColor(232,232,232));
    ui->online->setTextColor(QColor(232,232,232));
    QPalette pa=ui->data->palette();
    pa.setColor(QPalette::Base,QColor(54,54,54));
    ui->data->setPalette(pa);
    ui->online->setPalette(pa);
    ui->listWidget->setPalette(pa);
    ui->listWidget_2->setPalette(pa);

    //inicial the timer
    timer = new QTimer();
    timer->setInterval(500);
    timer->start();
    t_cpu = new QTimer();
    t_cpu->setInterval(2000);
    t_cpu->start();

    //make connect between signal and the slot function
    connect(timer,SIGNAL(timeout()),this,SLOT(successfully()),Qt::QueuedConnection);
    connect(timer,SIGNAL(timeout()),this,SLOT(successfully_1()),Qt::QueuedConnection);
    connect(timer,SIGNAL(timeout()),this,SLOT(Brackets()),Qt::QueuedConnection);
    connect(timer,SIGNAL(timeout()),this,SLOT(Brackets_1()),Qt::QueuedConnection);
    connect(timer,SIGNAL(timeout()),this,SLOT(USERID()),Qt::QueuedConnection);
    connect(timer,SIGNAL(timeout()),this,SLOT(USERID_1()),Qt::QueuedConnection);
    connect(timer,SIGNAL(timeout()),this,SLOT(Fail()),Qt::QueuedConnection);
    connect(timer,SIGNAL(timeout()),this,SLOT(Fail_1()),Qt::QueuedConnection);
    connect(t_cpu,SIGNAL(timeout()),this,SLOT(updateProcess()),Qt::QueuedConnection);

    //hide the scroll bar
    ui->data->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->online->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    //inicial the globle var
    message.clear();
    userlist.clear();
    grouplist.clear();
    USER.clear();
    SocketMap.clear();

    //show the cpu information
    QSettings *CPU = new QSettings("HKEY_LOCAL_MACHINE\\HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0",QSettings::NativeFormat);
    QString s = CPU->value("ProcessorNameString").toString();
    delete CPU;
    ui->data->append("[#CPU infomation] " + s);

    //inicial data base
    db = new DataBase("SERVER","DATABASE.db",this);
    connect(db,SIGNAL(Send(QString)),this,SLOT(Handle(QString)));
    db->inicialTable();
    db->createTable(USERINFO_DATABASE_TABLE);
    db->createGroup(USERGROUP_DATABASE_TABLE);
    inicialUserTabel();
    inicialUserGroup();

    //inicial the disk information
    DISK();
    //inicial the tcpsocket the tcp port is 8888
    tcpServer = new QTcpServer(this);
    tcpServer->listen(QHostAddress::Any,8888);

    //inicial the udpsocket is 9999
    udpSocket = new QUdpSocket();
    udpSocket->bind(QHostAddress::Any,9999);

    // trigger the new connect
    connect(tcpServer,&QTcpServer::newConnection,[=](){
        QTcpSocket* socket = tcpServer->nextPendingConnection();
        QString str = QString("[# %1:%2] connected!").arg(socket->peerAddress().toString()).arg(socket->peerPort());
        ui->online->append(str);

        //disconnect from the server
        connect(socket,&QTcpSocket::disconnected,[=](){
            if(socket!=NULL){
                socket->disconnectFromHost();
                socket->close();
            }
            QString str = QString("[# %1:%2] disconnected!").arg(socket->peerAddress().toString()).arg(socket->peerPort());
            ui->online->append(str);
            QString hash = MD5::Md5(socket->peerAddress().toString()+QString::number(socket->peerPort()));
            int uid = SocketMap[hash];
            if(uid!=0){
                ui->online->append("<" + QString::number(uid) + "> offline");
                db->resetState(USERINFO_DATABASE_TABLE,uid);
                int k = userlist[uid];
                ui->listWidget->item(k)->setIcon(QIcon(":/off.png"));
                ui->listWidget->item(k)->setTextColor(QColor(139,139,137));
                QMap<QString,int>::iterator it = SocketMap.find(hash);
                if(it!=SocketMap.end()){
                    SocketMap.erase(it);
                }
            }
        });

        //send message trigger readyread[Tcp information]
        connect(socket, &QTcpSocket::readyRead,[=](){
            QByteArray buffer = socket->readAll();
            QStringList content = QString(buffer).split("##");
            for(unsigned int i = 0;i<content.size();){
                if(content[i] == USER_LOGIN){//处理登录请求
                    QString s = QString(buffer).section("##",i,i+3);
                    UserLogin(s,socket);
                    i = i + 3;
                }
                else if(content[i] == USER_REGISTER){//处理注册请求
                    QString s =QString(buffer).section("##",i,i+4);
                    UserRegister(s,socket);
                    i = i + 4;
                }
                else if(content[i] == USER_ADD_FRIEND){//添加好友请求
                    QString s = QString(buffer).section("##",i,i+3);
                    if(addFriend(s) == true){
                        ui->online->append("[# add] add friend is successfully!");
                    }
                    else
                        ui->online->append("[# add] add friend is failed!");
                    i = i + 3;
                }
                else if(content[i] == DELETE_USERS){//删除好友管理
                    QString idSend = content[i+1];
                    int number = content[i+2].toUtf8().toInt();
                    for(int k = 0;k<number;k++){
                        QString idFriend = content[i+k+3];
                        //delete
                        db->deleteFriend(USERINFO_DATABASE_TABLE,idSend.toUtf8().toInt());
                        db->deleteFriend(USERINFO_DATABASE_TABLE,idFriend.toUtf8().toInt());
                        db->deleteFriendData(idSend,idFriend.toUtf8().toInt());
                        db->deleteFriendData(idFriend,idSend.toUtf8().toInt());
                        if(db->checkState(idFriend.toUtf8().toInt(),USERINFO_DATABASE_TABLE)){
                            QString s = "del##"+idSend+"##";
                            udpSend(idFriend.toUtf8().toInt(),s);
                        }else{
                            saveLocalCache(DELETE_USERS,idFriend.toUtf8().toInt(),idSend.toUtf8().toInt(),"del","",currentTime());
                        }
                    }
                    i = i + 3 + number;
                }
                else if(content[i] == USER_SEARCH){
                    QString s = QString(buffer).section("##",i,i+2);
                    if(SearchFriend(s,socket) == true){
                        ui->online->append("[# SearchFriend] Search Friend successfully!");
                    }
                    else{
                        ui->online->append("[# SearchFriend] Search Friend is not exists!");
                    }
                    i = i + 3;
                }
                else if(content[i] == USER_INICIAL){//客户端初始化完成
                    QString s = "##" + content[i+1];
                    UserInicial(s,socket);
                    i = i + 2;
                }
                else if(content[i] == AGREEMENT_TO_BE_FRIEND){//处理同意添加好友请求
                    QString s = QString(buffer).section("##",i,i+3);
                    if(agreementFriend(s,socket)){
                        ui->online->append("[# agreementFriend] agr successfully");
                    }
                    else{
                        ui->online->append("[# agreementFriend] agr failed");
                    }
                    i = i + 3;
                }
                else if(content[i] == CREATE_GROUP){//创建群聊消息
                    QString s = QString(buffer).section("##",i,i+4);
                    UserCreateGroup(s,socket);
                    i = i + 4;
                }
                else if(content[i] == SEARCH_GROUP){//搜索群聊
                    QString s = "##" + QString(buffer).section("##",i+1,i+2);
                    if(SearchGroup(s,socket)){
                        ui->online->append("[# SearchGroup] search group is successfully!");
                    }
                    else{
                        ui->online->append("[# SearchGroup] search group is failed!");
                    }
                    i = i + 3;
                }
                else if(content[i] == ADD_GROUP_COMPONENT){//添加人员
                    int number = content[i+3].toUtf8().toInt();
                    QString s = QString(buffer).section("##",i,i + number + 4);
                    if(addComponent(s)){
                        ui->online->append("[# addComponent] add group component is successfully!");
                    }else{
                        ui->online->append("[# addComponent] add group component is failed!");
                    }
                    i = i + number + 4;
                }
                else if(content[i] == EXIT_FROM_GROUP){//退出群聊
                    int number = content[i+2].toUtf8().toInt();
                    QString s = QString(buffer).section("##",i,i+number+3);
                    QString uid = QString(buffer).section("##",1,1);
                    for(int k=0;k<number;k++){
                        int gid = content[i+k+3].toUtf8().toInt();
                        QString messageHead = EXIT_FROM_GROUP;
                        QString res = QString::number(gid) + "#" + uid + "#";
                        if(db->getGroupAdmin(USERGROUP_DATABASE_TABLE,gid) == uid){
                            messageHead = ADMIN_EXIT_FROM_GROUP;
                            res = QString::number(gid);
                        }
                        QString Component = db->getAllGroupInfo(gid);
                        QStringList list = Component.split("##");
                        //数据库操作
                        if(messageHead == ADMIN_EXIT_FROM_GROUP){
                            if(db->deleteUserGroup(USERGROUP_DATABASE_TABLE,QString::number(gid))){
                                ui->data->append("[# deleteGroup] drop group <" + QString::number(gid) + " > is ok!");
                            }
                            else{
                                ui->data->append("[# deleteGroup] drop group <" + QString::number(gid) + " > is failed!");
                            }
                        }else{
                            if(db->deleteComponent(USERGROUP_DATABASE_TABLE,QString::number(gid),uid)){
                                ui->online->append("[# deleteComponent] user <" + uid +"> has been moved out!");
                            }
                            else{
                                ui->online->append("[# deleteComponent] user <" + uid +"> moved out failed!");
                            }
                        }
                        for(auto it:list){
                            if(it == "")
                                break;
                            if(it == uid)
                                continue;
                            if(db->checkState(it.toUtf8().toInt(),USERINFO_DATABASE_TABLE)){
                                udpSend(it.toUtf8().toInt(),messageHead+ "##" + res);
                            }else{
                                QString time = currentTime();
                                int idRev = it.toUtf8().toInt();
                                int idSend = uid.toUtf8().toInt();
                                saveLocalCache(messageHead,idRev,idSend,messageHead,QString::number(gid),time);
                            }
                        }
                    }
                    inicialUserGroup();
                    i = i + number + 3;
                }
                else if(content[i] == ""){
                    i = i + 1;
                }
                else{//消息解析失败
                    i = i + 1;
                }
            }
        });

    });

    // handle the udp message
    connect(udpSocket,&QUdpSocket::readyRead,[=](){
        QByteArray array;
        array.resize(udpSocket->bytesAvailable());
        quint16 port;
        QHostAddress address;
        udpSocket->readDatagram(array.data(),array.size(),&address,&port);
        QString res = array.data();
        if(res.section("##",0,0) == OTHER_UDP_MESSAGE){
            QString idSend = res.section("##",1,1);
            QString idRev = res.section("##",2,2);
            QString name = res.section("##",3,3);
            QString time = res.section("##",4,4);
            QString content = res.section("##",5,5);
            ui->online->append("[#udp rev] revice message from user <" + idSend+"> at " +time);
            bool flag = db->checkState(idRev.toUtf8().toInt(),USERINFO_DATABASE_TABLE);
            if(flag){
                res = "oth##" + idSend + "##" + name + "##" +time + "##" + content + "##";
                udpSend(idRev.toUtf8().toInt(),res);
            }
            else{//存到缓存，对方不在线
                UserData data =UserData(name,time,content);
                handleMessage(data,idRev,idSend);
                writetoCache("oth",idRev,idSend);
            }
        }else if(res.section("##",0,0) == GROUP_MESSAGE){
            QString idSend = res.section("##",1,1);
            QString gid = res.section("##",2,2);
            QString gname = res.section("##",3,3);
            QString name = ui->listWidget->item(userlist[idSend.toUtf8().toInt()])->text().section("\n",1,1).section(" ",2,2);
            QString time = res.section("##",4,4);
            QString content = res.section("##",5,5);
            ui->online->append("[#udp rev] revice message from user <" + idSend + "> in group <" + gid + "> at " + time);
            QString Component = db->getAllGroupInfo(gid.toUtf8().toInt());
            QStringList list = Component.split("##");
            QString sendContent = idSend + "##" + name + "##" + gid + "##" + time + "##" + content + "##";
            QString sendContent1 = name  + "@" + content + "@";
            for(auto it:list){
                if(it == "")
                    break;
                if(it == idSend)
                    continue;
                if(db->checkState(it.toUtf8().toInt(),USERINFO_DATABASE_TABLE)){
                    udpSend(it.toUtf8().toInt(),"grp##" + sendContent);
                }else{
                    int idRev = it.toUtf8().toInt();
                    saveLocalCache("grp",idRev,idSend.toUtf8().toInt(),gid,sendContent1,time);
                }
            }
        }
    });

}

/********************* user login *********************/
int Server::userLogin(int id, QString password){
    int res = db->checkUser(USERINFO_DATABASE_TABLE,id,MD5::Md5(password));
    return res;
}

/********************* user register *********************/
bool Server::userRegister(int id, QString userName,QString password,int friendNumber){
    return db->insertData(USERINFO_DATABASE_TABLE,id,userName,MD5::Md5(password),friendNumber);
}

/********************* write message into loca cache file *********************/
void Server::writetoCache(QString messageHead,QString idRev,QString idSend){//idRev收
    CacheFile file = CacheFile();
    QString res = "";
    for(int i=0;i<message.value(idRev.toUtf8().toInt()).value(idSend.toUtf8().toInt()).size();i++){
        QString name = message.value(idRev.toUtf8().toInt()).value(idSend.toUtf8().toInt())[i].getName();
        QString time = message.value(idRev.toUtf8().toInt()).value(idSend.toUtf8().toInt())[i].getTime();
        QString content = message.value(idRev.toUtf8().toInt()).value(idSend.toUtf8().toInt())[i].getContent();
        res += messageHead + "##" + idSend + "##" + name+ "##" +time + "##" + content + "\n";
    }
    file.writeFile(idRev,res);
    message.clear();
}

/********************* read the information from local cache file *********************/
QStringList Server::readfromCache(QString idRev){//从id账号中读取它的信息到内存
    CacheFile file = CacheFile();//idRev是文件夹名字
    //遍历文件夹
    QStringList res =  file.readFile(idRev);
    file.deleteFile(idRev);
    ui->data->append("[# readfromCache] <" + idRev + "> is released!");
    return res;
}

/********************* disconstruction function *********************/
Server::~Server()
{
    message.clear();
    userlist.clear();
    userlist.clear();
    USER.clear();
    SocketMap.clear();
    db->close();
    delete  udpSocket;
    delete tcpServer;
    delete ui;
    delete db;
    delete timer;
    delete t_cpu;
}

/********************* assemble user data *********************/
void Server::assembleData(QVector<UserData> data, QString idRev, QString idSend){
    QMap<int,QVector<UserData>>map;
    map.clear();
    map.insert(idSend.toUtf8().toInt(),data);
    message[idRev.toUtf8().toInt()] = map;//覆盖
    return;
}

void Server::handleMessage(UserData data, QString idRev, QString idSend){//socket 通信将消息存到服务器
    QVector<UserData>temp;
    temp.clear();
    if(message.contains(idRev.toUtf8().toInt())){//已经创建关键字
        temp = (message.value(idRev.toUtf8().toInt()).value(idSend.toUtf8().toInt()));
    }
    temp.push_back(data);
    assembleData(temp,idRev,idSend);
    temp.clear();
    return;
}

/********************* get current time *********************/
QString Server::currentTime(){
    QDateTime Time = QDateTime::currentDateTime();
    QString time = Time.toString("yyyy-MM-dd hh:mm");
    return time;
}

/********************* handle the message from server *********************/
void Server::Handle(QString s){
    ui->data->append(s);
    ui->data->moveCursor(QTextCursor::End);
}

/********************* update memory and cpu info *********************/
void Server::updateProcess(){
    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof (statex);
    GlobalMemoryStatusEx (&statex);
    ui->bar3_3->setPersent(statex.dwMemoryLoad);
    ui->bar3_4->setPersent((int)getCpuUsage());
}

/********************* send udp information *********************/
void Server::udpSend(int idRecv, QString s){
    QString ip = USER[idRecv].getIp();
    quint16 port = USER[idRecv].getUdpPort();
    udpSocket->writeDatagram(s.toUtf8().data(),QHostAddress(ip),port);
}

/********************* save infomation to local cache file *********************/
void Server::saveLocalCache(QString messageHead,int idRev,int idSend,QString name, QString content,QString time){
    UserData data =UserData(name,time,content);
    handleMessage(data,QString::number(idRev),QString::number(idSend));
    writetoCache(messageHead,QString::number(idRev),QString::number(idSend));
}

/********************* handle user event *********************/
bool Server::addFriend(QString buffer){//3
    QString idFriend = QString(buffer).section("##",1,1);//接收请求的人
    QString idMine = QString(buffer).section("##",2,2);//请求添加好友的人
    bool flag = db->checkState(idFriend.toUtf8().toInt(),USERINFO_DATABASE_TABLE);
    QString Info = db->allData(USERINFO_DATABASE_TABLE,idMine.toUtf8().toInt());
    QString name = Info.section("##",1,1);
    QString addTime = currentTime();
    if(flag){//被添加的用户在线
        QString res = "agr##" + idMine + "##"+name+"##"+addTime+"##"+name+"请求添加你为好友!##";
        QString ip = USER[idFriend.toUtf8().toInt()].getIp();
        qint16 port = USER[idFriend.toUtf8().toInt()].getUdpPort();
        udpSocket->writeDatagram(res.toUtf8().data(),QHostAddress(ip),port);
        return true;
    }
    else{//缓存在服务器本地
        UserData data =UserData(name,addTime,name+"请求添加你为好友!");
        handleMessage(data,idFriend,idMine);
        ui->online->append("[#save at the service] write to Cache is over!");
        writetoCache(ADD_FRIEND_OFFLINE,idFriend,idMine);//idrev,idsend
        return true;
    }
}
bool Server::agreementFriend(QString buffer,QTcpSocket* socket){//3
    QString idMine = QString(buffer).section("##",1,1);//申请添加好友人
    QString idFriend = QString(buffer).section("##",2,2);//收到请求的人[在线]
    bool flag1 = db->insertData(USERINFO_DATABASE_TABLE,idMine,idMine.toUtf8().toInt(),idFriend.toUtf8().toInt());
    bool flag2 = db->insertData(USERINFO_DATABASE_TABLE,idFriend,idFriend.toUtf8().toInt(),idMine.toUtf8().toInt());
    if(flag1){
        flag1 = db->addFriend(USERINFO_DATABASE_TABLE,idMine.toUtf8().toInt());
        flag2 = db->addFriend(USERINFO_DATABASE_TABLE,idFriend.toUtf8().toInt());
        if(flag1 && flag2){
            QString data = db->allData(USERINFO_DATABASE_TABLE,idMine.toUtf8().toInt());
            QString ack = "agr##" + idMine + "##" + data.section("##",1,1) + "##";
            socket->write(ack.toUtf8().data());//反馈给同意的一方
            //判断申请者
            bool flag = db->checkState(idMine.toUtf8().toInt(),USERINFO_DATABASE_TABLE);
            if(flag){//申请者在线
                QString ip = USER[idMine.toUtf8().toInt()].getIp();
                qint16 port = USER[idMine.toUtf8().toInt()].getUdpPort();
                QString name = db->allData(USERINFO_DATABASE_TABLE,idMine.toUtf8().toInt()).section("##",1,1);
                int k = userlist[idFriend.toUtf8().toInt()];
                QString friendName = ui->listWidget->item(k)->text().section("\n",1,1).section(" ",2,2);
                QString res = "uagr##" + idFriend + "##"+friendName+"##"+currentTime()+"##"+"添加为好友成功!##";
                udpSocket->writeDatagram(res.toUtf8().data(),QHostAddress(ip),port);
                return true;

            }
            else{//申请者离线
                data = db->allData(USERINFO_DATABASE_TABLE,idFriend.toUtf8().toInt());
                UserData userdata =UserData(data.section("##",1,1),"","添加为好友成功!");
                handleMessage(userdata,idMine,idFriend);
                writetoCache(AGREEMENT_TO_BE_FRIEND_OFFLINE,idMine,idFriend);
                return true;
            }
        }
    }
    else
        return false;
}
bool Server::SearchFriend(QString buffer, QTcpSocket *socket){//3
    int id = QString(buffer).section("##",1,1).toUtf8().toInt();
    QString idSend = QString(buffer).section("##",2,2);
    QString res = db->allData(USERINFO_DATABASE_TABLE,id);
    res = res.section("##",0,0) + "##" + res.section("##",1,1)+"##";//只截取id和name
    if(res!="####"){
        bool flag = db->is_Friend(idSend,QString::number(id));
        QString sendData = "";
        if(!flag)
            sendData  = "search##ok##" + res;
        else
            sendData = "search##okk##" + res;
        socket->write(sendData.toUtf8().data());
        return true;
    }else
    {
        QString sendData = "search##fail##";
        socket->write(sendData.toUtf8().data());
        return false;
    }
}
bool Server::UserLogin(QString buffer,QTcpSocket*socket){//3
    QString userId = QString(buffer).section("##",1,1);
    QString password = QString(buffer).section("##",2,2);
    //检查是否在线
    bool flag = db->checkState(userId.toUtf8().toInt(),USERINFO_DATABASE_TABLE);
    if(flag == false){
        int res = db->checkUser(USERINFO_DATABASE_TABLE,userId.toUtf8().toInt(),password);
        if(res == 1){
            db->setState(USERINFO_DATABASE_TABLE,userId.toUtf8().toInt());//上线
            QString data = db->allData(USERINFO_DATABASE_TABLE,userId.toUtf8().toInt());//获取相关的信息
            int uid = data.section("##",0,0).toUtf8().toInt();

            QStringList rawData = readfromCache(QString::number(uid));
            QString res = "";
            for(int k=0;k<rawData.size();k++)
                res += rawData[k] + "##";
            qDebug()<<"发送给客户端的消息内容 "<<res;
            socket->write(res.toUtf8().data());
            ui->online->append("user <" + QString::number(uid) + "> who's message is sent out all");
            socket->write("ini##");//给客户端一个信号
            USER[uid] = User(uid,socket->peerAddress().toString(),socket->peerPort(),(socket->peerPort()+1)%65535);
            QString socketHash = MD5::Md5(socket->peerAddress().toString()+QString::number(socket->peerPort()));
            SocketMap[socketHash] = uid;
            message.clear();//清空消息
        }
        else if (res == -1){//密码错误
            QString res = "login##fail##";
            socket->write(res.toUtf8().data());
            ui->online->append("[# UserLogin] <" + userId + "> password is incorrect! login failed!");
            return false;
        }
        else if(res == 0){//先注册
            QString res = "login##fail##";
            ui->online->append("[# UserLogin] <" + userId + "> plese regist first, login failed!");
            socket->write(res.toUtf8().data());
            return false;
        }
    }
    else{
        QString res = QString("[# %1,%2]").arg(socket->peerAddress().toString()).arg(socket->peerPort());
        ui->online->append(res+" is already online! connect! successfully");
        return true;
    }
}
bool Server::UserRegister(QString buffer,QTcpSocket*socket){//4
    QString id = buffer.section("##",1,1);
    QString name = buffer.section("##",2,2);
    QString psw =buffer.section("##",3,3);
    bool flag = db->insertData(USERINFO_DATABASE_TABLE,id.toUtf8().toInt(),name,MD5::Md5(psw),0);
    if(flag){
        bool isSuccess = db->createTable(id.toUtf8().toInt());
        if(isSuccess){
            ui->online->append("[# register at service] create database for user <" + id + "> is ok!");
            QString res = "regis##ok##";
            ui->listWidget->addItem(new QListWidgetItem(QIcon(":/off.png"),"user id " + id + "\nuser name "+ name + " "));
            int k = ui->listWidget->count();
            ui->listWidget->item(k-1)->setTextColor(QColor(139,137,137));
            userlist[id.toUtf8().toInt()] = k-1;
            socket->write(res.toUtf8().data());
            return true;
        }
        else{//数据库创建失败
            ui->online->append("[# register at service] create database for user <" + id + "> is failed!");
            QString res = "regis##no##";
            socket->write(res.toUtf8().data());
            return false;
        }
    }
    else{
        ui->online->append("[# register at service] insert data for user register <" + id + "> is failed!");
        QString res = "regis##no##";
        socket->write(res.toUtf8().data());
        return false;
    }
}
bool Server::UserCreateGroup(QString buffer,QTcpSocket*socket){//4
    QString gid = buffer.section("##",1,1);
    QString name = buffer.section("##",2,2);
    QString uid = buffer.section("##",3,3);
    if(db->insertComponent(USERGROUP_DATABASE_TABLE,gid,uid,name)){
        if(db->createUserGroup(USERGROUP_DATABASE_TABLE,gid) && db->addComponent(USERGROUP_DATABASE_TABLE,gid,uid))
            ui->online->append("[# UserCreateGroup] for user <" + uid +"> is successfully!");
        ui->listWidget_2->addItem(new QListWidgetItem(QIcon(":/group.png"),"group id " + gid + "(" + name + ")\nAdmin " + uid));
        int k = ui->listWidget_2->count();
        ui->listWidget_2->item(k-1)->setTextColor(QColor(139,137,137));
        userlist[gid.toUtf8().toInt()] = k-1;
        socket->write(("crp##ok##" + gid + "##" + name + "##").toUtf8().data());
        return true;
    }
    else{
        ui->online->append("[# UserCreateGroup] for user <" + uid + "> is failed!");
        socket->write("crp##no##");
        return false;
    }
}
void Server::UserInicial(QString buffer, QTcpSocket *socket){//2
    QString userId = QString(buffer).section("##",1,1);
    QString data = db->allData(USERINFO_DATABASE_TABLE,userId.toUtf8().toInt());//获取相关的信息
    data = "login##ok##" + data;//回复客户端
    socket->write(data.toUtf8().data());
    ui->online->append("[# user login] <" + userId + "> login successfully!");
    int k = userlist[userId.toUtf8().toInt()];
    ui->listWidget->item(k)->setIcon(QIcon(":/on.png"));
    ui->listWidget->item(k)->setTextColor(QColor(255,200,150));

}
bool Server::SearchGroup(QString buffer, QTcpSocket *socket){//3
    QString gid = buffer.section("##",1,1);
    QString uid = buffer.section("##",2,2);
    QString res = db->getGroupData(USERGROUP_DATABASE_TABLE,gid.toUtf8().toInt());
    if(res!=""){
        QString name = res.section("##",2,2);//群聊名称
        QString content;
        bool flag = db->is_InGroup(gid,uid);
        if(flag){//在群聊中
            content = "searchg##ok##" + gid + "##" + name + "##";
            ui->online->append("[# SearchGroup] <" + uid + "> is in " + gid);
        }
        else {//不在群聊中
            content = "searchg##okk##" + gid + "##" + name + "##";
            ui->online->append("[# SearchGroup] <" + uid + "> is not in " + gid);
        }
        socket->write(content.toUtf8().data());
        return true;
    }
    else{
        socket->write("searchg##fail##");
        return false;
    }
}
bool Server::addComponent(QString buffer){
    QString gid = buffer.section("##",1,1);
    QString uid = buffer.section("##",2,2);
    int number = buffer.section("##",3,3).toUtf8().toInt();
    if(number<=0){
        return false;
    }
    QString s = "";
    QString s1 = "";
    for(int k=0;k<number;k++){
        s = s + buffer.section("##",4+k,4+k) + "##" + ui->listWidget->item(userlist[buffer.section("##",4+k,4+k).toUtf8().toInt()])->text().section("\n",1,1).section(" ",2,2) + "##";
        s1 = s1 + s1 + buffer.section("##",4+k,4+k) + "$" + ui->listWidget->item(userlist[buffer.section("##",4+k,4+k).toUtf8().toInt()])->text().section("\n",1,1).section(" ",2,2) + "$";
    }
    QString res = QString::number(number) + "##" + gid + "##" + s;//新加入用户的数据
    QString res1 = QString::number(number) + "$" + gid + "$" + s1;
    //对原有成员发送新的数据包
    QString oldComponent = db->getAllGroupInfo(gid.toUtf8().toInt());
    QStringList oldList = oldComponent.split("##");
    for(auto it:oldList){
        if(it == ""){
            break;
        }
        if(db->checkState(it.toUtf8().toInt(),USERINFO_DATABASE_TABLE)){//在线
            udpSend(it.toUtf8().toInt(),"addg##"+res);
        }else{//离线
            QString time = currentTime();
            QString name = db->getGroupData(USERGROUP_DATABASE_TABLE,gid.toUtf8().toInt()).section("##",2,2);
            int idRev = it.toUtf8().toInt();
            int idSend = uid.toUtf8().toInt();
            saveLocalCache(NEW_COMPONENT_ADD_FOR_OLD_OFFLINE,idRev,idSend,name,res1,time);
            //123
        }
    }
    for(int k=0;k<number;k++){
        QString uid = buffer.section("##",4+k,4+k);
        db->addComponent(USERGROUP_DATABASE_TABLE,gid,uid);
    }
    //获取全部的群聊成员
    QString newComponent = db->getAllGroupInfo(gid.toUtf8().toInt());
    QStringList newList = newComponent.split("##");
    QString allUsersinfo = "";
    QString allUsersinfo1 = "";
    for(auto it:newList){
        if(it == ""){
            break;
        }
        allUsersinfo += it + "##" + ui->listWidget->item(userlist[it.toUtf8().toInt()])->text().section("\n",1,1).section(" ",2,2) + "##";
        allUsersinfo1 += it + "@" + ui->listWidget->item(userlist[it.toUtf8().toInt()])->text().section("\n",1,1).section(" ",2,2) + "@";
    }
    QString groupName = db->getGroupData(USERGROUP_DATABASE_TABLE,gid.toUtf8().toInt()).section("##",2,2);
    int groupNumber = db->getGroupNumber(USERGROUP_DATABASE_TABLE,gid);
    allUsersinfo = QString::number(groupNumber) + "##" + gid + "##" + groupName +"##" + allUsersinfo;
    for(int k = 0; k < number; k++){
        QString id = buffer.section("##",4+k,4+k);
        if(db->checkState(id.toUtf8().toInt(),USERINFO_DATABASE_TABLE)){
            udpSend(id.toUtf8().toInt(),"adda##" + allUsersinfo);
        }else{
            QString time = currentTime();
            int idRev = id.toUtf8().toInt();
            int idSend = uid.toUtf8().toInt();
            QString content = QString::number(groupNumber) + "@" + gid + "@" + allUsersinfo1;
            saveLocalCache(NEW_COMPONENT_ADD_FOR_NEW,idRev,idSend,groupName,content,time);
        }
    }
    return true;
}

/******************** Regular expression matching keywords ********************/
void Server::successfully(){
    QRegExp search_text = QRegExp("successfully|connected|ok|over|online");
    QTextDocument *document = ui->data->document();//全部数据
    bool found = false;
    QTextCursor highlight_cursor(document);
    QTextCursor cursor(document);
    //开始
    cursor.beginEditBlock();
    QTextCharFormat color_format(highlight_cursor.charFormat());
    QTextCharFormat color_format1(highlight_cursor.charFormat());
    color_format.setForeground(QColor(255,127,0));
    while (!highlight_cursor.isNull() && !highlight_cursor.atEnd()) {
        //查找指定的文本，匹配整个单词
        highlight_cursor = document->find(search_text, highlight_cursor, QTextDocument::FindWholeWords);
        if (!highlight_cursor.isNull())
        {
            if(!found)
            {
                found = true;
            }
            highlight_cursor.mergeCharFormat(color_format);
        }
    }
    cursor.endEditBlock();
    //结束
    QTextCharFormat fmt;//文本字符格式
    fmt.setForeground(QColor(232,232,232));// 前景色(即字体色)设为color色
    cursor = ui->data->textCursor();//获取文本光标
    cursor.mergeCharFormat(fmt);//光标后的文字就用该格式显示
    ui->data->mergeCurrentCharFormat(fmt);//textEdit使用当前的字符格式
    return;
}
void Server::successfully_1(){
    QRegExp search_text = QRegExp("successfully|connected|ok|over|online");
    QTextDocument *document = ui->online->document();//全部数据
    bool found = false;
    QTextCursor highlight_cursor(document);
    QTextCursor cursor(document);
    //开始
    cursor.beginEditBlock();
    QTextCharFormat color_format(highlight_cursor.charFormat());
    QTextCharFormat color_format1(highlight_cursor.charFormat());
    color_format.setForeground(QColor(255,127,0));
    while (!highlight_cursor.isNull() && !highlight_cursor.atEnd()) {
        //查找指定的文本，匹配整个单词
        highlight_cursor = document->find(search_text, highlight_cursor, QTextDocument::FindWholeWords);
        if (!highlight_cursor.isNull())
        {
            if(!found)
            {
                found = true;
            }
            highlight_cursor.mergeCharFormat(color_format);
        }
    }
    cursor.endEditBlock();
    //结束
    QTextCharFormat fmt;//文本字符格式
    fmt.setForeground(QColor(232,232,232));// 前景色(即字体色)设为color色
    cursor = ui->data->textCursor();//获取文本光标
    cursor.mergeCharFormat(fmt);//光标后的文字就用该格式显示
    ui->data->mergeCurrentCharFormat(fmt);//textEdit使用当前的字符格式
    return;
}
void Server::Brackets(){
    QRegExp search_text = QRegExp("\\[(.*)\\]");
    QTextDocument *document = ui->data->document();//全部数据
    bool found = false;
    QTextCursor highlight_cursor(document);
    QTextCursor cursor(document);
    //开始
    cursor.beginEditBlock();
    QTextCharFormat color_format(highlight_cursor.charFormat());
    QTextCharFormat color_format1(highlight_cursor.charFormat());
    color_format.setForeground(QColor(0,120,192));
    while (!highlight_cursor.isNull() && !highlight_cursor.atEnd()) {
        //查找指定的文本，匹配整个单词
        highlight_cursor = document->find(search_text, highlight_cursor, QTextDocument::FindWholeWords);
        if (!highlight_cursor.isNull())
        {
            if(!found)
            {
                found = true;
            }
            highlight_cursor.mergeCharFormat(color_format);
        }
    }
    cursor.endEditBlock();
    //结束
    QTextCharFormat fmt;//文本字符格式
    fmt.setForeground(QColor(232,232,232));// 前景色(即字体色)设为color色
    cursor = ui->data->textCursor();//获取文本光标
    cursor.mergeCharFormat(fmt);//光标后的文字就用该格式显示
    ui->data->mergeCurrentCharFormat(fmt);//textEdit使用当前的字符格式
    return;
}
void Server::Brackets_1(){
    QRegExp search_text = QRegExp("\\[(.*)\\]");
    QTextDocument *document = ui->online->document();//全部数据
    bool found = false;
    QTextCursor highlight_cursor(document);
    QTextCursor cursor(document);
    //开始
    cursor.beginEditBlock();
    QTextCharFormat color_format(highlight_cursor.charFormat());
    QTextCharFormat color_format1(highlight_cursor.charFormat());
    color_format.setForeground(QColor(0,205,102));
    while (!highlight_cursor.isNull() && !highlight_cursor.atEnd()) {
        //查找指定的文本，匹配整个单词
        highlight_cursor = document->find(search_text, highlight_cursor, QTextDocument::FindWholeWords);
        if (!highlight_cursor.isNull())
        {
            if(!found)
            {
                found = true;
            }
            highlight_cursor.mergeCharFormat(color_format);
        }
    }
    cursor.endEditBlock();
    //结束
    QTextCharFormat fmt;//文本字符格式
    fmt.setForeground(QColor(232,232,232));// 前景色(即字体色)设为color色
    cursor = ui->data->textCursor();//获取文本光标
    cursor.mergeCharFormat(fmt);//光标后的文字就用该格式显示
    ui->data->mergeCurrentCharFormat(fmt);//textEdit使用当前的字符格式
    return;
}
void Server::USERID(){
    QRegExp search_text = QRegExp("\\<(.*)\\>");
    QTextDocument *document = ui->data->document();//全部数据
    bool found = false;
    QTextCursor highlight_cursor(document);
    QTextCursor cursor(document);
    //开始
    cursor.beginEditBlock();
    QTextCharFormat color_format(highlight_cursor.charFormat());
    QTextCharFormat color_format1(highlight_cursor.charFormat());
    color_format.setForeground(QColor(0,150,12));
    while (!highlight_cursor.isNull() && !highlight_cursor.atEnd()) {
        //查找指定的文本，匹配整个单词
        highlight_cursor = document->find(search_text, highlight_cursor, QTextDocument::FindWholeWords);
        if (!highlight_cursor.isNull())
        {
            if(!found)
            {
                found = true;
            }
            highlight_cursor.mergeCharFormat(color_format);
        }
    }
    cursor.endEditBlock();
    //结束
    QTextCharFormat fmt;//文本字符格式
    fmt.setForeground(QColor(232,232,232));// 前景色(即字体色)设为color色
    cursor = ui->data->textCursor();//获取文本光标
    cursor.mergeCharFormat(fmt);//光标后的文字就用该格式显示
    ui->data->mergeCurrentCharFormat(fmt);//textEdit使用当前的字符格式
    return;
}
void Server::USERID_1(){
    QRegExp search_text = QRegExp("\\<(.*)\\>");
    QTextDocument *document = ui->online->document();//全部数据
    bool found = false;
    QTextCursor highlight_cursor(document);
    QTextCursor cursor(document);
    //开始
    cursor.beginEditBlock();
    QTextCharFormat color_format(highlight_cursor.charFormat());
    QTextCharFormat color_format1(highlight_cursor.charFormat());
    color_format.setForeground(QColor(0,150,12));
    while (!highlight_cursor.isNull() && !highlight_cursor.atEnd()) {
        //查找指定的文本，匹配整个单词
        highlight_cursor = document->find(search_text, highlight_cursor, QTextDocument::FindWholeWords);
        if (!highlight_cursor.isNull())
        {
            if(!found)
            {
                found = true;
            }
            highlight_cursor.mergeCharFormat(color_format);
        }
    }
    cursor.endEditBlock();
    //结束
    if (found == false) {}
    QTextCharFormat fmt;//文本字符格式
    fmt.setForeground(QColor(232,232,232));// 前景色(即字体色)设为color色
    cursor = ui->data->textCursor();//获取文本光标
    cursor.mergeCharFormat(fmt);//光标后的文字就用该格式显示
    ui->data->mergeCurrentCharFormat(fmt);//textEdit使用当前的字符格式
    return;
}
void Server::Fail(){
    QRegExp search_text = QRegExp("failed|fail|disconnected|error|incorrect|not|already|offline|released|Unable|no");
    QTextDocument *document = ui->data->document();//全部数据
    bool found = false;
    QTextCursor highlight_cursor(document);
    QTextCursor cursor(document);
    //开始
    cursor.beginEditBlock();
    QTextCharFormat color_format(highlight_cursor.charFormat());
    QTextCharFormat color_format1(highlight_cursor.charFormat());
    color_format.setForeground(QColor(178,34, 34));
    while (!highlight_cursor.isNull() && !highlight_cursor.atEnd()) {
        //查找指定的文本，匹配整个单词
        highlight_cursor = document->find(search_text, highlight_cursor, QTextDocument::FindWholeWords);
        if (!highlight_cursor.isNull())
        {
            if(!found)
            {
                found = true;
            }
            highlight_cursor.mergeCharFormat(color_format);
        }
    }
    cursor.endEditBlock();
    //结束
    QTextCharFormat fmt;//文本字符格式
    fmt.setForeground(QColor(232,232,232));// 前景色(即字体色)设为color色
    cursor = ui->data->textCursor();//获取文本光标
    cursor.mergeCharFormat(fmt);//光标后的文字就用该格式显示
    ui->data->mergeCurrentCharFormat(fmt);//textEdit使用当前的字符格式
    return;
}
void Server::Fail_1(){
    QRegExp search_text = QRegExp("failed|fail|disconnected|error|incorrect|not|already|offline|released|Unable|no");
    QTextDocument *document = ui->online->document();//全部数据
    bool found = false;
    QTextCursor highlight_cursor(document);
    QTextCursor cursor(document);
    //开始
    cursor.beginEditBlock();
    QTextCharFormat color_format(highlight_cursor.charFormat());
    QTextCharFormat color_format1(highlight_cursor.charFormat());
    color_format.setForeground(QColor(178,34, 34));
    while (!highlight_cursor.isNull() && !highlight_cursor.atEnd()) {
        //查找指定的文本，匹配整个单词
        highlight_cursor = document->find(search_text, highlight_cursor, QTextDocument::FindWholeWords);
        if (!highlight_cursor.isNull())
        {
            if(!found)
            {
                found = true;
            }
            highlight_cursor.mergeCharFormat(color_format);
        }
    }
    cursor.endEditBlock();
    //结束
    QTextCharFormat fmt;//文本字符格式
    fmt.setForeground(QColor(232,232,232));// 前景色(即字体色)设为color色
    cursor = ui->data->textCursor();//获取文本光标
    cursor.mergeCharFormat(fmt);//光标后的文字就用该格式显示
    ui->data->mergeCurrentCharFormat(fmt);//textEdit使用当前的字符格式
    return;
}
void Server::inicialUserTabel(){
    int number = db->getUserNumber(USERINFO_DATABASE_TABLE);
    QString data = db->getAllUserInfo(USERINFO_DATABASE_TABLE);
    ui->listWidget->clear();
    for(unsigned int i = 0;i < 3*number;i = i + 3){
        QListWidgetItem*item = new QListWidgetItem(QIcon(":/off.png"),"user id " + data.section("##",i,i)+"\nuser name "+data.section("##",i+1,i+1) + " ");
        ui->listWidget->addItem(item);
        ui->listWidget->item(i/3)->setTextColor(QColor(139,137,137));
        userlist[data.section("##",i,i).toUtf8().toInt()] = i/3;
    }
}
void Server::inicialUserGroup(){
    int number = db->getGroupNumber(USERGROUP_DATABASE_TABLE);
    QString data = db->getAllGroupInfo(USERGROUP_DATABASE_TABLE);
    ui->listWidget_2->clear();
    for(unsigned int i = 0;i < number*3;i = i + 3){
        ui->listWidget_2->addItem(new QListWidgetItem(QIcon(":/group.png"),"group id "+data.section("##",i,i) + "(" + data.section("##",i+2,i+2)+")\nAdmin "+data.section("##",i+1,i+1)));
        ui->listWidget_2->item(i/3)->setTextColor(QColor(139,137,137));
        grouplist[data.section("##",i,i).toUtf8().toInt()] = i/3;
    }
}

/******************** get DISK information ********************/
void Server::DISK(){
    QList<QStorageInfo> list = QStorageInfo::mountedVolumes();
    int i = 0;
    for(QStorageInfo& si : list)
    {
        QProgressBar *bar = new QProgressBar(this);
        double max = (si.bytesTotal()*1.0)/GB;
        bar->setRange(0,max);
        double value = (si.bytesFree()*1.0)/GB;
        bar->setValue(max-value);
        bar->setFixedSize(300,30);
        bar->setStyleSheet("QProgressBar{background:white;border:3px solid gray;border-radius:12px;text-align:center;} QProgressBar::chunk {background-color: #05B8CC; padding:1px 4px;text-align:center;}");
        if(i<3)
            bar->move(670,580+i*50);
        else if(i>=3 && i<6)
            bar->move(670+330,580+(i-3)*50);
        i++;
        double ratio = 100-(si.bytesFree()*1.0)/si.bytesTotal()*100;
        bar->setFormat(si.name()+" [剩余"+ QString::number(value,'g',4)+" GB] UR("+QString::number(ratio,'g',4)+"%)");
        bar->show();
    }
}
