#ifndef USERDATA_H
#define USERDATA_H

#include<QString>

class UserData
{
public:
    UserData();
    UserData(int id,QString name,QString time,QString content);
    QString getContent();
    QString getName();
    QString getTime();
    int getId();
    void setName(QString name);
    void setTime(QString time);
    void setContent(QString content);

private:
    int id;
    QString name;
    QString time;
    QString content;
};

#endif // USERDATA_H
