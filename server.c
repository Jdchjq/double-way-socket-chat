/* 这是一个简单的基于TCP的socket传输，
实现客户端和服务器的全双工通信
端口号由执行程序时，通过参数传进程序
当该程序处于监听状态时，客户端就可以进行连接
*/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>     //该头文件定义了sockaddr_in结构
#include <signal.h>
#include <pthread.h>
//这是sockaddr_in结构的定义

/*struct sockaddr_in 
{ 
  short sin_family;  //必须是AF_INET，表示IPV4
  u_short sin_port; 
  struct in_addr sin_addr; 
  char sin_zero [8];   //不使用，必须为零
};
*/

//线程参数
struct parament
{
    int socketfd;   //socket文件描述符
    char *servername;   //服务器主机名
    char *clientname;   //客户端主机名
};

void error(const char *msg);
void dostuff(int); /* 建立连接后，服务器对该连接做的事*/
void waitchildsig(int signl);      //接受子进程发来的结束消息
void pthread_recv(void *arg);       //接受消息线程
void pthread_send(void *arg);       //发送消息线程
struct parament init_pthread_parament(int fd);      //初始化传入线程的参数

int main(int argc, char *argv[])
{
     int sockfd, newsockfd, portno, pid;    //pid是多进程，用于接受多个连接
     socklen_t clilen;        //accept()的第三个参数类型，存放sockaddr_in结构的大小
     struct sockaddr_in serv_addr, cli_addr;    //存放协商好的传输协议信息

     if (argc < 2) {
         fprintf(stderr,"ERROR, no port provided\n");
         exit(1);
     }
     //1、创建socekt
     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0) 
        error("ERROR opening socket");
    //2、给serv_addr赋值，定义了IP地址和端口
     bzero((char *) &serv_addr, sizeof(serv_addr));
     portno = atoi(argv[1]);
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;    //获取本机IP
     serv_addr.sin_port = htons(portno);
     //3、bind()将套接字绑定到 ip地址的端口上
     if (bind(sockfd, (struct sockaddr *) &serv_addr,
        sizeof(serv_addr)) < 0) 
        error("ERROR on binding");
    //4、监听该端口，如果有客户端想建立连接，则继续后面的代码
     listen(sockfd,5);
     clilen = sizeof(cli_addr);
     while (1){
         //5、接受一个连接请求，返回对应该连接的新的套接字
        newsockfd = accept(sockfd, 
            (struct sockaddr *) &cli_addr, &clilen);
        if(newsockfd < 0) 
            error("ERROR on accept");
        pid = fork();      //创建子进程
        if(pid < 0)
            error("ERROR on fork");
        if(pid == 0){
            //6、子进程中dostuff()管理这个连接，父进程继续接受其他连接
            dostuff(newsockfd);
            exit(0);
        }
        else{
            signal(SIGCHLD,waitchildsig);   //通过特定函数处理子进程的结束信号
            close(newsockfd);
        } 
    }
     close(sockfd);     //7、关闭套接字
     return 0;
}

void error(const char *msg)     //出错处理函数
{
    perror(msg);
    exit(1);
}
void waitchildsig(int signl)
{
    if(signl==SIGCHLD)
    {
        waitpid(-1,NULL,WNOHANG);   //等待子进程的信号，WNOHANG表示非阻塞等待
    }
}
void dostuff (int sock) //创建两个线程用于接受和发送消息
{
   int n;
   int erro=0;
   socklen_t len=sizeof(erro);
   char buffer[256];
   pthread_t send,receiv; 
   struct parament par;
   par=init_pthread_parament(sock);
   int ret;
   ret=pthread_create(&send,NULL,(void *)pthread_send,(void *)&par);    //发送数据的子线程
   if(ret!=0)
    {
        perror("can't create send thread\n");
    }
    ret=pthread_create(&receiv,NULL,(void *)pthread_recv,(void*)&par); 
    if(ret!=0)
    {
        perror("can't create recev thread\n");
    }

    pthread_join(send,NULL);    //等待子线程的结束
  
}

void pthread_send(void *arg)
{
    struct parament par;
    int n;
    char buffer[256];  //缓冲区
    par=*((struct parament *)arg);
    printf("chan:welcome,please write your message\n");
    while(1)
    {
        bzero(buffer,256);
        buffer[0]='s';          //在消息头部放一个标志，s表示消息是服务器的
        fgets(buffer+1,254,stdin);        //stdin实现对终端输入的数据进行读入操作
        n = write(par.socketfd,buffer,strlen(buffer));    //写消息
        if (n < 0) 
            printf("ERROR writing to socket");
    }
}

void pthread_recv(void *arg)
{
    struct parament par;
    char buffer[256];
    int n=0;
    par=*((struct parament *)arg);     //传入的参数转回原来的类型
    while(1)
    {
        bzero(buffer,256);
        n = read(par.socketfd,buffer,255);
        if (n < 0) 
            error("ERROR reading from socket");
        if(buffer[0]=='c')      //判断是从客户端发来的消息
        {
            printf("%s:%s\n",par.clientname,buffer+1);
        }
    }

}

struct parament init_pthread_parament(int fd)
{
    struct parament para;
    char hostnambuff[256];      //本机主机名 
    gethostname(hostnambuff,sizeof(hostnambuff));
    para.servername=hostnambuff;
    //para.clientname暂时还无法获取.....
    para.socketfd=fd;
    return para;

}
