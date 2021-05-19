#include "cachefile.h"
#include<QDir>
#include<QString>
#include<QStringList>
#include<QDebug>
CacheFile::CacheFile(QString path){
    QDir dir;
    if(!dir.exists("./Cache/" + path)){
        dir.mkpath("./Cache/" + path);
    }
    this->path = "./Cache/" + path;
}
bool CacheFile::writeFile(QString name, QString data){
    QFile file(this->path+"/"+name);
    qDebug()<<"file path is "<<this->path+"/"+name;
    if(!file.open(QIODevice::Append)){
        qDebug()<<"文件打开失败[write]";
        return false;
    }
    else{
        file.write(data.toUtf8());
        file.close();
        return true;
    }
}
QStringList CacheFile::readFile(QString name){
    QStringList res;
    res.clear();
    QFile file(this->path+"/"+name);
    if(!file.open(QIODevice::ReadOnly)){
        qDebug()<<"文件打开失败[read]";
        return res;
    }
    else{
        while(!file.atEnd()){
            QByteArray line =file.readLine();
            QString temp = line;
            temp = temp.trimmed();
            res.push_back(temp);
        }
        file.close();
    }
    return res;
}
bool CacheFile::deleteFile(QString name){
    QFile::remove(this->path+"/"+name);
    return true;
}
QString CacheFile::getPath(){
    return this->path;
}
