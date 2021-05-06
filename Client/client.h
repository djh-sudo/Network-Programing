#ifndef CLIENT_H
#define CLIENT_H

#include <QMainWindow>
#include<QTcpSocket>
#include<QString>
#include<QTimer>
#include<QCloseEvent>
#include<userpanel.h>
#include<register.h>
QT_BEGIN_NAMESPACE
namespace Ui { class Client; }
QT_END_NAMESPACE
class Client : public QMainWindow
{
    Q_OBJECT

public:
    Client(QWidget *parent = nullptr);
    ~Client();

private slots:

    void on_connectServer_clicked();
    void on_login_clicked();
    void on_register_2_clicked();
    void showClt();
    void handle(QString s);

signals:
    void quit();
    void regShow();
    void panShow(QString);
    void sendData(QString s);

private:
    Ui::Client *ui;
    QTcpSocket *socket;
    QTimer* time;
    void closeEvent(QCloseEvent*event);
};
#endif // CLIENT_H
