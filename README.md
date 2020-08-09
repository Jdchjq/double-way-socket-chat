# double-way socket chat
 这是一个工作在Linux、Unix系统上的，全双工通信程序，基于socket实现。

使用教程：
1、先编译server.c和client.c

$gcc -o server server.c

$gcc -o client client.c

2、修改hosts文件。在客户主机上的hosts文件中，加入服务器的主机名和服务器IP地址的映射.
文件路径是  /etc/hosts

3、在服务器主机上先执行server，端口号最好选择20000～65535之间的
这里我选了一个57273，如果该端口已经被占用，再选其他的端口号。
在sever程序运行之前，最好先赋予该文件可执行的权限。

$chmod 777 server

$./server 57273

3、在客户主机上执行client，假设服务器的主机名是 ‘chan’。

$./client chan 57273

4、此时，双方可以互相发送和接收消息。
