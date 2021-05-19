#include "database.h"
#include<QDebug>
#include<QSqlRecord>
#include<QDateTime>
DataBase::DataBase(QString sqlName,QString tableName,QWidget *parent):QMainWindow(parent)
{
    this->sqlName = sqlName;
    this->tableName = tableName;
}
DataBase::DataBase(QWidget *parent):QMainWindow(parent){}
bool DataBase::inicialTable(){
    if(QSqlDatabase::contains(sqlName))//"SERVER"
        db = QSqlDatabase::database(sqlName);
    else db = db.addDatabase("QSQLITE",sqlName);
    db.setDatabaseName(tableName);//"DATABASE.db"
    if(!db.open()){
        emit Send("[# inicial database] fail to open the database!");
        emit Send(db.lastError().text());
        return false;
    }
    else{
        QStringList tables = db.tables();  //获取数据库中的表
        qDebug() << QString::fromLocal8Bit("number: %1").arg(tables.count()); //打印表的个数
        QStringListIterator itr(tables);
        while (itr.hasNext())
        {
            QString tableNmae = itr.next().toLocal8Bit();
            qDebug() << QString::fromLocal8Bit("name: ")+ tableNmae;
        }
        emit Send("[# inicial database] open the database successfully!");
    }
    query = QSqlQuery(db);
    return true;
}
bool DataBase::createTable(QString name){
    QString cmd= "create table if not exists "+name+" (userId BIGINT PRIMARY KEY,userName varchar(50),password varchar(200),state int,friendNumber int);";
    if(!query.exec(cmd))
    {
        emit Send("[# create table] create user table error!" +query.lastError().text());
        return false;
    }
    emit Send("[# create table] table creates successfully!");
    QString res = "update "+name+" set state = 0;";
    if(!query.exec(res)){
        emit Send("[# create table] " + query.lastError().text());
        return false;
    }
    emit Send("[# start the database] inicial state successfully!");
    return true;
}
bool DataBase::createTable(int id){
    //根据id创为用户建一个数据库
    QString cmd= "create table if not exists \""+QString::number(id)+"\" (userId BIGINT PRIMARY KEY,time varchar(50));";
    if(!query.exec(cmd))
    {
        emit Send("[# create table] create friend table error! " + query.lastError().text());
        return false;
    }
    emit Send("[# create table] user <" + QString::number(id) + "> friend table creates successfully!");
    return true;
}
bool DataBase::insertData(QString userTableName,int ID, QString userName, QString password,int friendNumber){
    QString res = "select userId from "+userTableName+" where userId = "+QString::number(ID)+";";
    query.exec(res);
    if(query.next()){
        emit Send(query.next() + " userID is exsisted!");
        return false;
    }
    else{
        QString insertContent = "insert into "+userTableName+" (userId,userName,password,state,friendNumber) values("+QString::number(ID,10)+",\""+userName+"\",\""+password+"\",0,"+QString::number(friendNumber)+");";
        bool flag = query.exec(insertContent);
        if(flag){
            emit Send("[# insert data] insert successfully!");
            return true;
        }
        else{
            emit Send("[# insert data] insert failed! " + query.lastError().text());
            return false;
        }
    }
}
int DataBase::checkUser(QString userTableName,int id, QString password){
    QString res = "select userId,password from "+userTableName+" where userId= "+QString::number(id,10)+";";
    query.exec(res);
    int checkId = -1;
    QString checkPassword = "";
    int index = 0;
    while(query.next()){
        index = 1;
        checkId = query.value(0).toInt();
        checkPassword = query.value(1).toString();
    }
    if(index){
        if(id == checkId){
            if(password == checkPassword){
                emit Send("[# checkUser] login successfully!");
                return 1;
            }
            else{
                emit Send("[# checkUser] password is incorrect!");
                return 0;
            }
        }
        else{
            emit Send("[# checkUser] <" +QString::number(id) + "> pls register first!");
            return -1;
        }
    }
    else{
        emit Send("[# checkUser] <" +QString::number(id) + "> pls register first!");
        return -1;
    }
}
bool DataBase::checkState(int userID,QString userTableName){
    QString res = "select state from "+userTableName+" where userId = "+QString::number(userID)+";";
    query.exec(res);
    if(query.next()){
        if (query.value(0)=="1")
            return true;
        else
            return false;
    }
    return false;
}
QString DataBase::allData(QString userTableName,int id){
    QString res = "select * from "+userTableName+" where userId = "+QString::number(id,10)+";";
    QString data = "";
    query.exec(res);
    if(query.next()){
        data += query.value(0).toString()+"##";//id
        data += query.value(1).toString()+"##";//name
        data += query.value(2).toString()+"##";//password
        data += query.value(3).toString()+"##";//state
        data += query.value(4).toString()+"##";//friend
    }
    return data;
}
QString DataBase::allFriendData(QString friendTableName){
    QString cmd = "select *from \"" + friendTableName + "\";";
    QString data = "";
    query.exec(cmd);
    if(query.next()){
        data += query.value(0).toString() + "##";//fid
        data += query.value(1).toString() + "##";//time
    }
    return data;
}
bool DataBase::deleteFriendData(QString friendTableName, int id){//还需要修改用户的friend的数量
    QString cmd = "select userId from \"" + friendTableName + "\" where userId = " + QString::number(id,10) + ";";
    query.exec(cmd);
    if(query.next()){
        cmd = "delete from \"" + friendTableName + "\" where userId = " + QString::number(id,10)+";";
        query.exec(cmd);
        if(query.next()){
            emit Send("[# deleteFriendData] delete friend data ok!");
            return true;
        }
    }
    else{
        emit Send("[# deleteFriendData] error " + query.lastError().text());
        emit Send("[# deleteFriendData] delete friend data failed!");
        return false;
    }
}
bool DataBase::setState(QString userTableName, int id){
    QString cmd = "update "+userTableName+" set state = 1 where userId = "+QString::number(id,10)+";";
    if(!query.exec(cmd)){
        emit Send("[# set State] " + query.lastError().text());
        return false;
    }
    return true;
}
bool DataBase::resetState(QString userTableName, int id){
    QString cmd = "update "+userTableName+" set state = 0 where userId = "+QString::number(id,10)+";";
    if(!query.exec(cmd)){
        emit Send("[# reset State] " + query.lastError().text());
        return false;
    }
    return true;
}
bool DataBase::insertData(QString userTabelName,QString friendTabelName,int id,int friendId){
    QString cmd = "select userId from "+userTabelName+" where userId = "+QString::number(friendId)+";";
    if(!(query.exec(cmd))){
        emit Send("[#insert data] " + query.lastError().text());
        return false;
    }
    else{
        if(query.next()){//找到用户
            QDateTime time = QDateTime::currentDateTime();
            QString addTime = time.toString("yyyy-MM-dd-hh-mm");
            cmd = "insert into \"" + friendTabelName + "\" (userId,time) values("+QString::number(friendId) + ",\""+addTime + "\");";
            if(!query.exec(cmd)){
                emit Send("[#insert data] friend table flash failed! " + query.lastError().text());
                return false;
            }
            else{
                emit Send("[#insert data] add friend successfully!");
                return true;
            }
        }
        else{
            emit Send("[#insert data] user is not exists! "+QString::number(id));
            return false;
        }
    }
}
DataBase::~DataBase(){
    db.close();
}
bool DataBase::addFriend(QString userTableName, int id){
    QString cmd = "select friendNumber from "+userTableName +" where userId = " + QString::number(id) + ";";
    if(query.exec(cmd)){
        query.next();
        int number = query.value(0).toInt();
        number = number + 1;
        cmd = "update " + userTableName+" set friendNumber = "+QString::number(number)+" where userId = " + QString::number(id) + ";";
        if(query.exec(cmd)){
            emit Send("[# addFriend] friend number update successfully!");
            return true;
        }
        else{
            emit Send("[# addFriend] friend number update failed" + query.lastError().text());
            return false;
        }
    }
    else{
        emit Send("[# addFriend] " + query.lastError().text());
        return false;
    }
}
bool DataBase::deleteFriend(QString userTableName, int id){
    QString cmd = "select friendNumber from " + userTableName + " where userId = " + QString::number(id) + ";";
    if(query.exec(cmd)){
        query.next();
        int number = query.value(0).toInt();
        number = number -1;
        if(number<0){
            emit Send("[# deleteFriend] user <" + QString::number(id) + "> have no friend!");
            return false;
        }else{
            cmd = "update " + userTableName + " set friendNumber = " + QString::number(number) + " where userId = " + QString::number(id) + ";";
            if(query.exec(cmd)){
                emit Send("[# deleteFriend] friend number update successfully!");
                return true;
            }else{
                emit Send("[# deleteFriend] friend number update failed!");
                emit Send("[# error] "+query.lastError().text());
                return false;
            }
        }
    }else{
        emit Send("[# deleteFriend] " + query.lastError().text());
        return false;
    }
}
bool DataBase::resetUser(QString userTableName,int id){
    QString cmd = "select userId from " + userTableName + " where userId = " + QString::number(id)+";";
    query.exec(cmd);
    if(query.next()){
        cmd = "update " + userTableName + " set friendNumber = 0 where userId = " + QString::number(id)+";";
        if(query.exec(cmd)){
            emit Send("[# resetUser] reset <" + QString::number(id) + "> successfully!");
            return true;
        }
        else
            Send("[# resetUser] reset <" + QString::number(id) + "> failed!");
        return false;
    }
}
bool DataBase::is_Friend(QString id1,QString id2){
    QString cmd = "select userId from \"" + id1 + "\" where userId = "+ id2+";";
    if(query.exec(cmd)){
        if(query.next()){
            if(query.value(0) == id2){
                emit Send("[# is_Friend] <"+ id1 + "> and <" + id2 + "> is already be friend!");
                return true;
            }else{
                emit Send("[# is_Friend] <"+ id1 + "> and <" + id2 + "> is not friend!");
                return false;
            }
        }
        else{
            emit Send("[# is_Friend] <"+ id1 + "> and <" + id2 + "> is not friend!");
            return false;
        }
    }else{
        emit Send("[# is_Friend] " + query.lastError().text());
        return false;
    }
}
bool DataBase::is_InGroup(QString gid, QString userId){
    QString cmd = "select userId from \"" + gid + "\" where userId = " + userId + ";";
    if(query.exec(cmd)){
        if(query.next()){
            if(query.value(0) == userId){
                emit Send("[# is_InGroup] <" + userId + " is already in group <" + gid + "> !");
                return true;
            }
            else{
                return false;
            }
        }
        return false;
    }
    else{
        emit Send("[# is_InGroup] " + query.lastError().text());
        return false;
    }
}
bool DataBase::setPassword(QString userTableName, int id, QString password){
    QString cmd = "select userId from " + userTableName + "where userId = " + QString::number(id)+";";
    if(query.exec()){
        QString uid = query.value(0).toString();
        if(uid == QString::number(id)){
            QString cmd = "update " + userTableName + " set password = " + password +" where userId = " + QString::number(id) + ";";
            if(query.exec()){
                emit("[# setPassword] <" + QString::number(id) + "> set password successfully!");
                return true;
            }else{
                emit("[# setPassword] "+query.lastError().text());
                return false;
            }
        }
        else{
            emit Send("[# error] "+query.lastError().text());
            emit("[# setPassword]< "+ QString::number(id) + "> set password failed!");
        }
    }
    else emit(query.lastError());
    return false;
}
int DataBase::getUserNumber(QString userTabelName){
    QString cmd = "select count(*) from " + userTabelName + ";";
    if(query.exec(cmd)){
        if(query.next()){
            emit Send("[# userNumber] from <" + userTabelName + "> get number is ok!");
            return query.value(0).toInt();
        }
    }
    else{
        emit Send("[# userNumber] " + query.lastError().text());
        return 0;
    }
}
QString DataBase::getAllUserInfo(QString userTabel){
    QString cmd = "select * from " +userTabel + ";";
    if(query.exec(cmd)){
        QString res = "";
        while(query.next()&&query.value(0).isValid()){
            emit Send("[# getAllUserInfo] from <" + userTabel + "> get <"+ query.value(0).toString() +"> infomation is ok!");
            res += query.value(0).toString()+"##" + query.value(1).toString()+"##" + query.value(3).toString() + "##";
        }
        return res;
    }
    else{
        emit Send("[# getAllUserInfo] " + query.lastError().text());
        return "";
    }
}
QString DataBase::getAllGroupInfo(QString groupTableName){
    QString cmd = "select * from " + groupTableName + ";";
    QString res = "";
    if(query.exec(cmd)){
        while(query.next() && query.value(0).isValid()){
            emit Send("[# getAllGroupInfo] from <" + groupTableName + "> get group <" + query.value(0).toString() +"> is ok!");
            res += query.value(0).toString() + "##" + query.value(1).toString() + "##" + query.value(2).toString() + "##";
        }
        return res;
    }
    else{
        emit Send("[# getAllGroupInfo] " + query.lastError().text());
        return "";
    }
}
bool DataBase::createGroup(QString groupTableName){
    QString cmd = "create table if not exists " + groupTableName + " (groupId BIGINT PRIMARY KEY,userId BIGINT,name varchar(200),number int);";
    if(query.exec(cmd)){
        emit Send("[# createGroup] inicial Group successfully!");
        return true;
    }
    else{
        emit Send("[# createGroup] " + query.lastError().text());
        emit Send("[# createGroup] inicial Group failed!");
        return false;
    }
}
int DataBase::getGroupNumber(QString groupTableName, QString gid){
    QString cmd = "select number from " + groupTableName + " where groupId = " + gid + ";";
    if(query.exec(cmd)){
        if(query.next()){
            int number = query.value(0).toInt();
            emit Send("[# getGroupNumber] get group number <" + gid + "> is ok!");
            return number;
        }
    }
    else{
        emit Send("[# error] " + query.lastError().text());
        emit Send("[# getGroupNumber] get group number <" + gid + "> failed!");
        return 0;
    }
}
bool DataBase::createUserGroup(QString groupTableName,QString groupId){
    QString cmd = "select groupId from " + groupTableName +" where groupId = " + groupId + ";";
    if(query.exec(cmd)){
        if(query.next()){
            cmd = "create table if not exists \"" +groupId + "\" (userId BIGINT PRIMARY KEY);";
            if(query.exec(cmd)){
                emit Send("[# groupId] create <" + groupId +"> is ok!");
                return true;
            }
            else{
                emit Send("[# createUserGroup] create <"+ groupId + "> is failed!");
                return false;
            }
        }
        else{
            emit Send("[# createUserGroup] <"+ groupId +"> is not exists!");
            return false;
        }
    }
    else{
        emit Send("[# createUserGroup] error! "+query.lastError().text());
        return false;
    }
}
bool DataBase::addComponent(QString userGroupTable,QString gid,QString uid){
    //创建一个gid
    QString cmd = "insert into \"" + gid + "\" (userId) values(" + uid + ");";
    if(query.exec(cmd)){
        emit Send("[# addComponent] insert <" + uid + "> into <" + gid + "> is ok!");
        if(setComponent(userGroupTable,gid.toUtf8().toInt())){
            emit Send("[# setComponent] update the number of <" + gid + "> is ok!");
            return true;
        }
        else{
            emit Send("[# setComponent] error " + query.lastError().text());
            emit Send("[# setComponent] update the number of <" + gid + "> is failed!");
            return false;
        }
    }
    else{
        emit Send("[# addComponent] insert <" + uid + "> into <" + gid + "> is failed!");
        return false;
    }
}
bool DataBase::insertComponent(QString groupTableName, QString gid, QString uid,QString name){
    QString cmd = "insert into " + groupTableName + " (groupId,userId,name,number) values(" + gid + "," + uid + ",\"" + name + "\",0);";
    if(query.exec(cmd)){
        emit Send("[# insertComponent] insert a new group for <" + uid + "> is ok!");
        return true;
    }
    else{
        emit Send("[# error] "+query.lastError().text());
        emit Send("[# insertComponent] insert a new group for <" + uid + "> is failed!");
        return false;
    }
}

bool DataBase::deleteComponent(QString groupTableName,QString gid, QString uid){
    QString cmd = "delete from \"" + gid + "\" where userId = " + uid + ";";
    if(query.exec(cmd)){
        emit Send("[# deleteComponent] delete the user <" + uid +" > from group <" +gid + "> is ok");
        int number = getGroupNumber(groupTableName,gid);
        if(number>0){
            number--;
            cmd = "update " + groupTableName + " set number = " + QString::number(number) + " where groupId = " + gid + ";";
            if(query.exec(cmd)){
                emit Send("[# deleteComponent] update group number is ok!");
                return true;
            }
            else{
                emit Send("[# deleteComponent] update group number is failed!");
                return false;
            }
        }else{
            emit Send("[#deleteComponent] number of group <" + gid + "> is zero!");
            return false;
        }
    }
    else{
        emit Send("[# deleteComponent] delete the user <" + uid +" > from group <" +gid + "> is failed");
        emit Send("[# deleteComponent] " + query.lastError().text());
        return false;
    }
}
bool DataBase::deleteGroup(QString groupTableName){
    QString cmd = "drop table " +groupTableName + ";";
    if(query.exec(cmd) && query.next()){
        emit Send("[# deletegroup] drop table <" + groupTableName + "> is ok!");
        return true;
    }
    else{
        emit Send("[# error] " + query.lastError().text());
        emit Send("[# deletegroup] drop table <" + groupTableName + "> is failed!");
        return false;
    }
}
bool DataBase::deleteUserGroup(QString groupTableName,QString groupId){
    QString cmd = "select groupId from " + groupTableName + " where groupId = " + groupId +";";
    if(query.exec(cmd)){
        if(query.next()){
            cmd = "delete from " + groupTableName + " where groupId = " + groupId + ";";
            if(query.exec(cmd)){
                emit Send("[# deleteUserGroup] delete the group <" + groupId + "> from " + groupTableName + " is ok!");
                cmd =  "drop table \"" + groupId +"\";";
                if(query.exec(cmd)){
                    emit Send("[# deleteUserGroup] delete group <" + groupId + "> is ok!");
                    return true;
                }else{
                    emit Send("[# deleteUserGroup] group <" + groupId + "> is existed!, but failed to delete it");
                    emit Send("[# deleteUserGroup] error "+query.lastError().text());
                    return false;
                }
            }else{
                emit Send("[# deleteUserGroup] delete the group <" + groupId + "> from " + groupTableName + " is failed!");
                return false;
            }

        }
        else{
            emit Send("[# deleteUserGroup] userGroup <" + groupId + "> is not exists!");
            emit Send("[# deleteUserGroup] error " + query.lastError().text());
            return false;
        }
    }else{
        emit Send("[# deleteUserGroup] excute delete userGroup <" + groupId + "> is failed!");
        emit Send("[# deleteUserGroup] error " + query.lastError().text());
        return false;
    }
}
bool DataBase::setComponent(QString groupTableName, int groupId){
    int number = getGroupNumber(groupTableName,QString::number(groupId));
    number++;
    QString cmd = "update " + groupTableName + " set number = " + QString::number(number) + " where groupId = " + QString::number(groupId) + ";";
    if(query.exec(cmd)){
        emit Send("[# setComponent] update the group <" + QString::number(groupId) + "> number is ok!");
        return true;
    }
    else{
        emit Send("[# setComponent] update the group <" + QString::number(groupId) + "> number is failed!");
        return false;
    }
}
QString DataBase::getGroupData(QString groupTableName, int groupId){
    QString cmd = "select * from " + groupTableName + " where groupId = " + QString::number(groupId) + ";";
    if(query.exec(cmd)){
        if(query.next()){
            emit Send("[# getGroupData] get GroupId ok!");
            QString res = query.value(0).toString() + "##" + query.value(1).toString()+ "##" +  query.value(2).toString() + "##";
            return res;
        }
        emit Send("[# getGroupData] groupId is not exists!");
        return "";
    }
    else{
        emit Send("[# getGroupData] get groupId failed!");
        return "";
    }
}
int DataBase::getGroupNumber(QString groupName){
    QString cmd = "select count(*) from " + groupName +";";
    if(query.exec(cmd)){
        if(query.next()){
            emit Send("[# getGroupNumber] get group number is ok!");
            return query.value(0).toInt();
        }
    }
    else{
        emit Send("[# getGroupNumber] " + query.lastError().text());
        return 0;
    }
}
QString DataBase::getGroupAdmin(QString groupTableName, int groupId){
    QString cmd = "select userId from " + groupTableName + " where groupId = " + QString::number(groupId) + ";";
    if(query.exec(cmd)){
        if(query.next() && query.value(0).isValid()){
            emit Send("[# getGroupAdmin] get group <" + QString::number(groupId) +"> is ok!");
            return QString::number(query.value(0).toInt());
        }else{

        }
    }else{
        emit Send("[# getGroupAdmin] get group <" + QString::number(groupId) +"> is failed!");
        return "";
    }
}
QString DataBase::getAllGroupInfo(int groupId){
    QString cmd = "select * from \"" + QString::number(groupId) + "\";";
    if(query.exec(cmd)){
        QString res  = "";
        while(query.next()){
            res += query.value(0).toString() + "##";
        }
        emit Send("[# getAllGroupInfo] get group <" + QString::number(groupId) + "> info is ok!");
        return res;
    }
    else{
        emit Send("[# getAllGroupInfo] get group <" + QString::number(groupId) + "> info is failed!");
        return "";
    }
}
