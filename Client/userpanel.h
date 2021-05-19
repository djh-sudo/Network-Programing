#ifndef USERPANEL_H
#define USERPANEL_H

#include <QMainWindow>
#include<QListWidgetItem>
#include<QTimer>
#include<QString>
#include"cachefile.h"
#include"userdata.h"
#include<QCloseEvent>
#include<QUdpSocket>

/*********************Macro defination *********************/
#define INICIAL_LOGIN "ini"
#define MESSAGE "msg"
#define AGREEMENT_TO_BE_FRIEND "agr"
#define OTHER_MESSAGE "oth"
#define ADD_IN_GROUP_FIRSTTIME "adda"
#define NEW_COMPONENT_ADD_IN_GROUP "addg"
#define GROUP_MESSAGE "grp"
#define ACCEPT_TO_BE_FRIEND "uagr"
#define SEARCH_GROUP "searchg"
#define SEARCH_USER_ID "search"
#define NEW_COMPONENT_ADD_INTO_GROUP "addaf"
#define OLD_COMPONENT_ADD_INTO_GROUP "addgf"
#define DELETE_USER "del"
#define CREATE_GROUP "crp"
#define FAIL "fail"
#define DELETE_GROUP "delg"
#define ADMIN_DELETE_GROUP "dela"
#define SYSTEM_FILE "system"
namespace Ui {
class UserPanel;
}

class UserPanel : public QMainWindow
{
    Q_OBJECT

public:
    explicit UserPanel(QWidget *parent = nullptr);
    ~UserPanel();
    Ui::UserPanel *ui;
    void readfromCache(QString id);//把文件数据读入到内存
    void writetoCache(QString id);//把文件数据写入磁盘
    void freshUserTable(int);//刷新左侧列表
    void flashCache(QString id);//刷新Cache[破坏型的写]
    void deleteCache(QString id);//删除文件
    QString produceGroupid(QString id);
    bool eventFilter(QObject *target, QEvent *event);//事件过滤器,对键盘部分按键进行处理
/********************用户行为处理函数 ********************/
    void Exit();
    bool UserSearch(QString s);
    void agreement(QString s);
    void CreateGroup(QString s);
    void offlineMessage(QString s);
    bool GroupSearch(QString s);
    QString Encode(QString s);
    QString Decode(QString r);
/********************信号槽函数********************/
private slots:
    void showPan(QString ip);//显示面板槽函数
    void on_lineEdit_returnPressed();//回车键槽函数
    void handle(QString s);//事件处理函数
    void on_tableWidget_cellClicked(int row, int column);//消息框点击函数
    void on_userlist_itemClicked(QListWidgetItem *item);
    void on_userlist_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);
    void on_send_clicked();
    QString currentTime();
    void on_empty_clicked();
    void on_addGroup_clicked();
    void on_add_clicked();
    void on_component_clicked();
    void on_deleteFriend_clicked();
    void on_deletegroup_clicked();


signals:
    void quit();//quit()信号
    void sendData(QString s);//发送消息信号
private:
    void closeEvent(QCloseEvent*event);
    QUdpSocket *udpSocket;
};

#endif // USERPANEL_H
