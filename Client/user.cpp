#include "user.h"
#include<QString>
User::User(){
    this->id = 0;
    this->name = "NULL";
    this->state = false;
    this->password = "NULL";
}
User::User(QString name,QString password,int id,QString ip,int port,int friendNumber,bool state){
    this->id = id;
    this->name = name;
    this->password = password;
    this->state = state;
    this->ip = ip;
    this->port = port;
    this->friendNumber = friendNumber;
}
User::User(QString name,QString password,int id,int friendNumber,bool state){
    this->id = id;
    this->name = name;
    this->password = password;
    this->state = state;
    this->ip = "127.0.0.1";
    this->port = 0;
    this->friendNumber = 0;
}
int User::getId(){
    return this->id;
}
QString User::getName(){
    return this->name;
}
QString User::getPassowrd(){
    return password;
}
bool User::getState(){
    return this->state;
}
void User::setName(QString name){
    this->name = name;
}
void User::setState(bool state){
    this->state = state;
}
void User::setPassword(QString password){
    this->password = password;
}
bool User::setIpPort(QString name, QString ip, int port){
    if(this->name == name){
        this->ip = ip;
        this->port = port;
        return true;
    }
    else
        return false;
}
QString User::getIp(){
    return this->ip;
}
int User::getPort(){
    return this->port;
}
void User::setId(int id){
    this->id = id;
}
User::~User(){

}
int User::getFriendNumber(){
    return this->friendNumber;
}
void User::addFriendNumber(){
    this->friendNumber = this->friendNumber + 1;
}
void User::subFriendNumber(){
    if(this->friendNumber > 0){
        this->friendNumber = this->friendNumber - 1;
    }
}
void User::setFriendNumber(int number){
    this->friendNumber = number;
}
