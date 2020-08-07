/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>  //该头文件定义了sockaddr_in结构
//这是sockaddr_in结构的定义

/*struct sockaddr_in 
{ 
  short sin_family; / *必须是AF_INET * / 
  u_short sin_port; 
  struct in_addr sin_addr; 
  char sin_zero [8]; / *不使用，必须为零* / 
};
*/


void error(const char *msg)     //错误处理函数
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])    //传入两个参数，默认，argv[0]是地址，argv[1]是端口号
{                                           /*portno是服务器接受连接的端口号*/
    int sockfd, newsockfd, portno;     /*sockfd文件描述符是该端口的主套接字，newsockfd是套接字建立连接后的返回值*/ 
    socklen_t clilen;      //存储客户端地址的大小
    char buffer[256];      //消息缓冲区
    struct sockaddr_in serv_addr, cli_addr;     //sockaddr_in是包含了互联网地址的结构，
    int n;                                //读写函数的文件描述符
    if (argc < 2) {
        fprintf(stderr,"ERROR, no port provided\n");
        exit(1);
    }                                           //创建一个套接字
    sockfd = socket(AF_INET, SOCK_STREAM, 0); //第一个参数表示套接字的地址域，AF_INET表示IPv4，第二个参数表示套接字的类型，SOCK_STREAM表示流套接字，对应可靠传输TCP
    if (sockfd < 0)                            //第三个参数是协议，0表示由操作系统选择合适的协议。根据SOCK_STREAM，操作系统会选择TCP协议
        error("ERROR opening socket");
    
    bzero((char *) &serv_addr, sizeof(serv_addr));      //将缓冲区的所有值设置为0
    portno = atoi(argv[1]);                  //将传入的端口号参数从数字字符串转成整数
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;     //s_addr是long类型，存放服务器的IP地址，用宏定义INADDR_ANY获取
    serv_addr.sin_port = htons(portno);         //将主机字节的顺序转化成网络字节顺序，htons()是将短整型数据转成大端格式（高位在低地址）
    if (bind(sockfd, (struct sockaddr *) &serv_addr,    //bind()函数，将套接字和服务器地质结构绑定，一般出错原因会是该套接字已在计算机上使用
        sizeof(serv_addr)) < 0){
        error("ERROR on binding");
    }
        
    listen(sockfd,5);       //监听该发送给该套接字上的连接请求，5表示该进程正在处理连接时，等待队列的最大限制数量，这也是大多数系统允许的最大数量
    clilen = sizeof(cli_addr);
    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);     //进程阻塞，直到客户端与服务器建立连接，返回一个新的对应该连接的套接字
    if (newsockfd < 0) 
        error("ERROR on accept");
    
    bzero(buffer,256);
    n = read(newsockfd,buffer,255);         //等待并读取客户端发来的信息
    if (n < 0) error("ERROR reading from socket");

    printf("Here is the message: %s\n",buffer);
    n = write(newsockfd,"I got your message",18);   //显示信息，18是信息的大小
    if (n < 0) error("ERROR writing to socket");

    close(newsockfd);
    close(sockfd);
    return 0; 
}
