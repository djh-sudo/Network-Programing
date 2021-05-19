#include "userpanel.h"
#include "ui_userpanel.h"
#include <user.h>
#include <QIcon>
#include <QMessageBox>
#include <QDebug>
#include <client.h>
#include "readonlydelegate.h"
#include <QVector>
#include <QStringList>
#include "userdata.h"
#include <QDateTime>
#include <QStringList>
#include <QSet>
#include <QMessageBox>
#include <md5.h>
#include <QInputDialog>
#include <QMenu>
#include <QSplitter>
#include <QList>
#include <QDir>
#include <toast.h>
#include <QtAlgorithms>
/*********************Global var *********************/
User user;
QVector<UserData>message;//消息队列
QSet<QString>set;
QMap<int,int>userlist;//通过id号获得在表单中的位置
QSet<int>groupID;
static bool flag = false;
extern QString __IP__;
static bool flagGroup = false;

/********************* Construction function *********************/
UserPanel::UserPanel(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::UserPanel)
{

    ui->setupUi(this);
    //inicial the random key
    srand(time(NULL));

    message.clear();
    userlist.clear();
    set.clear();
    groupID.clear();

    set.insert("ini");
    set.insert("msg");
    set.insert("agr");
    set.insert("oth");
    set.insert("addg");
    set.insert("adda");
    //set the style
    QIcon head;
    head.addFile(tr(":/QQ.png"));
    ui->state->setIcon(head);
    ui->head->setIcon(head);
    ui->lineEdit->setStyleSheet("border:1px solid gray;border-radius:8px;padding:1px 4px");
    ui->tableWidget->setIconSize(QSize(50,50));
    ui->userlist->setIconSize(QSize(50,50));
    ui->tableWidget->verticalHeader()->setDefaultSectionSize(60);//行高
    ui->tableWidget->setColumnCount(3);
    ui->tableWidget->setColumnWidth(0,60);
    ui->tableWidget->setColumnWidth(1,490);
    ui->tableWidget->setColumnWidth(2,60);

    ui->tableWidget->horizontalHeader()->setVisible(false);
    ui->tableWidget->verticalHeader()->setVisible(false);
    // read only
    ReadOnlyDelegate* readOnlyDelegate = new ReadOnlyDelegate();
    ui->tableWidget->setItemDelegateForColumn(2, readOnlyDelegate); //设置某列只读
    ui->tableWidget->setItemDelegateForColumn(0, readOnlyDelegate); //设置某列只读

    ui->tableWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->tableWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    //do not show the grid
    ui->tableWidget->setShowGrid(false);
    //hide the button
    ui->textEdit->hide();
    ui->empty->hide();
    ui->add->setHidden(true);
    ui->deletegroup->setHidden(true);
    ui->component->setHidden(true);
    // install the eventfilter function
    ui->textEdit->installEventFilter(this);//
}

/********************* Construction function *********************/
void UserPanel::showPan(QString ip){
    this->show();
    //inicial the GUI
    ui->id->setText(QString::number(user.getId()));
    ui->name->setText(user.getName());
    ui->userlist->setCurrentRow(0);
    ui->userlist->addItem(new QListWidgetItem(QIcon(":/QQ1.png"),"系统消息"));
    if(userlist[0] == -1){
        ui->userlist->item(0)->setBackground(QColor(255,155,78));
    }
    //refresh the userlist
    freshUserTable(0);
    //bind the udp port
    udpSocket = new QUdpSocket();
    udpSocket->bind(QHostAddress(ip),(user.getPort()+1)%65535);
    //make a udp read signal
    connect(udpSocket,&QUdpSocket::readyRead,this,[=](){
        QByteArray array;
        array.resize(udpSocket->bytesAvailable());
        quint16 port;
        QHostAddress address;
        udpSocket->readDatagram(array.data(),array.size(),&address,&port);
        //实时显示
        QString res = array.data();
        message.clear();
        //=================================收到对方通过在线发送的消息===================
        if(res.section("##",0,0) == OTHER_MESSAGE){
            QString id = res.section("##",1,1);
            QString name = res.section("##",2,2);
            QString time = res.section("##",3,3);
            QString content = res.section("##",4,4);
            //处理消息
            ui->userlist->item(userlist[id.toUtf8().toInt()])->setBackground(QColor(255,155,78));
            //渲染消息
            message.clear();
            message.push_back(UserData(id.toUtf8().toInt(),name,time,content));
            writetoCache(id);
            QString text = ui->userlist->currentItem()->text();
            if(ui->userlist->currentItem()!=NULL && text.section("\n",1,1) == id){//左侧列表选中这一行才可以显示
                int number = ui->tableWidget->rowCount();//获取当前的行数
                ui->tableWidget->insertRow(number);
                ui->tableWidget->setItem(number,0,new QTableWidgetItem(QIcon(":/s5.png"),""));
                ui->tableWidget->setItem(number,1,new QTableWidgetItem(name + " " + time+"\n"+ Decode(content)));
                ui->tableWidget->item(number,1)->setTextAlignment(Qt::AlignLeft);//右对齐
            }
        }
        //======================收到对方在线添加好友的消息==============================
        else if(res.section("##",0,0) == AGREEMENT_TO_BE_FRIEND){//
            ui->userlist->item(0)->setBackground(QColor(255,155,78));
            QString idSend = res.section("##",1,1);//id
            QString name = res.section("##",2,2);//name
            QString addTime = res.section("##",3,3);
            QString content = res.section("##",4,4);
            //渲染左侧视图，同时写入文件保存
            message.clear();
            message.push_back(UserData(idSend.toUtf8().toInt(),name,currentTime(),content));
            if(ui->userlist->currentItem()!=NULL && ui->userlist->currentRow() == 0){
                int number = ui->tableWidget->rowCount();//获取当前的行数
                ui->tableWidget->insertRow(number);
                ui->tableWidget->setItem(number,0,new QTableWidgetItem("同意"));
                ui->tableWidget->setItem(number,1,new QTableWidgetItem((message[number].getTime()+"\n" + message[number].getContent())));
                ui->tableWidget->setItem(number,2,new QTableWidgetItem("拒绝"));
                ui->tableWidget->item(number,0)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
                ui->tableWidget->item(number,1)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
                ui->tableWidget->item(number,2)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
                ui->tableWidget->repaint();
            }
            writetoCache(SYSTEM_FILE);
        }
        //==================在线收到对方同意以后反馈的数据===============================
        else if(res.section("##",0,0) == ACCEPT_TO_BE_FRIEND){
            ui->userlist->item(0)->setBackground(QColor(255,155,78));
            QString id = res.section("##",1,1);
            QString name = res.section("##",2,2);
            QString time = res.section("##",3,3);
            message.clear();
            message.push_back(UserData(id.toUtf8().toInt(),name,time,""));
            writetoCache(QString::number(user.getId()));
            message.push_back(UserData(id.toUtf8().toInt(),name,time,"你们已经成为好友,打个招呼吧!"));
            writetoCache(id);
            freshUserTable(1);//刷新左侧用户列表
        }
        //===============================在线收到加入群聊的消息=========================
        else if(res.section("##",0,0) == NEW_COMPONENT_ADD_IN_GROUP){
            int number = res.section("##",1,1).toUtf8().toInt();
            QString gid = res.section("##",2,2);
            ui->userlist->item(userlist[gid.toUtf8().toInt()])->setBackground(QColor(255,155,78));
            message.clear();
            for(int k=0;k<number*2;k+=2){
                QString id = res.section("##",3+k,3+k);
                QString name = res.section("##",k+4,k+4);
                QString time = currentTime();
                message.push_back(UserData(id.toUtf8().toInt(),name,time,"大家好,我是"+name));
            }
            writetoCache(gid);
            for(int k=0;k<number*2;k+=2){
                QString id = res.section("##",3+k,3+k);
                QString name = res.section("##",k+4,k+4);
                QString time = currentTime();
                message.push_back(UserData(id.toUtf8().toInt(),name,time,""));
            }
            writetoCache(MD5::Md5(gid));
        }
        else if(res.section("##",0,0) == ADD_IN_GROUP_FIRSTTIME){//初次渲染
            int number = res.section("##",1,1).toUtf8().toInt();
            QString gid = res.section("##",2,2);
            QString gname = res.section("##",3,3);
            QString time = currentTime();
            userlist[gid.toUtf8().toInt()] = -1;
            groupID.insert(gid.toUtf8().toInt());
            message.clear();
            message.push_back(UserData(gid.toUtf8().toInt(),gname,time,"g"));
            writetoCache(QString::number(user.getId()));
            for(int k=0;k<number*2;k+=2){
                QString id = res.section("##",4+k,4+k);
                QString name = res.section("##",k+5,k+5);
                QString time = currentTime();
                QString text = ui->userlist->currentItem()->text();
                if(ui->userlist->currentItem()!=NULL && text.section("\n",1,1) == gid){//左侧列表选中这一行才可以显示
                    ui->tableWidget->insertRow(number);
                    ui->tableWidget->setItem(number,0,new QTableWidgetItem(QIcon(":/s5.png"),""));
                    ui->tableWidget->setItem(number,1,new QTableWidgetItem(name + " " + time + "\n大家好,我是" + name));
                    ui->tableWidget->item(number,1)->setTextAlignment(Qt::AlignLeft);//右对齐
                }
                message.push_back(UserData(id.toUtf8().toInt(),name,time,"大家好,我是"+name));

            }
            writetoCache(gid);
            for(int k=0;k<number*2;k+=2){
                QString id = res.section("##",4+k,4+k);
                QString name = res.section("##",k+5,k+5);
                QString time = currentTime();
                message.push_back(UserData(id.toUtf8().toInt(),name,time,""));
            }
            writetoCache(MD5::Md5(gid));
            freshUserTable(1);
        }
        else if(res.section("##",0,0) == GROUP_MESSAGE){
            QString idSend = res.section("##",1,1);
            QString nameSend = res.section("##",2,2);
            QString gid = res.section("##",3,3);
            QString time = res.section("##",4,4);
            QString content = res.section("##",5,5);
            ui->userlist->item(userlist[gid.toUtf8().toInt()])->setBackground(QColor(255,155,78));
            message.clear();
            message.push_back(UserData(gid.toUtf8().toInt(),nameSend,time,content));
            writetoCache(gid);
            QString text = ui->userlist->currentItem()->text();
            if(ui->userlist->currentItem()!=NULL && text.section("\n",1,1) == gid){//左侧列表选中这一行才可以显示
                int number = ui->tableWidget->rowCount();//获取当前的行数
                ui->tableWidget->insertRow(number);
                ui->tableWidget->setItem(number,0,new QTableWidgetItem(QIcon(":/s5.png"),""));
                ui->tableWidget->setItem(number,1,new QTableWidgetItem(nameSend + " " + time+"\n" +Decode(content)));
                ui->tableWidget->item(number,1)->setTextAlignment(Qt::AlignLeft);//右对齐
            }
        }
        else if(res.section("##",0,0) == DELETE_USER){
            int id = res.section("##",1,1).toUtf8().toInt();
            deleteCache(QString::number(id));
            readfromCache(QString::number(user.getId()));
            int loc = userlist[id];
            message.erase(message.begin()+loc-1);
            flashCache(QString::number(user.getId()));
            ui->userlist->clear();
            ui->userlist->setCurrentRow(0);
            ui->userlist->addItem(new QListWidgetItem(QIcon(":/QQ1.png"),"系统消息"));
            freshUserTable(0);
            Toast::instance().show(Toast::INFO,QString::number(id) + "与你解除好友关系");
        }
        else if(res.section("##",0,0) == DELETE_GROUP){
            int gid = res.section("##",1,1).section("#",0,0).toUtf8().toInt();
            int uid = res.section("##",1,1).section("#",1,1).toUtf8().toInt();
            readfromCache(MD5::Md5(QString::number(gid)));
            int count = 0;
            for(auto it:message){
                if(it.getId() == uid){
                    message.erase(message.begin()+count);
                }
                count++;
            }
            flashCache(MD5::Md5(QString::number(gid)));
            Toast::instance().show(Toast::INFO, QString::number(uid) + " 已退出群聊("+QString::number(gid)+")!");
        }
        else if(res.section("##",0,0) == ADMIN_DELETE_GROUP){
            int gid = res.section("##",1,1).toUtf8().toInt();
            deleteCache(QString::number(gid));
            deleteCache(MD5::Md5(QString::number(gid)));
            readfromCache(QString::number(user.getId()));
            int loc = userlist[gid];
            message.erase(message.begin()+loc-1);
            flashCache(QString::number(user.getId()));
            ui->userlist->clear();
            ui->userlist->setCurrentRow(0);
            ui->userlist->addItem(new QListWidgetItem(QIcon(":/QQ1.png"),"系统消息"));
            freshUserTable(0);
            Toast::instance().show(Toast::INFO, "管理员已解散群聊("+QString::number(gid)+")!");
        }
    });
}

/******************** disconstruction function ********************/
UserPanel::~UserPanel()
{
    message.clear();
    userlist.clear();
    set.clear();
    groupID.clear();
    if(udpSocket!=NULL){
        udpSocket->disconnectFromHost();
        delete udpSocket;
    }
    delete ui;
}

/******************** search information ********************/
void UserPanel::on_lineEdit_returnPressed()//回车搜索
{
    QString data = ui->lineEdit->text();
    emit sendData("search##" + data + "##" + QString::number(user.getId()) + "##");
    emit sendData("searchg##"+ data + "##" + QString::number(user.getId()) + "##");
}

/******************** receive info from server ********************/
void UserPanel::handle(QString s){
    ui->tableWidget->clear();
    ui->tableWidget->setRowCount(0);
    QIcon right;
    right.addFile(tr(":/right.png"));
    ui->state->setIcon(right);
    ui->state2->setText("在线");
    QStringList content = s.split("##");
    for(unsigned int i = 0;i < content.size();){
        if(content[i] == "exit"){
            Exit();
            i = i + 1;
        }
        else if(content[i] == SEARCH_USER_ID){
            QString s;
            if(content[i+1] != FAIL)
                s = "##" + content[i+1] + "##" + content[i+2] + "##" + content[i+3];
            else{
                s = "##" + content[i+1];
            }
            if(UserSearch(s))
                i = i + 4;
            else i = i + 2;
            continue;
        }
        else if(content[i] == AGREEMENT_TO_BE_FRIEND){
            QString s = "##" + content[i+1] + "##" + content[i+2];
            agreement(s);
            i = i + 3;
            continue;
        }
        else if(content[i] == CREATE_GROUP){
            QString s = "##" + content[i+1] + "##" + content[i+2] + "##" + content[i+3];
            CreateGroup(s);
            i = i + 4;
        }else if(content[i] == SEARCH_GROUP){
            QString s;
            if(content[i+1] != FAIL)
                s = "##" + content[i+1] + "##" + content[i+2] + "##" + content[i+3];
            else s = "##" + content[i+1];
            if(GroupSearch(s))
                i = i + 4;
            else i = i + 2;
        }
        else if(content[i] == ""){
            i = i + 1;
        }
        else{
            offlineMessage(s);
            return;
        }
    }
    return;
}

//=====================点击信息框显示信息========================
void UserPanel::on_tableWidget_cellClicked(int row, int column)
{
    if(!ui->tableWidget->item(row,column))
        return;
    if(row >= 0 && column >=0){
        if(ui->tableWidget->item(row,column)->text() == "添加"){
            ui->tableWidget->item(row,1)->setText("消息已经发送,请等待对方同意.");
            ui->tableWidget->item(row,0)->setText("");
            ui->tableWidget->item(row,2)->setText("");
            emit sendData("add##" + ui->lineEdit->text()+"##" +QString::number(user.getId())+"##");
        }
        else if(ui->tableWidget->item(row,column)->text() == "同意"){
            readfromCache(SYSTEM_FILE);
            ui->lineEdit->clear();
            ui->lineEdit->setText(QString::number(message[row].getId()));//message getid存储的是自己的id
            QString s = "agr##" + ui->lineEdit->text()+"##" + QString::number(user.getId()) + "##";
            //删除消息
            message.erase(message.begin()+row);
            if(message.size()==0){
                deleteCache(SYSTEM_FILE);
            }
            else
                flashCache(SYSTEM_FILE);
            message.clear();
            emit sendData(s);//消息传递给服务器，等待服务器确认
        }
        else if(ui->tableWidget->item(row,column)->text() == "拒绝"){
            ui->tableWidget->item(row,column)->setText("已拒绝");
            ui->tableWidget->item(row,column-2)->setText("");
            message.erase(message.begin()+row);
            if(message.size()==0){
                deleteCache(SYSTEM_FILE);
            }
            else
                flashCache(SYSTEM_FILE);
        }
    }
}

//=================从文件中读入信息到message中========================
void UserPanel::readfromCache(QString id){//从文件id读入cache
    message.clear();
    CacheFile userFile = CacheFile(QString::number(user.getId()));
    QStringList res = userFile.readFile(id);
    for(int i=0;i<res.size();i++){
        QString ID = res[i].section("##",0,0);//id
        QString name = res[i].section("##",1,1);//name
        QString time = res[i].section("##",2,2);//time
        QString content = res[i].section("##",3,3);//content
        if(ID!="")
            message.push_back(UserData(ID.toUtf8().toInt(),name,time,content));
    }
}

//=======================将message信息写入到id文件中========================
void UserPanel::writetoCache(QString id){
    CacheFile userFile = CacheFile(QString::number(user.getId()));
    QString res = "";
    for(int i=0;i<message.size();i++){
        QString ID = QString::number(message[i].getId());
        QString name = message[i].getName();
        QString time = message[i].getTime();
        QString content = message[i].getContent();
        res += ID + "##" + name + "##" + time + "##" + content + "##\n";
    }
    userFile.writeFile(id,res);
    message.clear();
}

//=======================刷新Cache========================================
void UserPanel::flashCache(QString id){
    CacheFile userFile = CacheFile(QString::number(user.getId()));
    QString res = "";
    for(int i=0;i<message.size();i++){
        QString ID = QString::number(message[i].getId());
        QString name = message[i].getName();
        QString time = message[i].getTime();
        QString content = message[i].getContent();
        res += ID + "##" + name + "##" + time + "##" + content + "##\n";
    }
    userFile.flashFile(id,res);
    message.clear();
}

//========================左侧用户栏点击事件================================
void UserPanel::on_userlist_itemClicked(QListWidgetItem *item)
{
    ui->tableWidget->clear();
    ui->tableWidget->setRowCount(0);
    ui->tableWidget->setAlternatingRowColors(false);
    ui->tableWidget->setIconSize(QSize(50,50));
    ui->userlist->setIconSize(QSize(50,50));
    ui->tableWidget->verticalHeader()->setDefaultSectionSize(60);//行高
    ui->tableWidget->setColumnCount(3);
    ui->tableWidget->setColumnWidth(0,60);
    ui->tableWidget->setColumnWidth(1,490);
    ui->tableWidget->setColumnWidth(2,60);
    if(flag && ui->add->isEnabled() == false){
        ui->userlist->item(0)->setSelected(false);
        ui->userlist->item(0)->setBackground(QColor(200,200,200));
        for(QSet<int>::iterator it = groupID.begin();it != groupID.end();it++){
            ui->userlist->item(userlist[*it])->setSelected(false);
            ui->userlist->item(userlist[*it])->setBackground(QColor(200,200,200));
        }
        return;
    }else if(flag && ui->deleteFriend->isEnabled() == false){
        ui->userlist->item(0)->setSelected(false);
        ui->userlist->item(0)->setBackground(QColor(200,200,200));
        for(QSet<int>::iterator it = groupID.begin();it != groupID.end();it++){
            ui->userlist->item(userlist[*it])->setSelected(false);
            ui->userlist->item(userlist[*it])->setBackground(QColor(200,200,200));
        }
        readfromCache(MD5::Md5(ui->label->text()));
        for(auto it:message){
            ui->userlist->item(userlist[it.getId()])->setSelected(false);
            ui->userlist->item(userlist[it.getId()])->setBackground(QColor(200,200,200));
        }
        return;
    }
    else if(flagGroup){
        for(int i=0;i<ui->userlist->count();i++){
            int id = ui->userlist->item(i)->text().section("\n",1,1).toUtf8().toInt();
            if(!groupID.contains(id)){
                ui->userlist->item(i)->setSelected(false);
                ui->userlist->item(i)->setBackground(QColor(200,200,200));
            }
        }
        return;
    }
    item->setTextColor(Qt::black);
    item->setBackground(QColor(255,255,255));
    ui->tableWidget->clear();
    ui->tableWidget->setRowCount(0);
    QString text = item->text();
    if(item != NULL && text == "系统消息"){
        ui->label->setText("系统消息");
        ui->component->hide();
        ui->deletegroup->hide();
        ui->add->hide();
        ui->label_2->clear();
        ui->textEdit->hide();
        ui->empty->show();
        readfromCache(SYSTEM_FILE);
        ui->tableWidget->setRowCount(message.size());
        for(int i = 0;i<message.size();i++){
            ui->tableWidget->setItem(i,0,new QTableWidgetItem("同意"));
            ui->tableWidget->setItem(i,1,new QTableWidgetItem((message[i].getTime() + "\n"+message[i].getContent())));
            ui->tableWidget->setItem(i,2,new QTableWidgetItem("拒绝"));
            ui->tableWidget->item(i,0)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
            ui->tableWidget->item(i,1)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
            ui->tableWidget->item(i,2)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
        }
        return;
    }
    else{
        ui->textEdit->show();
        ui->empty->show();
        QString id = text.section("\n",1,1);
        QString name = text.section("\n",0,0);
        ui->label->setText(id);
        ui->label_2->setText(name);
        //将其内容渲染值界面
        message.clear();
        readfromCache(id);
        ui->tableWidget->clear();
        ui->tableWidget->setRowCount(message.size());
        ui->tableWidget->setAlternatingRowColors(true);
        if(groupID.contains(id.toUtf8().toInt()) || flag){
            ui->add->show();
            ui->component->show();
            ui->deletegroup->show();
        }
        else{
            ui->add->setHidden(true);
            ui->component->setHidden(true);
            ui->deletegroup->setHidden(true);
        }
        for(int i=0;i<message.size();i++){
            //根据消息的id进行渲染区分
            if(user.getId() == message[i].getId()){//自己发送的消息
                ui->tableWidget->setItem(i,0,new QTableWidgetItem(""));
                ui->tableWidget->setItem(i,1,new QTableWidgetItem((message[i].getTime() + " "+ user.getName() + "\n" + Decode(message[i].getContent()))));
                ui->tableWidget->setItem(i,2,new QTableWidgetItem(QIcon(":/s7.png"),""));
                ui->tableWidget->item(i,0)->setBackgroundColor(QColor(255,250,205));
                ui->tableWidget->item(i,1)->setBackgroundColor(QColor(255,250,205));
                ui->tableWidget->item(i,2)->setBackgroundColor(QColor(255,250,205));
                ui->tableWidget->item(i,1)->setTextAlignment(Qt::AlignRight);
                ui->tableWidget->item(i,2)->setTextAlignment(Qt::AlignHCenter);
                //设置行高
            }
            else{
                ui->tableWidget->setItem(i,0,new QTableWidgetItem(QIcon(":/s5.png"),""));
                ui->tableWidget->setItem(i,1,new QTableWidgetItem((message[i].getName() + " " + message[i].getTime() + "\n" + Decode(message[i].getContent()))));
                ui->tableWidget->item(i,0)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
                ui->tableWidget->item(i,1)->setTextAlignment(Qt::AlignLeft);
                //设置行高
            }
        }
    }
    return;
}

//====================用户切换=============================================
void UserPanel::on_userlist_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    if(current == NULL){
        return;
    }
    else if(previous){
        QString ptext = previous->text();
        QString ctext = current->text();
        if(ctext == ptext)
            return;
        else{
            ui->textEdit->clear();
        }
    }
}

/******************** close window ********************/
void UserPanel::closeEvent(QCloseEvent *event){
    emit quit();
}

/******************** event filter (virtual function) ********************/
bool UserPanel::eventFilter(QObject *target, QEvent *event){
    if(target == ui->textEdit){
        if(event->type() == QEvent::KeyPress){
            QKeyEvent *k = static_cast<QKeyEvent *>(event);
            if(k->key() == Qt::Key_Return||k->key() == Qt::Key_Enter){
                on_send_clicked();//发送消息
                return true;
            }
        }
    }
    return QWidget::eventFilter(target,event);//其余按键可以通过
}

/******************** fresh the user tabel ********************/
void UserPanel::freshUserTable(int num){//刷新左侧用户栏
    message.clear();
    readfromCache(QString::number(user.getId()));//存储了friend信息
    if(num == 0){
        for(int i=0;i<message.size();i++){
            if(message[i].getContent() == ""){
                ui->userlist->addItem(new QListWidgetItem(QIcon(":/s2.png"),message[i].getName()+"\n" + QString::number(message[i].getId())));
            }else{
                ui->userlist->addItem(new QListWidgetItem(QIcon(":/group.png"),message[i].getName()+"\n" + QString::number(message[i].getId())));
                groupID.insert(message[i].getId());
            }
            if(userlist.value(message[i].getId()) == -1){
                ui->userlist->item(i+1)->setBackground(QColor(255,155,78));
            }
            userlist[message[i].getId()] = i + 1;
        }
    }
    else{//动态刷新列表
        if(message[message.size()-1].getContent() == ""){
            ui->userlist->addItem(new QListWidgetItem(QIcon(":/s2.png"),message[message.size()-1].getName()+"\n" + QString::number(message[message.size()-1].getId())));
        }
        else{
            ui->userlist->addItem(new QListWidgetItem(QIcon(":/group.png"),message[message.size()-1].getName()+"\n" + QString::number(message[message.size()-1].getId())));
            groupID.insert(message[message.size()-1].getId());
        }
        ui->userlist->item(message.size())->setBackground(QColor(255,155,78));
        userlist[message[message.size()-1].getId()] = message.size();
    }
    ui->userlist->repaint();//渲染
    message.clear();
}

/********************send message ********************/
void UserPanel::on_send_clicked(){//发送消息给对方
    QString content = ui->textEdit->toPlainText();
    content = Encode(content);
    if(content.length()>65536){
        Toast::instance().show(Toast::ERROR,"消息过长,请分批发送!");
        return;
    }
    if(content == ""){
        Toast::instance().show(Toast::WARN,"发送消息不可以为空");
        return;
    }
    else if(!groupID.contains(ui->label->text().toUtf8().toInt())){
        QString time = currentTime();
        QString id = QString::number(user.getId());
        QString name = user.getName();
        message.clear();
        message.push_back(UserData(user.getId(),name,time,content));
        int number = ui->tableWidget->rowCount();//获取当前的行数
        ui->tableWidget->insertRow(number);
        ui->tableWidget->setItem(number,0,new QTableWidgetItem(""));
        ui->tableWidget->setItem(number,2,new QTableWidgetItem(QIcon(":/s5.png"),""));
        ui->tableWidget->setItem(number,1,new QTableWidgetItem(time+" "+name + "\n" + Decode(content)));
        ui->tableWidget->item(number,1)->setTextAlignment(Qt::AlignRight);//右对齐
        ui->tableWidget->item(number,0)->setBackgroundColor(QColor(255,250,205));
        ui->tableWidget->item(number,1)->setBackgroundColor(QColor(255,250,205));
        ui->tableWidget->item(number,2)->setBackgroundColor(QColor(255,250,205));
        ui->tableWidget->scrollToBottom();
        QString friendId = ui->label->text();
        writetoCache(friendId);
        ui->textEdit->clear();
        QString res = "oth##" + id + "##" + ui->label->text() + "##" + user.getName() + "##" + time + "##" + content + "##";
        udpSocket->writeDatagram(res.toUtf8().data(),QHostAddress(__IP__),9999);
    }else{
        QString time = currentTime();
        QString id = QString::number(user.getId());
        QString name = user.getName();
        message.clear();
        message.push_back(UserData(user.getId(),name,time,content));
        QString gid = ui->label->text();
        writetoCache(gid);
        int number = ui->tableWidget->rowCount();//获取当前的行数
        ui->tableWidget->insertRow(number);
        ui->tableWidget->setItem(number,0,new QTableWidgetItem(""));
        ui->tableWidget->setItem(number,2,new QTableWidgetItem(QIcon(":/s5.png"),""));
        ui->tableWidget->setItem(number,1,new QTableWidgetItem(time+" "+name + "\n" + Decode(content)));
        ui->tableWidget->item(number,1)->setTextAlignment(Qt::AlignRight);//右对齐
        ui->tableWidget->item(number,0)->setBackgroundColor(QColor(255,250,205));
        ui->tableWidget->item(number,1)->setBackgroundColor(QColor(255,250,205));
        ui->tableWidget->item(number,2)->setBackgroundColor(QColor(255,250,205));
        ui->tableWidget->scrollToBottom();
        ui->textEdit->clear();
        QString res = "grp##" + id + "##" + ui->label->text() + "##" + ui->label_2->text() + "##" + time + "##" + content + "##";
        udpSocket->writeDatagram(res.toUtf8().data(),QHostAddress(__IP__),9999);
    }
}

/******************** get current time ********************/
QString UserPanel::currentTime(){
    QDateTime datatime = QDateTime::currentDateTime();
    QString time = datatime.toString("yyyy-MM-dd hh:mm:ss");
    return time;
}

/******************** delete local cache file ********************/
void UserPanel::deleteCache(QString id){
    CacheFile userFile = CacheFile(QString::number(user.getId()));
    userFile.deleteFile(id);
    message.clear();
}

/******************** empty local cache file ********************/
void UserPanel::on_empty_clicked()
{
    if(QMessageBox::Yes == QMessageBox::question(this,"userInfo","\t\t是否清除信息记录?",QMessageBox::Yes,QMessageBox::No)){
        QString id = ui->label->text();
        if(id!="系统消息"){
            CacheFile file = CacheFile(QString::number(user.getId()));
            file.deleteFile(id);
            ui->tableWidget->clear();
            ui->tableWidget->setRowCount(0);
        }
        else if(id!=""){
            CacheFile file = CacheFile(QString::number(user.getId()));
            file.deleteFile(SYSTEM_FILE);
            ui->tableWidget->clear();
            ui->tableWidget->setRowCount(0);
        }
    }
    else return;
}

/******************** create a new group ********************/
void UserPanel::on_addGroup_clicked()
{
    QString gid = produceGroupid(ui->id->text());
    bool bOk = false;
    QString sName = QInputDialog::getText(this, "创建群聊","\t请输入群聊名称",QLineEdit::Normal,"",&bOk);
    if(bOk){
        if(!sName.isEmpty()){
            if(!sName.contains("#") && !sName.contains("@") && !sName.contains("$")){
                emit sendData("crp##" + gid + "##" + sName + "##" + QString::number(user.getId()) + "##");
                return;
            }
            else{
                Toast::instance().show(Toast::ERROR,"群聊名称包含非法字符!");
                return;
            }
        }else{
            Toast::instance().show(Toast::WARN,"群聊名称为空");
            return;
        }
    }
    else{
        Toast::instance().show(Toast::INFO,"已取消创建群聊");
    }
}

/******************** produce a group is using hash ********************/
QString UserPanel::produceGroupid(QString id){
    QDateTime time = QDateTime::currentDateTime();
    QString s = time.toString("yyyy-MM-dd hh:mm:ss");
    s = MD5::Md5(s+id);
    s = "0x" + s.left(6);
    int value = s.toInt(NULL,16);
    return QString::number(value);
}

/******************** add new group component ********************/
void UserPanel::on_add_clicked(){
    flag = ! flag;
    if(flag){
        ui->deleteFriend->setEnabled(false);
        Toast::instance().show(Toast::INFO,"请在左侧选择添加的群聊成员");
        ui->userlist->currentItem()->setSelected(false);
        ui->userlist->setSelectionMode(QAbstractItemView::MultiSelection);
        ui->add->setIcon(QIcon(":/waiting.png"));
        ui->userlist->item(0)->setSelected(false);
        ui->userlist->item(0)->setBackground(QColor(200,200,200));
        for(QSet<int>::iterator it = groupID.begin();it != groupID.end();it++){
            ui->userlist->item(userlist[*it])->setSelected(false);
            ui->userlist->item(userlist[*it])->setBackground(QColor(200,200,200));
        }
        readfromCache(MD5::Md5(ui->label->text()));
        for(auto it:message){
            ui->userlist->item(userlist[it.getId()])->setSelected(false);
            ui->userlist->item(userlist[it.getId()])->setBackground(QColor(200,200,200));
        }
        return;
    }
    else{
        ui->deleteFriend->setEnabled(true);
        ui->userlist->item(0)->setSelected(false);
        ui->userlist->item(0)->setBackground(QColor(255,255,255));
        for(QSet<int>::iterator it = groupID.begin();it!=groupID.end();it++){
            ui->userlist->item(userlist[*it])->setSelected(false);
            ui->userlist->item(userlist[*it])->setBackground(QColor(255,255,255));
        }
        QString gid = ui->label->text();
        ui->userlist->setSelectionMode(QAbstractItemView::SingleSelection);
        ui->add->setIcon(QIcon(":/add.png"));
        QList<QListWidgetItem*> items = ui->userlist->selectedItems();
        int number = items.count();
        if(number == 0){
            return;
        }
        QString res = "";
        QString member = "";
        for(int i = 0;i<number;i++){
            int row = ui->userlist->row(items.at(i));
            res += ui->userlist->item(row)->text().section("\n",0,0)+"("+ui->userlist->item(row)->text().section("\n",1,1)+")\n";
            member += ui->userlist->item(row)->text().section("\n",1,1)+"##";
        }
        if(QMessageBox::Yes == QMessageBox::question(this,"userInfo","是否将以下成员加入群聊("+ gid +"):\n"+res,QMessageBox::Yes,QMessageBox::No)){
            ui->userlist->item(userlist[gid.toUtf8().toInt()])->setBackground(QColor(255,155,78));
            QString res = "addg##" + gid + "##" + QString::number(user.getId()) + "##" + QString::number(number) + "##" + member;
            emit sendData(res.toUtf8().data());
            Toast::instance().show(Toast::INFO,"添加完成!");
        }
        else{
            for(int i=0;i<ui->userlist->count();i++){
                if(ui->userlist->item(i)->backgroundColor() == QColor(200,200,200)){
                    ui->userlist->item(i)->setBackground(QColor(255,255,255));
                }
            }
            Toast::instance().show(Toast::INFO,"添加已取消!");
            return;
        }
    }
}

/******************** deal with user events ********************/
void UserPanel::Exit(){
    QIcon error;
    error.addFile(tr(":/error.png"));
    ui->state->setIcon(error);
    ui->state2->setText("离线");
    ui->add->setEnabled(false);
    ui->deletegroup->setEnabled(false);
    ui->deleteFriend->setEnabled(false);
}

/******************** search result ********************/
bool UserPanel::UserSearch(QString s){
    ui->tableWidget->setRowCount(1);
    if(s.section("##",1,1) == "ok"||s.section("##",1,1) == "okk"){
        QString id = s.section("##",2,2);
        QString name = s.section("##",3,3) ;
        ui->tableWidget->setItem(0,0,new QTableWidgetItem(QIcon(":/s1.png"),""));
        ui->tableWidget->setItem(0,1,new QTableWidgetItem("userId "+id+"\nusername "+name));
        //服务器还要判断二者是否为好友关系
        if(id != QString::number(user.getId()) && s.section("##",1,1) == "ok"){
            ui->tableWidget->setItem(0,2,new QTableWidgetItem("添加"));
        }
        else{
            ui->tableWidget->setItem(0,2,new QTableWidgetItem(""));
        }
        ui->tableWidget->item(0,1)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
        ui->tableWidget->item(0,2)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
        return true;
    }
    else {
        ui->tableWidget->setItem(0,0,new QTableWidgetItem(QIcon(":/s1.png"),""));
        ui->tableWidget->setItem(0,1,new QTableWidgetItem("未查询到用户!"));
        return false;
    }
}

/******************** search group ********************/
bool UserPanel::GroupSearch(QString s){
    if(s.section("##",1,1) == "ok" || s.section("##",1,1) == "okk"){
        ui->tableWidget->setRowCount(2);
        QString id = s.section("##",2,2);
        QString name = s.section("##",3,3);
        ui->tableWidget->setItem(1,0,new QTableWidgetItem(QIcon(":/group.png"),""));
        ui->tableWidget->setItem(1,1,new QTableWidgetItem("Group Id "+id+"\nGroup Name "+name));
        if(s.section("##",1,1) == "ok"){
            ui->tableWidget->setItem(1,2,new QTableWidgetItem("已加入"));
        }
        else{
            ui->tableWidget->setItem(1,2,new QTableWidgetItem("加入"));
        }
        ui->tableWidget->item(1,1)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
        ui->tableWidget->item(1,2)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
        return true;
    }
    else{
        ui->tableWidget->setRowCount(2);
        ui->tableWidget->setItem(1,0,new QTableWidgetItem(QIcon(":/group.png"),""));
        ui->tableWidget->setItem(1,1,new QTableWidgetItem("未查询到群聊!"));
        return false;
    }
}

/******************** agreement to be friend ********************/
void UserPanel::agreement(QString s){
    user.addFriendNumber();//好友数量+1;
    QString id = s.section("##",1,1);
    QString name = s.section("##",2,2);
    QString time = currentTime();
    message.clear();
    message.push_back(UserData(id.toUtf8().toInt(),name,time,"你们已经成为好友,打个招呼吧!"));
    flashCache(id);
    message.push_back(UserData(id.toUtf8().toInt(),name,time,""));
    writetoCache(QString::number(user.getId()));
    freshUserTable(1);//刷新左侧用户列表
}

/******************** create a new group ********************/
void UserPanel::CreateGroup(QString s){
    if(s.section("##",1,1) == "ok"){
        QString gid = s.section("##",2,2);
        QString name = s.section("##",3,3);
        QString time = currentTime();
        message.clear();
        message.push_back(UserData(gid.toUtf8().toInt(),name,time,"g"));
        writetoCache(QString::number(user.getId()));
        message.push_back(UserData(user.getId(),user.getName(),time,"大家好,我是" + user.getName()));
        writetoCache(gid);
        freshUserTable(1);
        groupID.insert(gid.toUtf8().toInt());
        message.clear();
        message.push_back(UserData(user.getId(),user.getName(),time,""));
        writetoCache(MD5::Md5(gid));
        return;
    }
    else{
        Toast::instance().show(Toast::ERROR,"群聊创建失败，请重试!");
        return;
    }
}

/******************** handle offline message ********************/
void UserPanel::offlineMessage(QString s){
    QStringList list = s.split("##");
    for(int i = 0;i<list.size();i++){
        if(list[i] == INICIAL_LOGIN){
            emit sendData("ini##" + QString::number(user.getId()));
            i = i + 1;
        }
        else if(list[i] == MESSAGE){
            QString id = list[i+1];
            QString name = list[i+2];
            int k = 3;
            message.clear();
            for(;!set.contains(list[i+k]) && s.section("##",i+k,i+k)!="";){
                QString time = s.section("##",i+k,i+k);
                QString content = s.section("##",i+k+1,i+k+1);
                k = k + 2;
                message.push_back(UserData(id.toUtf8().toInt(),name,time,content));
            }
            i = i + k - 1;
            writetoCache(SYSTEM_FILE);
            userlist[0] = -1;
            message.clear();
        }
        else if(list[i] == ACCEPT_TO_BE_FRIEND){
            QString id = list[i+1];
            QString name = list[i+2];
            QString time = currentTime();
            message.clear();
            message.push_back(UserData(id.toUtf8().toInt(),name,time,""));
            writetoCache(QString::number(user.getId()));
            message.clear();
            message.push_back(UserData(id.toUtf8().toInt(),name,time,"你们已经成为好友,打个招呼吧!"));
            writetoCache(id);
            userlist[id.toUtf8().toInt()] = -1;
            i = i + 3;
        }
        else if(list[i] == OTHER_MESSAGE){//接收到的tcp
            QString id = list[i+1];
            QString name = list[i+2];
            int k = 3;
            for(;!set.contains(list[i+k]) && s.section("##",i+k,i+k)!="";){
                QString time = s.section("##",i+k,i+k);
                QString content = s.section("##",i+k+1,i+k+1);
                k = k + 2;
                message.push_back(UserData(id.toUtf8().toInt(),name,time,content));
                userlist[id.toUtf8().toInt()] = -1;
            }
            i = i + k - 1;
            writetoCache(id);
            message.clear();
        }
        else if(list[i] == NEW_COMPONENT_ADD_INTO_GROUP){
            QString name = list[i+2];
            QString time = list[i+3];
            QString alldata = list[i+4];
            QStringList data = alldata.split("@");
            int number = data[0].toUtf8().toInt();
            QString gid = data[1];
            message.clear();
            message.push_back(UserData(gid.toUtf8().toInt(),name,time,"g"));
            writetoCache(QString::number(user.getId()));
            for(int k=0;k<number*2;k+=2){
                QString id = data[k+2];
                QString name = data[k+3];
                message.push_back(UserData(id.toUtf8().toInt(),name,time,""));
                if(id.toUtf8().toInt() == user.getId())
                    user.setName(name);
            }
            writetoCache(MD5::Md5(gid));
            for(int k=0;k<number*2;k+=2){
                QString id = data[k+2];
                QString name = data[k+3];
                message.push_back(UserData(id.toUtf8().toInt(),name,time,"大家好,我是" + name));
            }
            writetoCache(gid);
            groupID.insert(gid.toUtf8().toInt());
            userlist[gid.toUtf8().toInt()] = -1;
            i = i + 4;
        }
        else if(list[i] == OLD_COMPONENT_ADD_INTO_GROUP){//修改部分
            QString name = list[i+2];
            QString time = list[i+3];
            QString alldata = list[i+4];
            QStringList data = alldata.split("$");
            int number = data[0].toUtf8().toInt();
            QString gid = data[1];
            for(int k=0;k<number*2;k+=2){
                QString id = data[k+2];
                QString name = data[k+3];
                message.push_back(UserData(id.toUtf8().toInt(),name,time,""));
            }
            writetoCache(MD5::Md5(gid));
            for(int k=0;k<number*2;k+=2){
                QString id = data[k+2];
                QString name = data[k+3];
                message.push_back(UserData(id.toUtf8().toInt(),name,time,"大家好,我是" + name));
            }
            writetoCache(gid);
            userlist[gid.toUtf8().toInt()] = -1;
            i = i + 4;
        }
        else if(list[i] == GROUP_MESSAGE){
            int number = list[i+1].toUtf8().toInt();
            QString idSend = list[i+2];
            QString gid = list[i+3];
            QDir dir;
            if(dir.exists("./Cache/" + QString::number(user.getId()) + "/" + MD5::Md5(gid))){
                for(int k = 0;k<number*2;k+=2){
                    QString time = list[i+k+4];
                    QString content = list[i+k+5];
                    QStringList l = content.split("@");
                    QString name = l[0];
                    QString data = l[1];
                    message.push_back(UserData(idSend.toUtf8().toInt(),name,time,data));
                }
                userlist[gid.toUtf8().toInt()] = -1;
                writetoCache(gid);
            }
            i = i + 3 + 2*number;
        }
        else if(list[i] == DELETE_USER){
            int id = list[i+1].toUtf8().toInt();
            deleteCache(QString::number(id));
            readfromCache(QString::number(user.getId()));
            int count = 0;
            for(auto it:message){
                if(it.getId() == id){
                    message.erase(message.begin()+count);
                    flashCache(QString::number(user.getId()));
                    break;
                }
                count++;
            }
            i = i + 4;
        }
        else if(list[i] == DELETE_GROUP){//isDend 退出群聊
            QString idSend = list[i+1];
            QString time = list[i+3];
            QString gid = list[i+4];
            readfromCache(MD5::Md5(gid));
            int count = 0;
            for(auto it:message){
                if(it.getId() == idSend.toUtf8().toInt()){
                    message.erase(message.begin()+count);
                    flashCache(MD5::Md5(gid));
                    break;
                }
                count++;
            }
            i = i + 4;
        }
        else if(list[i] == ADMIN_DELETE_GROUP){//解散群聊
            QString idAdmin = list[i+1];
            QString gid = list[i+4];
            deleteCache(gid);
            deleteCache(MD5::Md5(gid));
            readfromCache(QString::number(user.getId()));
            int loc = userlist[gid.toUtf8().toInt()];
            message.erase(message.begin()+loc);
            flashCache(QString::number(user.getId()));
            QSet<int>::iterator it = groupID.find(gid.toUtf8().toInt());
            if(it!=groupID.end()){
                groupID.erase(it);
            }
            ui->tableWidget->clear();
            ui->userlist->clear();
            ui->userlist->setCurrentRow(0);
            ui->userlist->addItem(new QListWidgetItem(QIcon(":/QQ1.png"),"系统消息"));
            freshUserTable(0);
            Toast::instance().show(Toast::INFO,"群聊("+ gid +")已被管理员("+ idAdmin + ") 解散!");
            i = i + 4;
        }

    }
}

/******************** view group component ********************/
void UserPanel::on_component_clicked()
{
    ui->tableWidget->setAlternatingRowColors(true);
    QString gid = ui->label->text();
    ui->tableWidget->clear();
    ui->tableWidget->setRowCount(0);
    ui->tableWidget->setIconSize(QSize(50,50));
    ui->userlist->setIconSize(QSize(50,50));
    ui->tableWidget->verticalHeader()->setDefaultSectionSize(60);//行高
    ui->tableWidget->setColumnCount(3);
    ui->tableWidget->setColumnWidth(0,60);
    ui->tableWidget->setColumnWidth(1,300);
    ui->tableWidget->setColumnWidth(2,250);
    readfromCache(MD5::Md5(gid));
    ui->tableWidget->setRowCount(message.size());
    for(int i = 0;i<message.size();i++){
        int k = rand()%9+1;
        ui->tableWidget->setItem(i,0,new QTableWidgetItem(QIcon(":/s"+QString::number(k)+".png"),""));
        ui->tableWidget->setItem(i,1,new QTableWidgetItem("id "+ QString::number(message[i].getId()) +"\nname " + message[i].getName()));
        ui->tableWidget->setItem(i,2,new QTableWidgetItem("time " + message[i].getTime()));
        ui->tableWidget->item(i,1)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
    }
}

/******************** delete the user ********************/
void UserPanel::on_deleteFriend_clicked(){
    flag = !flag;
    if(flag){
        ui->add->setEnabled(false);
        ui->userlist->item(0)->setSelected(false);
        ui->userlist->item(0)->setBackground(QColor(200,200,200));
        ui->deleteFriend->setIcon(QIcon(":/waiting.png"));
        ui->userlist->setSelectionMode(QAbstractItemView::MultiSelection);
        Toast::instance().show(Toast::INFO,"请在左侧选择的删除的好友");
        ui->userlist->item(0)->setSelected(false);
        ui->userlist->item(0)->setBackground(QColor(200,200,200));
        for(QSet<int>::iterator it = groupID.begin();it != groupID.end();it++){
            ui->userlist->item(userlist[*it])->setSelected(false);
            ui->userlist->item(userlist[*it])->setBackground(QColor(200,200,200));
        }
        return;
    }
    else{
        ui->add->setEnabled(true);
        ui->userlist->item(0)->setSelected(false);
        ui->userlist->item(0)->setBackground(QColor(255,255,255));
        QString uid = ui->label->text();
        ui->userlist->setSelectionMode(QAbstractItemView::SingleSelection);
        ui->deleteFriend->setIcon(QIcon(":/error.png"));
        QList<QListWidgetItem*>items = ui->userlist->selectedItems();
        for(int i=0;i<ui->userlist->count();i++){
            if(ui->userlist->item(i)->backgroundColor() == QColor(200,200,200)){
                ui->userlist->item(i)->setBackground(QColor(255,255,255));
            }
        }

        int number = items.count();
        if(number ==0){
            return;
        }else{
            QString res = "";
            QString member = "";
            for(int i = 0;i<number;i++){
                int row = ui->userlist->row(items.at(i));
                res += ui->userlist->item(row)->text().section("\n",0,0)+"("+ui->userlist->item(row)->text().section("\n",1,1)+")\n";
                member += ui->userlist->item(row)->text().section("\n",1,1)+"##";
            }
            if(QMessageBox::Yes == QMessageBox::question(this,"userInfo","是否将以下好友删除:\n"+res,QMessageBox::Yes,QMessageBox::No)){
                QString res = "del##" + QString::number(user.getId()) + "##" + QString::number(number) + "##" + member;
                emit sendData(res.toUtf8().data());
                QVector<int>toBeDelete;
                toBeDelete.clear();
                for(int k=0;k<number;k++){
                    int id = member.section("##",k,k).toUtf8().toInt();
                    int loc = userlist[id];
                    toBeDelete.push_back(loc);
                    deleteCache(QString::number(id));
                }
                qSort(toBeDelete.begin(),toBeDelete.end(),std::greater<int>());
                readfromCache(QString::number(user.getId()));
                for(int k=0;k<number;k++){
                    int loc = toBeDelete[k];
                    message.erase(message.begin()+loc-1);
                }
                toBeDelete.clear();
                flashCache(QString::number(user.getId()));
                //刷新左侧用户列表
                ui->userlist->clear();
                ui->userlist->setCurrentRow(0);
                ui->userlist->addItem(new QListWidgetItem(QIcon(":/QQ1.png"),"系统消息"));
                freshUserTable(0);
                Toast::instance().show(Toast::INFO,"删除好友成功!");
            }
            else{
                Toast::instance().show(Toast::INFO,"删除已取消!");
                return;
            }
        }
    }
}

/******************** Ecode the user data ********************/
QString UserPanel::Encode(QString s){
    if(s.contains("#") || s.contains("=")){
        s.replace("=","==");
        s.replace("#","#=");
        return s;
    }else
        return s;
}

/******************** Decode the user data ********************/
QString UserPanel::Decode(QString s){
    if(s.contains("#=") || s.contains("==")){
        s.replace("#=","#");
        s.replace("==","=");
        return s;
    }else return s;
}

/******************** Delete user group ********************/
void UserPanel::on_deletegroup_clicked()
{
    flagGroup = !flagGroup;
    if(flagGroup){
        ui->addGroup->setEnabled(false);
        ui->add->setEnabled(false);
        ui->deleteFriend->setEnabled(false);
        ui->userlist->item(0)->setSelected(false);
        ui->userlist->item(0)->setBackground(QColor(255,225,255));
        ui->deletegroup->setIcon(QIcon(":/waiting.png"));
        ui->userlist->setSelectionMode(QAbstractItemView::MultiSelection);
        Toast::instance().show(Toast::INFO,"请在左侧选择退出的群聊");
        for(int i=0;i<ui->userlist->count();i++){
            int id = ui->userlist->item(i)->text().section("\n",1,1).toUtf8().toInt();
            if(!groupID.contains(id)){
                ui->userlist->item(i)->setSelected(false);
                ui->userlist->item(i)->setBackground(QColor(200,200,200));
            }
        }
        return;
    }
    else{
        ui->add->setEnabled(true);
        ui->deleteFriend->setEnabled(true);
        ui->addGroup->setEnabled(true);
        ui->userlist->item(0)->setBackground(QColor(255,255,255));
        ui->deletegroup->setIcon(QIcon(":/error.png"));
        ui->userlist->setSelectionMode(QAbstractItemView::SingleSelection);
        QList<QListWidgetItem*>items = ui->userlist->selectedItems();
        for(int i=0;i<ui->userlist->count();i++){
            ui->userlist->item(i)->setBackground(QColor(255,255,255));
        }
        int number = items.count();
        if(number == 0)
            return;
        else{
            QString res = "";
            QString member = "";
            for(int k=0;k<number;k++){
                int row = ui->userlist->row(items.at(k));
                res += ui->userlist->item(row)->text().section("\n",0,0) + "(" + ui->userlist->item(row)->text().section("\n",1,1)+")\n";
                member += ui->userlist->item(row)->text().section("\n",1,1) + "##";
            }
            if(QMessageBox::Yes ==  QMessageBox::question(this,"groupInfo","是否退出以下群聊:\n"+res+"\n**若您为群聊创建者,此群将会被解散!**",QMessageBox::Yes,QMessageBox::No)){
                QString res = "delg##" + QString::number(user.getId()) + "##" + QString::number(number) + "##" + member;
                emit sendData(res.toUtf8().data());
                QVector<int>toBeDelete;
                toBeDelete.clear();
                for(int k=0;k<number;k++){
                    QString gid = member.section("##",k,k);
                    deleteCache(gid);
                    deleteCache(MD5::Md5(gid));
                    int loc = userlist[gid.toUtf8().toInt()];
                    toBeDelete.push_back(loc);
                    QSet<int>::iterator it = groupID.find(gid.toUtf8().toInt());
                    if(it!=groupID.end()){
                        groupID.erase(it);
                    }
                }
                qSort(toBeDelete.begin(),toBeDelete.end(),std::greater<int>());
                readfromCache(QString::number(user.getId()));
                for(int k=0;k<number;k++){
                    int loc = toBeDelete[k];
                    message.erase(message.begin()+loc-1);
                }
                toBeDelete.clear();
                flashCache(QString::number(user.getId()));
                //刷新左侧用户列表
                ui->userlist->clear();
                ui->userlist->setCurrentRow(0);
                ui->userlist->addItem(new QListWidgetItem(QIcon(":/QQ1.png"),"系统消息"));
                freshUserTable(0);
                ui->tableWidget->clear();
                Toast::instance().show(Toast::INFO,"退出群聊成功!");
            }
            else{
                Toast::instance().show(Toast::INFO,"退出群聊已取消");
            }
        }
    }
}
