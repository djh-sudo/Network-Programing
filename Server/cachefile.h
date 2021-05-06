#ifndef CACHEFILE_H
#define CACHEFILE_H
#include<QString>
#include<QFile>
class CacheFile
{
public:
    CacheFile(QString path="temp");
    bool deleteFile(QString name);
    bool writeFile(QString name,QString data);
    QStringList readFile(QString name);
    QString getPath();
private:
    QString name;//文件名
    QString path;//缓存路径
};

#endif // CACHEFILE_H
