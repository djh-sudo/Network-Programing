#include "register.h"
#include "ui_register.h"

Register::Register(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Register)
{
    ui->setupUi(this);
    ui->name->setStyleSheet("border:3px solid gray;border-radius:12px;padding:1px 4px");
    ui->password1->setStyleSheet("border:3px solid gray;border-radius:12px;padding:1px 4px");
    ui->password2->setStyleSheet("border:3px solid gray;border-radius:12px;padding:1px 4px");

}

Register::~Register()
{
    delete ui;
}
