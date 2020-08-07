/* A simple server in the internet domain using TCP
   The port number is passed as an argument 
   This version runs forever, forking off a separate 
   process for each connection
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
  short sin_family; / *必须是AF_INET * / 
  u_short sin_port; 
  struct in_addr sin_addr; 
  char sin_zero [8]; / *不使用，必须为零* / 
};
*/

struct parament
{
    int socketfd;
    char *servername;
    char *clientname;
};

void dostuff(int); /* function prototype */
void waitchildsig(int signl);      //接受子进程发来的结束消息
void pthread_recv(void *arg);
void pthread_send(void *arg);
struct parament init_pthread_parament(int fd);
void error(const char *msg)
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
int main(int argc, char *argv[])
{
     int sockfd, newsockfd, portno, pid;
     socklen_t clilen;
     struct sockaddr_in serv_addr, cli_addr;

     if (argc < 2) {
         fprintf(stderr,"ERROR, no port provided\n");
         exit(1);
     }
     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0) 
        error("ERROR opening socket");
     bzero((char *) &serv_addr, sizeof(serv_addr));
     portno = atoi(argv[1]);
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(portno);
     if (bind(sockfd, (struct sockaddr *) &serv_addr,
        sizeof(serv_addr)) < 0) 
        error("ERROR on binding");
     listen(sockfd,5);
     clilen = sizeof(cli_addr);
     while (1){
        newsockfd = accept(sockfd, 
            (struct sockaddr *) &cli_addr, &clilen);
        if(newsockfd < 0) 
            error("ERROR on accept");
        pid = fork();      //创建子进程
        if(pid < 0)
            error("ERROR on fork");
        if(pid == 0){
            dostuff(newsockfd);
            exit(0);
        }
        else{
            signal(SIGCHLD,waitchildsig);   //通过特定函数处理该信号
            close(newsockfd);
        } 

    } /* end of while */

     close(sockfd);
     return 0; /* we never get here */
}

/******** DOSTUFF() *********************
 There is a separate instance of this function 
 for each connection.  It handles all communication
 once a connnection has been established.
 *****************************************/
void dostuff (int sock)
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
