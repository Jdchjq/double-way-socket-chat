#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>      //hostent结构体定义在改头文件中

//下面给出hostent结构体的定义，该结构体用于存储域名解析返回的服务器基本信息
/*struct hostent    
{ 
  char * h_name; / *主机的正式名称* / 
  char ** h_aliases; / *别名列表* / 
  int h_addrtype; / *主机地址类型* / 
  int h_length; / *地址的长度* / 
  char ** h_addr_list; / *来自名称服务器的地址列表* / 
  #define h_addr h_addr_list [0] / *地址，以实现向后兼容* / 
};*/


void error(const char *msg)     //错误处理函数
{
    perror(msg);
    exit(0);
}
//运行时一共传三个参数，第一个是clinet，第二个是服务器的主机名，第三个是服务器的端口号
int main(int argc, char *argv[])
{
    int sockfd, portno, n;  //sockfd是socket文件描述符，portno是服务器提供的端口
    struct sockaddr_in serv_addr;
    struct hostent *server;     //定义了hostent指针server,存储服务器的基本信息

    char buffer[256];
    if (argc < 3) {         //传入的参数少于3个，出错
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }
    //1、创建一个socket
    portno = atoi(argv[2]);     //将字符形式的端口号转成整数，并赋值
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
    
    //2、域名解析过程（先查找本地hosts文件，找不到就发送DNS请求），参数是主机名，返回是一个hostent结构体
    server = gethostbyname(argv[1]); 
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    //3、将域名解析得到的信息给struct sockaddr_in serv_addr的成员
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;     //AF_INET表示IPV4协议
    bcopy((char *)server->h_addr,       //复制ip地址给serv_addr.sin_addr.s_addr
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);
    //4、连接服务器
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR connecting");
    //5、写消息，传递
    printf("Please enter the message: ");
    bzero(buffer,256);
    fgets(buffer,255,stdin);
    n = write(sockfd,buffer,strlen(buffer));
    if (n < 0) 
         error("ERROR writing to socket");
    //6、读消息
    bzero(buffer,256);
    n = read(sockfd,buffer,255);
    if (n < 0) 
         error("ERROR reading from socket");
    printf("%s\n",buffer);
    //7、关闭socket连接
    close(sockfd);
    return 0;
}