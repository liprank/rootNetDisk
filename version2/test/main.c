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
				int existFlag=0;
				strcpy(filename,t.buff);
				int fd = open(filename,O_RDWR);
				struct stat st;
				bzero(&st,sizeof(st));
				fstat(fd,&st);

				recvn(clientfd,(char*)&existFlag,sizeof(existFlag));//收取existFlag
				printf("existFlag:%d",existFlag);

				if(existFlag==1){
					//服务器上文件已存在
					off_t curr_filelen=0;
        			recv(clientfd,&curr_filelen,sizeof(curr_filelen),MSG_WAITALL);//接收服务器上已存在文件大小

					sendn(clientfd, (char*)&st.st_size, sizeof(st.st_size));//发送文件长度

					char buff[1000]={0};
					int len=0;
					while(1) {
						memset(buff, 0, sizeof(buff));
						int ret = read(fd, buff, sizeof(buff));
						if(ret != 1000) {
							printf("read ret: %d\n", ret);
						}
						if(ret == 0) {
							//文件已经读取完毕
							break;
						}
						len = ret;
						ret = sendn(clientfd, buff, len);
						if(ret == -1) {
							break;
						}
						if(ret != 1000) {
							printf("send ret: %d\n", ret);
						}
					}
					printf("send file over.\n");

				}else if(existFlag==0){
					sendn(clientfd, (char*)&st.st_size, sizeof(st.st_size));

					if(st.st_size>=1024*1000000){//大文件
					//最后发送文件内容
					int ret = sendfile(clientfd, fd, NULL, st.st_size);
					printf("send %d bytes.\n", ret);

					}else{
						//正常文件
						char buff[1000]={0};
						int len=0;
						while(1) {
							memset(buff, 0, sizeof(buff));
							int ret = read(fd, buff, sizeof(buff));
							if(ret != 1000) {
								printf("read ret: %d\n", ret);
							}
							if(ret == 0) {
								//文件已经读取完毕
								break;
							}
							len = ret;
							ret = sendn(clientfd, buff, len);
							if(ret == -1) {
								break;
							}
							if(ret != 1000) {
								printf("send ret: %d\n", ret);
							}
						}
						printf("send file over.\n");
					}

				}
			}else if(t.type == CMD_TYPE_GETS){//gets,下载文件
			 	printf("start downloading\n");
				char filename[50] = {0};
				int filename_len = 0;
				off_t file_len = 0;
				
				//recvn(clientfd,&filename_len,sizeof(filename_len));//获取文件名的长度
				//printf("filename_len: %d\n",filename_len);	

				//recvn(clientfd,filename,filename_len);//获取文件名
				strcpy(filename,t.buff);
				printf("filename: %s\n",filename);
			
				//断点重传
				int existFlag = 0;
				DIR *dir = opendir(".");
				struct dirent *entry;
				while((entry = readdir(dir)) != NULL){
					if(strcmp(entry->d_name,filename) == 0){
						existFlag = 1;
					}
				}
				printf("existFlag: %d\n",existFlag);
				if(existFlag == 1){
					sendn(clientfd,&existFlag,sizeof(existFlag));

					int fd = open(filename,O_CREAT|O_RDWR,0666);

					struct stat st;
					fstat(fd,&st);
					off_t curr_filelen = 0;
					curr_filelen = st.st_size;
					sendn(clientfd,&curr_filelen,sizeof(curr_filelen));
					printf("curr_filelen: %ld\n",curr_filelen);

					char buf[1000] = {0};
					
					lseek(fd,curr_filelen,SEEK_SET);

					recv(clientfd,&file_len,sizeof(file_len),MSG_WAITALL);

					int ret=0;
					while(1){
						memset(buff,0,sizeof(buff));
						ret=recv(clientfd,buff,sizeof(buff),0);
						if(ret=0)break;
						write(fd,buff,sizeof(buff));
						
					}

				}
				//不需要断点重传
				else{

					sendn(clientfd,&existFlag,sizeof(existFlag));
					recvn(clientfd,&filename_len,sizeof(filename_len));
					printf("filename_len: %d\n",filename_len);	

					recvn(clientfd,filename,filename_len);
					printf("filename: %s\n",filename);

					int fd = open(filename,O_CREAT|O_RDWR|O_TRUNC,0666);

					recvn(clientfd,(char*)&file_len,sizeof(file_len));//获取文件长度
					printf("mmap file length: %ld\n", file_len);	
					ftruncate(fd,file_len);

					if(file_len >= 1024*1000000){
						//最后接收文件内容
						char * pMap = (char *)mmap(NULL, file_len, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
						if(pMap) {//映射成功
						//clientfd代表的是内核态套接字的缓冲区
						//pMap代表的是内核文件缓冲区
							int ret = recv(clientfd, pMap, file_len, MSG_WAITALL);
							printf("ret: %d\n", ret);
						}	
					}else{
						char buff[1000]={0};
						int ret=0;
						while(1){
							memset(buff,0,sizeof(buff));
							ret=recv(clientfd,buff,sizeof(buff),0);
							if(ret=0)break;
							write(fd,buff,sizeof(buff));
							
						}
					}
				}
				printf("download over\n");
			}
		}
		//如果是给服务端发数据,发送文件
		else if(FD_ISSET(clientfd,&rdset)){	
			if(t.type != CMD_TYPE_GETS&&t.type != CMD_TYPE_GETS){
				bzero(buff,sizeof(buff));
				recv(clientfd,buff,sizeof(buff),0);
				printf("recv:%s\n",buff);
			
			}
		}
	}
	close(clientfd);
	return 0;
}
