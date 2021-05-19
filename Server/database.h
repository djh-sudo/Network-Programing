#ifndef DATABASE_H
#define DATABASE_H

#include<QSqlDatabase>
#include<QSqlError>
#include<QMutex>
#include<QSqlQuery>
#include<QString>
#include<QMainWindow>
#include<QWidget>
class DataBase:public QMainWindow
{
    Q_OBJECT

public:
    explicit DataBase(QString sqlName,QString tabelName,QWidget *parent = nullptr);
    explicit DataBase(QWidget *parent = nullptr);
    bool createTable(QString name);
    bool createTable(int id);
    bool inicialTable();
    bool insertData(QString userTableName,int ID,QString userName,QString password,int friendNumber);
    bool insertData(QString userTabelName,QString friendTabelName,int id,int friendId);//+好友
    bool deleteData(QString userTabelName,QString friendTabelName,int id,int friendId);//-好友
    int checkUser(QString userTableName,int id,QString password);
    bool checkState(int userID,QString userTableName);
    QString allData(QString userTableName,int id);
    bool setPassword(QString userTableName,int id,QString password);
    bool setState(QString userTableName,int id);//在线
    bool resetState(QString userTableName,int id);//离线
    bool addFriend(QString userTableName,int id);
    bool deleteFriend(QString userTableName,int id);
    QString allFriendData(QString friendTableName);
    bool deleteFriendData(QString friendTableName,int id);
    bool resetUser(QString userTableName,int id);
    bool is_Friend(QString id1,QString id2);
    bool is_InGroup(QString gid,QString userId);
    int getUserNumber(QString userTableName);
    QString getAllUserInfo(QString userTabel);
    bool createGroup(QString groupTableName);
    bool addComponent(QString userGroupTable,QString gid,QString uid);
    int getGroupNumber(QString groupTableName,QString gid);
    bool insertComponent(QString groupTableName,QString gid,QString uid,QString name);
    bool createUserGroup(QString groupTableName,QString groupId);
    bool deleteComponent(QString groupTableName,QString gid,QString uid);
    QString getGroupData(QString groupTableName,int groupId);
    bool setComponent(QString groupTableName,int groupId);//设置群聊人数
    QString getAllGroupInfo(QString groupTableName);
    int getGroupNumber(QString groupName);
    bool deleteGroup(QString groupTableName);
    bool deleteUserGroup(QString groupTableName,QString groupId);
    QString getGroupAdmin(QString groupTableName,int groupId);
    QString getAllGroupInfo(int groupId);
    ~DataBase();

signals:
    void Send(QString s);

private:
    QString sqlName;
    QString tableName;
    QSqlDatabase db;
    QSqlQuery query;

};

#endif // DATABASE_H
