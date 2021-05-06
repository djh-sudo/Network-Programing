#include "modify.h"
#include "ui_modify.h"

Modify::Modify(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Modify)
{
    ui->setupUi(this);
}

Modify::~Modify()
{
    delete ui;
}
