/*
*@client.h
*@author ymz
*@date 6-18 
*@brief client.c中用到的函数 -> add user login -> add func ll,tree,cats -> fix cmdtype and add userenroll
*/
#pragma once

#define USER_NAME "\033[1;31m[root]\033[0m 请输入用户名\n"
#define PASSWORD "\033[1;31m[root]\033[0m 请输入密码\n"

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

//send username and send encrytptedcode
    CMD_TYPE_USERNAME = 100,
    CMD_TYPE_USERNAME_RESP_OK,
	CMD_TYPE_USERNAME_RESP_ERROR,
    CMD_TYPE_ENCODE,
    CMD_TYPE_ENCODE_RESP_OK,
    CMD_TYPE_ENCODE_RESP_ERROR,

//send username and send encrytptedcode enroll
	CMD_TYPE_USERNAME_EN,
    CMD_TYPE_USERNAME_RESP_OK_EN,
	CMD_TYPE_USERNAME_RESP_ERROR_EN,
    CMD_TYPE_ENCODE_EN,
    CMD_TYPE_ENCODE_RESP_OK_EN,
    CMD_TYPE_ENCODE_RESP_ERROR_EN,

}CmdType;


typedef struct 
{
    int len;//记录内容长度
    CmdType type;//消息类型
    char buff[1000];//记录内容本身
}train_t;

typedef struct{
    train_t t;
    int existFlag;
}info;

typedef struct{
	char username[20];//记录用户名
	char token[300];//记录token
}client_user_info;

//int tcpConnect(const char * ip, unsigned short port);
int recvn(int sockfd, void * buff, int len);
int sendn(int sockfd, const void * buff, int len);

int userLogin(int sockfd);
int userEnroll(int sockfd);//add user enroll

//add user in mysql
int mysqlConnect();
int mysqlEnroll(char* username);


int parseCommand(const char * input, int len, train_t * pt);

//判断一个字符串是什么命令
int getCommandType(const char * str);

void due_putscommand(int clientfd, train_t t);
void due_getscommand(int clientfd, train_t t);
char* generate_sha1(int fd,char *result);

void create_duePG_thread(short port);

void welcomeMessage();
void byebye();
