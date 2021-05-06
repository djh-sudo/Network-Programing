#ifndef REGISTER_H
#define REGISTER_H

#include <QMainWindow>
#include "client.h"
#include<QCloseEvent>
namespace Ui {
class Register;
}
class Register : public QMainWindow
{
    Q_OBJECT

public:
    explicit Register(QWidget *parent = nullptr);
    void closeEvent(QCloseEvent *event);
    ~Register();

private slots:
    void on_name_textChanged(const QString &arg1);
    void on_register_2_clicked();
    void on_password1_textChanged(const QString &arg1);
    void on_password2_textChanged(const QString &arg1);
    void on_toolButton_pressed();
    void on_toolButton_released();
    void on_pushButton_7_clicked();
    void showReg();
    void on_connectServer_clicked();
    void on_pushButton_9_clicked();
    QString produceId();
    void on_copy_clicked();
    void Close();

signals: void quit();
    void showClient();
private:
    Ui::Register *ui;
    QTcpSocket *socket;
    QTimer* time;
};

#endif // REGISTER_H
