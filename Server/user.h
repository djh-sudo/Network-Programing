#ifndef USER_H
#define USER_H
#include<QString>

class User
{
public:
    User();
    User(int id,QString ip,quint16 port,quint16 udpPort);
    User(int id,QString ip,quint16 port);
    User(int id);
    quint16 getPort();
    int getId();
    QString getIp();
    quint16 getUdpPort();
    void setUdpPort(quint16 port);
    ~User();
private:
    int id;
    quint16 port;
    QString ip;
    quint16 udpPort;
};

#endif // USER_H
