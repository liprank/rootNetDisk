/*
*@file client.c
*@author ymz
*@date 6-16 版本1 -> 版本2    6.17 zzl add some fixs 6-18 ymz add userlogin 
*@brief 在客户端输入命令，服务端成功接受,gets功能正确接收/增加用户登陆
*/

#include "client.h"
#include <func.h>

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
	pt->type=getCommandType(pstrs[0]);//根据第一个字段获取命令类型

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
        t.type = TASK_LOGIN_SECTION1;
        strncpy(t.buff, user, t.len);
        ret = sendn(sockfd, &t, 8 + t.len);
        printf("login1 send %d bytes.\n", ret);

        //接收信息
        memset(&t, 0, sizeof(t));
        ret = recvn(sockfd, &t.len, 4);
        printf("length: %d\n", t.len);
        ret = recvn(sockfd, &t.type, 4);
        if(t.type == TASK_LOGIN_SECTION1_RESP_ERROR) {
            //无效用户名, 重新输入
            printf("user name not exist.\n");
            continue;
        }
        //用户名正确，读取setting
        ret = recvn(sockfd, t.buff, t.len);
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
        /* printf("password: %s\n", passwd); */
        char * encrytped = crypt(passwd, pt->buff);
        /* printf("encrytped: %s\n", encrytped); */
        t.len = strlen(encrytped);
        t.type = TASK_LOGIN_SECTION2;
        strncpy(t.buff, encrytped, t.len);
        ret = sendn(sockfd, &t, 8 + t.len);
        /* printf("userLogin2 send %d bytes.\n", ret); */

        memset(&t, 0, sizeof(t));
        ret = recvn(sockfd, &t.len, 4);
        /* printf("2 length: %d\n", t.len); */
        ret = recvn(sockfd, &t.type, 4);
        if(t.type == TASK_LOGIN_SECTION2_RESP_ERROR) {
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
//add userlogin

