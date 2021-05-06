#include "userdata.h"
#include"cachefile.h"

UserData::UserData(){}

UserData::UserData(int id,QString name,QString time,QString content){
    this->name = name;
    this->time = time;
    this->content = content;
    this->id = id;
}

QString UserData::getName(){
    return this->name;
}

QString UserData::getTime(){
    return this->time;
}

QString UserData::getContent(){
    return this->content;
}
void UserData::setName(QString name){
    this->name = name;
}
void UserData::setTime(QString time){
    this->time = time;
}
void UserData::setContent(QString content){
    this->content = content;
}
int UserData::getId(){
    return this->id;
}
