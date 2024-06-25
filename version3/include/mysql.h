#include <mysql/mysql.h>
#include "../include/user.h"

//连接
MYSQL* mysqlconnect();

//检查用户名是否存在
bool isUserExist(MYSQL* pconn,char* username);

//将用户所有信息放入结构体中
void getAllmessage(MYSQL* pconn,user_info_t* user);

//将注册用户所有信息放入数据库中
void writeIntoSql(MYSQL* pconn,user_info_t* user);
