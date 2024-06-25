#include "pwd.h"


// 根据用户表中的username 获取user表中的 userId curDirectory
UserInfo getUserInfo(MYSQL *conn, const char *username) {
    UserInfo userInfo = {-1, -1};  // 初始化用户信息
    char query[256];
    
    // 构建SQL查询语句
    snprintf(query, sizeof(query), "SELECT userId, curDirectory FROM users WHERE userName = '%s'", username);

    // 执行SQL查询
    printf("开始执行SQL查询语句\n");
    if (mysql_query(conn, query)) {
        fprintf(stderr, "MySQL Query Error: %s\n", mysql_error(conn));
        return userInfo;
    }

    // 存储查询结果
    MYSQL_RES *res = mysql_store_result(conn);
    if (res == NULL) {
        fprintf(stderr, "MySQL Store Result Error: %s\n", mysql_error(conn));
        return userInfo;
    }

    // 提取查询结果
    MYSQL_ROW row = mysql_fetch_row(res);
    if (row) {
        userInfo.userId = atoi(row[0]);
        userInfo.curDirectory = atoi(row[1]);

        // DEBUG
        // printf("根据用户提供的username， 解析当前用户Id，和工作目录Id");
        // printf("当前用户名为: %s, 当前用户Id: %d, 当前工作目录Id: %d \n",
        //       username, userInfo.userId, userInfo.curDirectory);
    }

    // 释放结果集
    mysql_free_result(res);
    return userInfo;
}

// 获取目录路径的函数实现
void getDirectoryPath(MYSQL *conn, int userId, int parentId, char *path) {
    char stack[1024][256];  // 栈用于暂存目录名
    int top = -1;           // 栈顶指针

    // 向上遍历目录树直到根目录
    while (parentId != 1) {
        char query[256];

        // 构建SQL查询语句
        snprintf(query, sizeof(query), "SELECT fileId, fileName, parentId FROM files WHERE ownerId = %d AND fileId = %d", userId, parentId);

        // 执行SQL查询
        if (mysql_query(conn, query)) {
            fprintf(stderr, "MySQL Query Error: %s\n", mysql_error(conn));
            exit(EXIT_FAILURE);
        }

        // 存储查询结果
        MYSQL_RES *res = mysql_store_result(conn);
        if (res == NULL) {
            fprintf(stderr, "MySQL Store Result Error: %s\n", mysql_error(conn));
            exit(EXIT_FAILURE);
        }

        // 提取查询结果
        MYSQL_ROW row = mysql_fetch_row(res);
        if (!row) {
            mysql_free_result(res);
            break;
        }

        // 更新parentId和栈
        parentId = atoi(row[2]);
        top++;
        strncpy(stack[top], row[1], sizeof(stack[top]));
        stack[top][sizeof(stack[top]) - 1] = '\0';  // 保证字符串终止

        // 释放结果集
        mysql_free_result(res);
    }

    // 拼接路径
    while (top >= 0) {
        strcat(path, "/");
        strcat(path, stack[top]);
        top--;
    }
    printf("函数 path : %s", path);
}

// 处理 "pwd" 命令的函数实现
void pwdCommand(task_t *task) {
    const char *username = user->name;
    
    // 获取用户信息
    UserInfo userInfo = getUserInfo(task->pconn, username);
    if (userInfo.userId == -1) {
        // 用户不存在或查询错误，发送错误信息给客户端
        const char *errorMsg = "User not found\n";
        send(task->peerfd, errorMsg, strlen(errorMsg), 0);
        return;
    }

    // 获取目录路径
    char path[1024] = "";
    getDirectoryPath(task->pconn, userInfo.userId, userInfo.curDirectory, path);

    // 向客户端发送路径
    send(task->peerfd, path, strlen(path), 0);
}

