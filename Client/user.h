#ifndef USER_H
#define USER_H
#include<QString>

class User
{
public:
    User();
    User(QString name,QString password,int id,QString ip,int port,int friendNumber = 0,bool state=false);
    User(QString name,QString password,int id,int friendNumber = 0,bool state=false);
    void setName(QString name);
    void setPassword(QString password);
    void setState(bool state);
    void setId(int id);
    void setFriendNumber(int number);
    QString getName();
    QString getPassowrd();
    int getId();
    bool getState();
    int getPort();
    QString getIp();
    bool setIpPort(QString name,QString ip,int port);
    int getFriendNumber();
    void addFriendNumber();
    void subFriendNumber();

    ~User();
private:
    QString name;
    QString password;
    int id;
    bool state;
    int port;
    QString ip;
    int friendNumber;
};

#endif // USER_H
