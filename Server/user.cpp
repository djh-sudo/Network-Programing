#include "user.h"
#include<QString>
User::User(){}
User::User(int id,QString ip,quint16 port,quint16 udpPort){
    this->id = id;
    this->ip = ip;
    this->port = port;
    this->udpPort = udpPort;
}
User::User(int id){
    this->id = id;
    this->ip = "127.0.0.1";
    this->port = 0;
    this->udpPort = 0;
}
User::User(int id,QString ip,quint16 port){
    this->id = id;
    this->ip = ip;
    this->port = port;
    this->udpPort = 0;
}

QString User::getIp(){
    return this->ip;
}
quint16 User::getPort(){
    return this->port;
}
int User::getId(){
    return this->id;
}
quint16 User::getUdpPort(){
    return this->udpPort;
}
User::~User(){
}
