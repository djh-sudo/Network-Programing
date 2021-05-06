#include "userdata.h"
#include"cachefile.h"

UserData::UserData(){}

UserData::UserData(QString name,QString time,QString content){
    this->name = name;
    this->time = time;
    this->content = content;
}

QString UserData::getName() const{
    return this->name;
}

QString UserData::getTime() const{
    return this->time;
}

QString UserData::getContent() const{
    return this->content;
}

