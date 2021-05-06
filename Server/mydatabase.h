#ifndef MYDATABASE_H
#define MYDATABASE_H

#include <QWidget>

namespace Ui {
class MyDataBase;
}

class MyDataBase : public QWidget
{
    Q_OBJECT

public:
    explicit MyDataBase(QWidget *parent = nullptr);
    ~MyDataBase();

private:
    Ui::MyDataBase *ui;
};

#endif // MYDATABASE_H
