#ifndef __USER_H__
#define __USER_H__

#include <mysql/mysql.h>
typedef enum {
    STATUS_LOGOFF = 0,
    STATUS_LOGIN
}LoginStatus;

typedef struct {
    int sockfd;//套接字文件描述符
    LoginStatus status;//登录状态
    char name[20];//用户名(客户端传递过来的)
	char salt[20];//获取盐值
    char encrypted[100];//从/etc/shadow文件中获取的加密密文 -> 在数据库中获取密码
    char pwd[128];//用户当前路径
}user_info_t;

//从数据库中查找用户信息
void loginCheck1(MYSQL* pconn,user_info_t* user);//获取用户信息，在mysql中查询
void loginCheck2(user_info_t* user, const char * encrypted);//在数据库中对比密码，发送给客户端是否正确

//add enroll check
void enrollCheck1(MYSQL* pconn,user_info_t* user);
void enrollCheck2(MYSQL* pconn, const char * encrypted,user_info_t* user);

//generate saltstr
void GenerateStr(char* salt);

//generate Token
int jwtEncode(unsigned char* userName, char* Token);

#endif

