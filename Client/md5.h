#ifndef MD5_H
#define MD5_H

#include<QString>
#include<QCryptographicHash>
class MD5
{
public:
    MD5();
    static QString Md5(QString pwd){
        QString md5;
        QByteArray ba,bb;
        QCryptographicHash md(QCryptographicHash::Md5);
        ba.append(pwd);
        md.addData(ba);
        bb = md.result();
        return md5.append(bb.toHex());
    }
};

#endif // MD5_H
