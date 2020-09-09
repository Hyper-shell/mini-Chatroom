#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#define MAX_NUM  50			//	最大客户端数量
#define IP "0.0.0.0"		//	IP地址
typedef struct Client
{
	int cli_sock;
	char name[20];
	pthread_t tid;
	struct sockaddr_in cli_addr;
}Client;
Client client[MAX_NUM];
size_t client_count = 0;	//	当前客户端的数量
typedef struct sockaddr* SP;
void* run(void* arg)
{
	int cli_sock = *(int*)arg;
	int index = 0;
	char buf[4096];
	size_t buf_size = sizeof(buf);
	size_t name_size = sizeof(client->name);
	for(int i=0;i<MAX_NUM;i++)
	{
		if(cli_sock==client[i].cli_sock)
	    {
			index = i;
			break;
     	}
	}
	//	先接收用户名
	size_t ret_size = recv(cli_sock,client[index].name,name_size,0);
	if(0 == ret_size)
	{
		close(client->cli_sock);
		return NULL;
	}
	printf("用户[%s]已上线\n",client[index].name);
	sprintf(buf,"%s进入聊天室!",client[index].name);
	for(int i=0;i<MAX_NUM;i++)
	{
		if(0 != client[i].cli_sock && client[i].cli_sock != cli_sock)
		{
			send(client[i].cli_sock,buf,strlen(buf)+1,0);
		}
	}
	for(;;)
	{
		//	清空原有内容
		memset(buf,0,buf_size);
		//	接收消息
		size_t ret_size = recv(cli_sock,buf,buf_size,0);
		//	收到"quit"后关闭线程
		if(0 == ret_size || 0 == strcmp("quit",buf))
		{
			printf("用户[%s]已下线\n",client[index].name);
			sprintf(buf,"%s已下线!",client[index].name);
			for(int i=0;i<MAX_NUM;i++)
			{
				if(0 != client[i].cli_sock && client[i].cli_sock != cli_sock)
				{
					send(client[i].cli_sock,buf,strlen(buf)+1,0);
				}
			}

			close(client->cli_sock);
			client->cli_sock = 0;
			client_count--;
			return NULL;
		}
		//	服务端显示用户
		printf("用户[%s]:[%s] bits:%d tid:%lu\n",client->name,buf,ret_size,client->tid);	
		//	"广播"收到内容
		char put[4096];
		sprintf(put,"%s:%s",client[index].name,buf);
		for(int i=0;i<MAX_NUM;i++)
		{
			if(0 != client[i].cli_sock && client[i].cli_sock != cli_sock)
			{
				send(client[i].cli_sock,put,strlen(put)+1,0);
			}
		}
	}
}

int main(int argc,const char* argv[])
{
	printf("服务端启动中...\n");
	int svr_sock = socket(AF_INET,SOCK_STREAM,0);
	if(0 > svr_sock)
	{
		perror("socket");
		return EXIT_FAILURE;
	}
	struct sockaddr_in svr_addr = {};
	svr_addr.sin_family = AF_INET;
	svr_addr.sin_port = htons(1204);
	//svr_addr.sin_addr.s_addr = inet_addr(IP);	//	接收特定IP
	svr_addr.sin_addr.s_addr = INADDR_ANY;		//	接收本机所有IP
	socklen_t addrlen = sizeof(svr_addr);
	//	绑定
	if(bind(svr_sock,(SP)&svr_addr,addrlen))
	{
		perror("bind");
		return EXIT_FAILURE;
	}
	//	监听
	if(listen(svr_sock,5))
	{
		perror("listen");
		return EXIT_FAILURE;
	}
	size_t index = 0;
	printf("等待用户..\n");
	for(;;)
	{
		//	有空位
		if(0 == client[index].cli_sock)
		{
			client[index].cli_sock = accept(svr_sock,(SP)&client[index].cli_sock,&addrlen);	
			if(0>client[index].cli_sock)
			{
				perror("accept");
				break;
			}
			//	创建一个线程
			pthread_t tid;
			client[index].tid = tid;
			pthread_create(&client[index].tid,NULL,run,&client[index]);
			client_count++;
		}
		else
		{
			if(client_count>=MAX_NUM)
			{
				//	聊天室满
				printf("网络延迟较高，稍后再试...\n");
			}
			index = (index+1)%50;
		}
	}
}
