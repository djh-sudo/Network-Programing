#include "mydatabase.h"
#include "ui_mydatabase.h"

MyDataBase::MyDataBase(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MyDataBase)
{
    ui->setupUi(this);
}

MyDataBase::~MyDataBase()
{
    delete ui;
}
