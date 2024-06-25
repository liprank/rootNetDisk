/*
*@client.h
*@author ymz
*@date 6-18 
*@brief client.c中用到的函数 -> add user login -> add func ll,tree,cats
*/
#pragma once

#define USER_NAME "please input a valid user name:\n"
#define PASSWORD "please input the right password:\n"

typedef enum {
    CMD_TYPE_PWD=1,
    CMD_TYPE_LS,
	CMD_TYPE_LL,      //add ll
    CMD_TYPE_CD,
    CMD_TYPE_MKDIR,
	CMD_TYPE_RM,      //add rm
    CMD_TYPE_RMDIR,
	CMD_TYPE_PUTS,    //add puts
	CMD_TYPE_GETS,    //add gets
	CMD_TYPE_TREE,    //add tree
	CMD_TYPE_CAT,     //add cat
    CMD_TYPE_NOTCMD,  //不是命令

    TASK_LOGIN_SECTION1 = 100,
    TASK_LOGIN_SECTION1_RESP_OK,
    TASK_LOGIN_SECTION1_RESP_ERROR,
    TASK_LOGIN_SECTION2,
    TASK_LOGIN_SECTION2_RESP_OK,
    TASK_LOGIN_SECTION2_RESP_ERROR,

}CmdType;


typedef struct 
{
    int len;//记录内容长度
    CmdType type;//消息类型
    char buff[1000];//记录内容本身
}train_t;

//int tcpConnect(const char * ip, unsigned short port);
int recvn(int sockfd, void * buff, int len);
int sendn(int sockfd, const void * buff, int len);

int userLogin(int sockfd);

int parseCommand(const char * input, int len, train_t * pt);

//判断一个字符串是什么命令
int getCommandType(const char * str);
