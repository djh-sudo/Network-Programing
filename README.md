# Network-Programing
利用QT编写一个QQ聊天程序
# 一个简单的聊天程序由服务端(`Server`)和客户端(`Client`)组成。
## 编程库
* 1.`QTcpServer`   2. `QTcpSocket`     3.`QUdpSocket`
## 二者交互原理
### 服务器(`Server`)
* 1.服务器端监听端口
```C
    tcpServer = new QTcpServer(this);
    tcpServer->listen(QHostAddress::Any,8888);
```
* 2.建立信号和槽连接

一旦有一个新的连接,会自动触发`newConnection`信号

之后，只要连接不断开，一旦收到Tcp发送的数据包,会自动触发`readyRead`信号
```C
connect(tcpServer,&QTcpServer::newConnection,[=](){
        QTcpSocket* socket = tcpServer->nextPendingConnection();
        
        connect(socket, &QTcpSocket::readyRead,[=](){
        // write your code here...
        
        });
   });
```
* 3.`Udp`通信
`UDP`通信由于没有事先建立一个逻辑连接,所以服务器端监听端口后，只要有数据发送到对应端口(不丢包的情况下)，就能够收到数据，自动触发`readyRead`信号
```C
connect(udpSocket,&QUdpSocket::readyRead,[=](){
        QByteArray array;
        array.resize(udpSocket->bytesAvailable());
        quint16 port;
        QHostAddress address;
        udpSocket->readDatagram(array.data(),array.size(),&address,&port);
        QString res = array.data();
        
        // write your code here to handle the data
    });
```
##### `Server`源程序结构
```C
cachefile.h             //文件数据读写
database.h              //数据库操作
func.h                  //宏定义以及cpu使用率测量
md5.h                   //计算字符串md5
ringsprogressbar.h      //绘制ringsprogress
server.h                //服务器核心模块
user.h                  //用户
userdata.h              //用户数据格式
```

### 客户端(`Client`)
客户端同样需要连接一个`readyRead`信号，用于接收来自服务器的数据

同时，客户端是主动端，服务器是被动端，所以客户端需要主动去连接服务器
```C
    socket->connectToHost(QHostAddress(ip),port);
```
连接成功后，同样会触发一个`conected`信号

```C
connect(socket,&QTcpSocket::connected,[=](){
    connect(socket,&QTcpSocket::readyRead,[=](){
    
    //write your code here
    });
});
```
#### `Client`源程序结构
```C
cachefile.h                 //缓存文件读写
client.h                    //客户端登录模块
mg5.h                       //计算md5模块
readonlydelegate.h          //只读设置
register.h                  //注册模块
toast.h                     //定制消息
user.h                      //用户管理
userdata.h                  //用户数据
userpanel.h                 //用户面板(核心)
```
# To be continue...
### 服务器端效果展示
![](https://github.com/djh-sudo/Network-Programing/blob/main/Image/Server.png)
### 客户端效果展示
<div align=center><img width="600" height="600" src="https://github.com/djh-sudo/Network-Programing/blob/main/Image/client.png"/></div>


