#ifndef FILE_UTILS_H
#define FILE_UTILS_H
#include "../include/user.h"
#include "../include/thread_pool.h"
#include <mysql/mysql.h>

extern user_info_t * user;

typedef struct {
    int userId;
    int curDirectory;
} UserInfo;

UserInfo getUserInfo(MYSQL *conn, const char *username);
void getDirectoryPath(MYSQL *conn, int userId, int parentId, char *path);

#endif // FILE_UTILS_H

