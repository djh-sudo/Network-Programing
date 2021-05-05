# Network-Programing
利用QT编写一个QQ聊天程序

# 编程库
1.`QTcpServer`
2.`QTcpSocket`
3.`QUdpSocket`
# 二者交互原理
1.服务器端监听端口
```C
    tcpServer = new QTcpServer(this);
    tcpServer->listen(QHostAddress::Any,8888);
```
2.建立信号和槽连接

一旦有一个新的连接,会自动触发`newConnection`槽函数
之后，只要连接不断开，一旦收到Tcp发送的数据包,会自动触发`readyRead`槽函数。
```
connect(tcpServer,&QTcpServer::newConnection,[=](){
        QTcpSocket* socket = tcpServer->nextPendingConnection();
        
        connect(socket, &QTcpSocket::readyRead,[=](){
        // write you code here...
        
        });
   });
```
# 一个简单的聊天程序由服务端(`Server`)和客户端(`Client`)组成。

To be continue...
