/*
*@file client.c
*@author ymz
*@date 6-16 版本1 -> 版本2    6.17 zzl add some fixs 6-18 ymz add userlogin 
*@brief 在客户端输入命令，服务端成功接受,gets功能正确接收/增加用户登陆
*/

#include "config.h"
#include "client.h"
#include <func.h>

client_user_info* clientUser;
info io;

int main(int argc, char *argv[])
{
    // ./client ../conf/client.conf 
	//读取配置文件
	HashTable ht;
	initHashTable(&ht);
	readConfig(argv[1],&ht);
	
    int clientfd = socket(AF_INET,SOCK_STREAM,0);
    struct sockaddr_in addr;//服务端地址
    addr.sin_family = AF_INET;
    addr.sin_port = htons(atoi((const char*)find(&ht,PORT)));
    addr.sin_addr.s_addr = inet_addr((const char*)find(&ht,IP));

	//addr.sin_port = htons(atoi(argv[2]));
    //addr.sin_addr.s_addr = inet_addr(argv[1]);

    int ret = connect(clientfd,(struct sockaddr *)&addr,sizeof(addr));
    ERROR_CHECK(ret,-1,"connect");

//add signup
	char welcome[100] ={0};
	welcomeMessage();
	printf(
	"\n"
	"\033[1;31m[root]\033[0m 欢迎来到root组的网盘，请先注册或登录吧\n"
	"\033[1;31m[root]\033[0m 输入login登录，输入signup注册，输入exit退出\n");

	clientUser = (client_user_info*)calloc(1,sizeof(client_user_info));
	
	//begin:
	//scanf("%s",welcome);
	//if(!strcmp(welcome,"login")){
	//	userLogin(clientfd);//user login
	//}else if(!strcmp(welcome,"signup")){
	//	printf("hello\n");
	//	userEnroll(clientfd);//user enroll
	//}else{
	//	printf("请重新输入！\n");
	//	goto begin;
	//}

	while(1){
		scanf("%s",welcome);
		if(!strcmp(welcome,"login")){
			userLogin(clientfd);//user login
			break;
		}else if(!strcmp(welcome,"signup")){
			userEnroll(clientfd);//user enroll
			break;
		}else if(!strcmp(welcome,"exit")){
			printf("\n");
			byebye();
			return 0;
		}
		else{
			printf("\033[1;31m[root]\033[0m请重新输入！\n");
		}
	}

	fd_set rdset;
	FD_ZERO(&rdset);
	train_t t;
	char buff[2000] = {0};

	while(true){
		FD_ZERO(&rdset);
		FD_SET(STDIN_FILENO,&rdset);
		FD_SET(clientfd,&rdset);

		select(clientfd+1,&rdset,NULL,NULL,NULL);

		//输入命令，发送到服务端
		if(FD_ISSET(STDIN_FILENO,&rdset)){
			bzero(buff,sizeof(buff));

			int ret = read(STDIN_FILENO,buff,sizeof(buff));
			//printf("ret: %d\n",ret);
			if(ret==0){
				//ctrl + d, end input
				byebye();
				break;
			}
			
			bzero(&t,sizeof(t));
			buff[strlen(buff)-1]='\0';

			//printf("buff:%s\n",buff);
			parseCommand(buff,strlen(buff),&t);//分析命令
			
			//printf("buff: %s\n",buff);

			sendn(clientfd,&t,4+4+t.len);//发送命令
		
			//如果命令为puts，gets交给子进程
			if(t.type == CMD_TYPE_PUTS){//puts,上传文件
				due_putscommand(clientfd,t);
				

			}else if(t.type == CMD_TYPE_GETS){//gets,下载文件
				due_getscommand(clientfd,t);
			 	
			}
		}
		//如果是给服务端发数据,发送文件
		else if(FD_ISSET(clientfd,&rdset)){	
			if(t.type != CMD_TYPE_GETS&&t.type != CMD_TYPE_GETS){
				bzero(buff,sizeof(buff));
	
				int len = 0;
				if(t.type == CMD_TYPE_TREE){
					bzero(buff,sizeof(buff));
					recvn(clientfd,&len,sizeof(int));
					recvn(clientfd,buff,len);
				}else{
					bzero(buff,sizeof(buff));
					recv(clientfd,buff,sizeof(buff),0);
				}

				
				if(strcmp(buff,"exit") == 0){
					byebye();
					break;
				}else{
					printf("\033[1;31m[root]\033[0m \n");
					printf("%s\n",buff);
				}
			
			}
		}
	}
	close(clientfd);
	return 0;
}

