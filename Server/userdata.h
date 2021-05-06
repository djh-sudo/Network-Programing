#ifndef USERDATA_H
#define USERDATA_H

#include<QString>

class UserData
{
public:
    UserData();
    UserData(QString name,QString time,QString content);
    QString  getName() const;
    QString  getTime() const;
    QString  getContent() const;

private:
    QString name;
    QString time;
    QString content;
};

#endif // USERDATA_H
