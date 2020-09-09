#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

//	修改匹配IP
#define IP "0.0.0.0"

typedef struct sockaddr* SP;
int cli_sock;
int rcv_sock;
char username[20];
int sockfd;

void reg()
{
	printf("输入用户名:");
	gets(username);
}

void* rcv(void* arg)
{
	int sockfd = *(int*)arg;
	char buf[4096];
	size_t buf_size = sizeof(buf);
	//	接收消息
	for(;;)
	{
		//	清空buf原有内容
		memset(buf,0,buf_size);
		size_t ret_size = recv(sockfd,buf,buf_size,0);
		if(0 >= ret_size)
		{
			perror("recv");
			//close(sockfd);
			return NULL;
		}
        printf("\n%s\n",buf);
	}
}

int main(int argc, char const *argv[])
{
	//	输入账号名字
	reg();
    // 创建socket
    int cli_sock = socket(AF_INET,SOCK_STREAM,0);
    if(0 > cli_sock)
    {
        perror("socket");
        return EXIT_FAILURE;
    }
    // 准备通信地址(由服务端决定)
    struct sockaddr_in svr_addr = {};
    svr_addr.sin_family = AF_INET;
    svr_addr.sin_port = htons(1204);
    svr_addr.sin_addr.s_addr = inet_addr(IP);
	socklen_t addrlen = sizeof(svr_addr);
    // 连接
    if(connect(cli_sock,(SP)&svr_addr,addrlen))
    {
    	perror("connect");
	    return EXIT_FAILURE;
	}
	//	发送用户名
	send(cli_sock,username,strlen(username),0);
	//	用户名不合法[uf]
	//	创建一个线程,用于接收消息
	pthread_t tid;
	pthread_create(&tid,NULL,rcv,&cli_sock);
	char buf[4096] = {};
    size_t buf_size = sizeof(buf);
	//	正式通讯内容
    for(;;)
    {
		//	提示输入符号(常显示uf)
        printf(">>>");
		memset(buf,0,buf_size);
        gets(buf);
        send(cli_sock,buf,strlen(buf),0);
        if(0 == strcmp("quit",buf))
        {
            printf("通信结束!\n");
            break;
        }
    }
    // 关闭socket
    close(cli_sock);
    return 0;
}
