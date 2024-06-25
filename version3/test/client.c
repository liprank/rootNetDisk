/*
*@file client.c
*@author ymz
*@date 6-16 版本1 -> 版本2    6.17 zzl add some fixs 6-18 ymz add userlogin 6-19 fix cmdtype and add enroll
*@brief 在客户端输入命令，服务端成功接受,gets功能正确接收/增加用户登陆
*/

#include "client.h"
#include <func.h>
#include "splitString.h"

char Salt[20];

int sendn(int sockfd, const void * buff, int len)
{
	int left = len;
	const char * pbuf = buff;
	int ret = 0;
	while(left > 0) {
		ret = send(sockfd, pbuf, left, 0);
		if(ret == -1) {
			perror("send");
			return -1;
		}
		pbuf += ret;
		left -= ret;
	}
	return len - left;
}


int recvn(int sockfd, void * buff, int len)
{
	int left = len;
	char * pbuf = buff;
	int ret = 0;
	while(left > 0) {
		ret = recv(sockfd, pbuf, left, 0);
		if(ret < 0) {
			perror("recv");
			break;
		} else if (ret == 0) {
			break;
		}
		pbuf += ret;
		left -= ret;
	}
	return len - left;
}

int getCommandType(const char* str){
	if(!strcmp(str,"pwd"))
		return CMD_TYPE_PWD;
	else if(!strcmp(str,"ls"))
		return CMD_TYPE_LS;
	else if(!strcmp(str,"ll"))//add ll
		return CMD_TYPE_LL;
	else if(!strcmp(str,"cd"))
		return CMD_TYPE_CD;
	else if(!strcmp(str,"mkdir"))
		return CMD_TYPE_MKDIR;
	else if(!strcmp(str,"rm"))//add rm
		return CMD_TYPE_RM;
	else if(!strcmp(str,"rmdir"))
		return CMD_TYPE_RMDIR;
	else if(!strcmp(str,"puts"))
		return CMD_TYPE_PUTS; 
	else if(!strcmp(str,"gets"))
		return CMD_TYPE_GETS; 
	else if(!strcmp(str,"tree"))//add tree
		return CMD_TYPE_TREE;
	else if(!strcmp(str,"cat"))//add cat
		return CMD_TYPE_CAT;
	else 
		return CMD_TYPE_NOTCMD; 
}

int parseCommand(const char* pinput,int len, train_t *pt){
	char* pstrs[10] = {0};

	char temp[100];
	strncpy(temp,pinput,sizeof(temp)-1);
	temp[sizeof(temp)-1] = '\0';
	
	int cnt=0;
	splitString(temp," ",pstrs,10,&cnt);//分词
	pt->type=(CmdType)getCommandType(pstrs[0]);//根据第一个字段获取命令类型

	if(cnt>1){
		pt->len=strlen(pstrs[1]);
		strncpy(pt->buff,pstrs[1],pt->len);
	}

	
	return 0;
}

//add userlogin
static int userLogin1(int sockfd, train_t *t);
static int userLogin2(int sockfd, train_t *t);

int userLogin(int sockfd)
{
    int ret;
    train_t t;
    memset(&t, 0, sizeof(t));
    userLogin1(sockfd, &t);
    userLogin2(sockfd, &t);
    return 0;
}

static int userLogin1(int sockfd, train_t *pt)
{
    printf("userLogin1.\n");
    train_t t;
    memset(&t, 0, sizeof(t));
    while(1) {
        printf(USER_NAME);
        char user[20]= {0};
        int ret = read(STDIN_FILENO, user, sizeof(user));
        user[ret - 1] = '\0';
        t.len = strlen(user);
        t.type = CMD_TYPE_USERNAME;
        strncpy(t.buff, user, t.len);
        ret = sendn(sockfd, &t, 8 + t.len);
        printf("login1 send %d bytes.\n", ret);

        //接收信息
        memset(&t, 0, sizeof(t));
        ret = recvn(sockfd, &t.len, 4);
        printf("length: %d\n", t.len);
        ret = recvn(sockfd, &t.type, 4);
        if(t.type == CMD_TYPE_USERNAME_RESP_ERROR) {
            printf("用户名不存在，请重新设置.\n");
            continue;
        }

        //用户名正确，读取setting
        ret = recvn(sockfd, t.buff, t.len);
		bzero(Salt,sizeof(Salt));
		strcpy(Salt,t.buff);

        break;
    }
    memcpy(pt, &t, sizeof(t));
    return 0;
}

static int userLogin2(int sockfd, train_t * pt)
{
    printf("userLogin2.\n");
    int ret;
    train_t t;
    memset(&t, 0, sizeof(t));
    while(1) {
        char * passwd = getpass(PASSWORD);
		//TODO:直接发送密码，需要加密
		char encrytped[100] = {0};
		strcpy(encrytped,crypt(passwd,Salt));

        t.len = strlen(encrytped);
        t.type = CMD_TYPE_ENCODE;
        strncpy(t.buff, encrytped, t.len);
        ret = sendn(sockfd, &t, 8 + t.len);

		bzero(&t,sizeof(t));
        ret = recvn(sockfd, &t.len, 4);
        ret = recvn(sockfd, &t.type, 4);
		
        if(t.type == CMD_TYPE_ENCODE_RESP_ERROR) {
            //密码不正确
            printf("sorry, password is not correct.\n");
            continue;
        } else {
            ret = recvn(sockfd, t.buff, t.len);
            printf("Login Success.\n");
            printf("please input a command.\n");
            fprintf(stderr, "%s", t.buff);
            break;
        } 
    }
    return 0;
}

//add userenroll
static int userEnroll1(int sockfd, train_t *pt);
static int userEnroll2(int sockfd, train_t *pt);

int userEnroll(int sockfd){
	train_t t;
	bzero(&t,sizeof(t));
	userEnroll1(sockfd,&t);
	userEnroll2(sockfd,&t);
}

//check the username is exited, exit is not eithr is ok
static int userEnroll1(int sockfd, train_t *pt){
	printf("userEnroll1.\n");
	train_t t;
	bzero(&t,sizeof(t));
	while(1){
		printf(USER_NAME);
		char user[20] = {0};
		int ret = read(STDIN_FILENO, user, sizeof(user));
		user[ret - 1] = '\0';
		t.len = strlen(user);
		t.type = CMD_TYPE_USERNAME_EN;
		strncpy(t.buff, user, t.len);
		ret = sendn(sockfd, &t, 8 + t.len);//发送注册用户名
		printf("enroll1 send %d bytes.\n",ret);

		//接收消息
		bzero(&t,sizeof(t));
		ret = recvn(sockfd, &t.len, 4);
		printf("length: %d\n", t.len);
		ret = recvn(sockfd, &t.type, 4);
		if(t.type == CMD_TYPE_USERNAME_RESP_ERROR_EN){
			printf("用户名已存在，请重新输入！\n");
			continue;
		}else if(t.type == CMD_TYPE_USERNAME_RESP_OK_EN){//TODO:server 是在哪查的？
			//用户名不存在，进行注册
			//TODO:打开mysql，注册用户表
			printf("用户名不存在，可以注册！\n");
			break;
		}
	}
}

static int userEnroll2(int sockfd, train_t *pt){
	printf("userEnroll2.\n");
	int ret;
	train_t t;
	bzero(&t,sizeof(t));
	while(1){
		char* passwd = getpass(PASSWORD);		
		char encrytped[20] = {0};
		strcpy(encrytped,passwd);

		t.len = strlen(encrytped);
		t.type = CMD_TYPE_ENCODE_EN;
		strncpy(t.buff,encrytped,t.len);
		ret = sendn(sockfd, &t,8+t.len); //发送密码
		printf("send encode : %d bytes\n",ret);

		bzero(&t,sizeof(t));
		ret = recvn(sockfd,&t.type,4);//接收注册是否成功

		if(t.type == CMD_TYPE_ENCODE_RESP_ERROR_EN){
			printf("密码不足8位，请重新设置\n");
			continue;
		}else {
			printf("注册成功.\n");
			printf("please input a command.\n");
			break;
		}
	}
}


