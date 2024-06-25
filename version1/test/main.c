/*
*@file client.c
*@author ymz
*@date 6-16 版本1 -> 版本2    6.17 zzl add some fixs 6-18 ymz add userlogin 
*@brief 在客户端输入命令，服务端成功接受,gets功能正确接收/增加用户登陆
*/

#include "config.h"
#include "client.h"
#include <func.h>

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

	userLogin(clientfd);//user login

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

			printf("here!\n");
			
			int ret = read(STDIN_FILENO,buff,sizeof(buff));
			printf("ret: %d\n",ret);
			if(ret==0){
				//ctrl + d, end input
				printf("byebye.\n");
				break;
			}
			
			bzero(&t,sizeof(t));
			buff[strlen(buff)-1]='\0';

			printf("buff:%s\n",buff);
			parseCommand(buff,strlen(buff),&t);//分析命令
			
			printf("buff: %s\n",buff);

			sendn(clientfd,&t,4+4+t.len);//发送命令
			
			if(t.type == CMD_TYPE_PUTS){//puts,上传文件
				char filename[50] = {0};
				strcpy(filename,t.buff);
				int fd = open(filename,O_RDWR);
				struct stat st;
				bzero(&st,sizeof(st));
				fstat(fd,&st);
				
				sendn(clientfd,(char*)&st.st_size,sizeof(st.st_size));//发送文件大小
				int ret = sendfile(clientfd,fd,NULL,st.st_size);//发送文件
				printf("send %d bytes.\n",ret);
			}

		}
		//如果是给服务端发数据,发送文件
		else if(FD_ISSET(clientfd,&rdset)){
			
			 if(t.type == CMD_TYPE_GETS){//gets,下载文件
				char filename[50] = {0};
				int filename_len = 0;
				off_t file_len = 0;
				recvn(clientfd,&filename_len,sizeof(filename_len));//获取文件名的长度
				printf("filename_len: %d\n",filename_len);	

				recvn(clientfd,filename,filename_len);//获取文件名
				printf("filename: %s\n",filename);
				
				int fd = open(filename,O_CREAT|O_RDWR|O_TRUNC,0666);

				recvn(clientfd,(char*)&file_len,sizeof(file_len));//获取文件长度
				printf("mmap file length: %ld\n", file_len);	

				ftruncate(fd,file_len);

				//最后接收文件内容
				char * pMap = mmap(NULL, file_len, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
				if(pMap) {//映射成功
				//clientfd代表的是内核态套接字的缓冲区
				//pMap代表的是内核文件缓冲区
					int ret = recv(clientfd, pMap, file_len, MSG_WAITALL);
					printf("ret: %d\n", ret);
				}
			}
			else{
				bzero(buff,sizeof(buff));
				recv(clientfd,buff,sizeof(buff),0);
				printf("recv:%s\n",buff);
			
			}
		}
		
	}
	close(clientfd);
	return 0;
}
