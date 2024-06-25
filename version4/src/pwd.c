#include "../include/pwd.h"

extern user_info_t* user;  
extern int parentId;
int yu_userId(MYSQL * pconn, user_info_t * user, int userId);
int yu_fileId(MYSQL * pconn, int parentId, int ownerId, int fileId);
int yu_parentId(MYSQL * pconn, int fileId, int ownerId, int parentId);
// 根据用户表中的username 获取user表中的 userId curDirectory
//UserInfo getUserInfo(MYSQL *conn, const char *username) {
//    UserInfo userInfo = {-1, -1};  // 初始化用户信息
//    char query[256];
//
//    // 构建SQL查询语句
//    snprintf(query, sizeof(query), "SELECT userId, curDirectory FROM users WHERE userName = '%s'", username);
//
//    // 执行SQL查询
//    printf("开始执行SQL查询语句\n");
//    if (mysql_query(conn, query)) {
//        fprintf(stderr, "MySQL Query Error: %s\n", mysql_error(conn));
//        return userInfo;
//    }
//
//    // 存储查询结果
//    MYSQL_RES *res = mysql_store_result(conn);
//    if (res == NULL) {
//        fprintf(stderr, "MySQL Store Result Error: %s\n", mysql_error(conn));
//        return userInfo;
//    }
//
//    // 提取查询结果
//    MYSQL_ROW row = mysql_fetch_row(res);
//    if (row) {
//        userInfo.userId = atoi(row[0]);
//        userInfo.curDirectory = atoi(row[1]);
//
//        // DEBUG
//        // printf("根据用户提供的username， 解析当前用户Id，和工作目录Id");
//        // printf("当前用户名为: %s, 当前用户Id: %d, 当前工作目录Id: %d \n",
//        //       username, userInfo.userId, userInfo.curDirectory);
//    }
//
//    // 释放结果集
//    mysql_free_result(res);
//    return userInfo;
//}
//
//// 获取目录路径的函数实现
//
//// 获取目录路径的函数实现
//void getDirectoryPath(MYSQL *conn, int userId, int curDirectory, char *path) {
//    char stack[1024][256];  // 栈，用于暂存目录名
//    int top = -1;           // 栈顶指针
//    int parentId = curDirectory;
//
//    char query[256];
// 
//    // 构建SQL查询语句，根据parentId查询其相关的 fileId, fileName 和 parentId
//    snprintf(query, sizeof(query), "SELECT fileId, fileName, parentId FROM files WHERE ownerId = %d AND parentId = %d", userId, parentId);
//
//    // 执行SQL查询
//    if (mysql_query(conn, query)) {
//        fprintf(stderr, "MySQL Query Error: %s\n", mysql_error(conn));
//        exit(EXIT_FAILURE);
//    }
//
//    // 存储查询结果
//   // MYSQL_RES *res = mysql_store_result(conn);
//   // if (res == NULL) {
//   //     fprintf(stderr, "MySQL Store Result Error: %s\n", mysql_error(conn));
//   //     exit(EXIT_FAILURE);
//   // }
//
//    // 提取查询结果
//    //MYSQL_ROW row = mysql_fetch_row(res);
//    //if (!row) {
//    //  mysql_free_result(res);
//    //     break;
//    //}
//
//    // 更新parentId并压栈目录名
//    //strncpy(stack[++top], row[1], sizeof(stack[top]) - 1);
//    //stack[top][sizeof(stack[top]) - 1] = '\0';  // 确保字符串终止
//
//    //sprintf(path, "/%s", row[1]);
//    //printf("path = %s", row[1]);
//
//    // 释放结果集
//   // mysql_free_result(res);
//
//    char home[1024] = "/home";
//    char newDest[1024] = "" ;
//    strcpy(newDest, home);
//    char new_2[1024] = "";
//        while(parentId != 0){
//
//            memset(new_2, '\0', sizeof(new_2));
//            memset(newDest, '\0', sizeof(newDest));
//
//            char query[256];
//
//            // 构建SQL查询语句，根据parentId查询其相关的 fileId, fileName 和 parentId
//            snprintf(query, sizeof(query), "SELECT fileName, parentId FROM files WHERE fileId = %d", parentId);
//
//            // 执行SQL查询
//            if (mysql_query(conn, query)) {
//                fprintf(stderr, "MySQL Query Error: %s\n", mysql_error(conn));
//                exit(EXIT_FAILURE);
//            }
//
//            // 存储查询结果
//            MYSQL_RES *res = mysql_store_result(conn);
//            if (res == NULL) {
//                fprintf(stderr, "MySQL Store Result Error: %s\n", mysql_error(conn));
//                exit(EXIT_FAILURE);
//            }
//
//            // 提取查询结果
//            MYSQL_ROW row = mysql_fetch_row(res);
//            if (!row) {
//                mysql_free_result(res);
//                //     break;
//            }
//
//            char * end;
//            parentId = strtol(row[1], &end, 10);
//
//            //atoi(row[0]) = parentId ;  // 获取当前目录的 fileId 作为新的 parentId
//
//            // 更新parentId并压栈目录名
//            strncpy(stack[++top], row[1], sizeof(stack[top]) - 1);
//            stack[top][sizeof(stack[top]) - 1] = '\0';  // 确保字符串终止
//            printf("row[0] = %s\n",row[0] );
//
//
//            strcat(new_2, "/");
//            strcat(new_2, row[0]);
//            strcat(new_2, newDest);
//            strcpy(newDest, new_2);
//            // sprintf(path, "/%s", row[0]);
//            // printf("path = %s", row[0]);
//
//            // 释放结果集
//            mysql_free_result(res);
//        }
//        strcat(new_2, home);
//        strcat(new_2, newDest);
//        strcpy(newDest, new_2);
//
//    
//
//    strcpy(path, newDest );
//    printf("%s", path);
//
//}
//
//

// 处理 "pwd" 命令的函数实现
void pwdCommand(task_t *task) {
    // const char *username = user->name;
    int userId = 0;
    userId= yu_userId(task->pconn, user, userId);
    int ownerId = userId;
    printf("ownerId = %d\n", ownerId);                                                                     
    int fileId = 0;
    fileId = yu_fileId(task->pconn, parentId, ownerId, fileId);
    int curDirectory = parentId;



    char home[1024] = "/home";
    char newDest[1024] = "" ;
    while(curDirectory != 0){
        char query[256];

        // 构建SQL查询语句，根据parentId查询其相关的 fileId, fileName 和 parentId
        snprintf(query, sizeof(query), "SELECT fileName, parentId FROM files WHERE fileId = %d", curDirectory);

        // 执行SQL查询
        if (mysql_query(task->pconn, query)) {
            fprintf(stderr, "MySQL Query Error: %s\n", mysql_error(task->pconn));
            //exit(EXIT_FAILURE);
            printf("mysql_query出问题了\n");
        }
        MYSQL_RES* result = mysql_store_result(task->pconn);
        if(result){
            MYSQL_ROW row;
            while((row = mysql_fetch_row(result)) != NULL){
                char * end;
                curDirectory = strtol(row[1], &end, 10);

                //atoi(row[0]) = parentId ;  // 获取当前目录的 fileId 作为新的 parentId

                printf("row[0] = %s\n",row[0] );


                char new_2[1024] = "";
                strcat(new_2, "/");
                strcat(new_2, row[0]);
                strcat(new_2, newDest);
                strcpy(newDest, new_2);
                printf("curDirectory = %d, newDest = %s",curDirectory, newDest);
                // sprintf(path, "/%s", row[0]);
                // printf("path = %s", row[0]);

            }
        }
        // 释放结果集

        mysql_free_result(result);
    }
    strcat(home, newDest);
    strcpy(newDest, home);



    printf("%s", newDest);

    //``  // 获取用户信息
    //``  UserInfo userInfo = getUserInfo(task->pconn, username);
    //``  if (userInfo.userId == -1) {
    //``      // 用户不存在或查询错误，发送错误信息给客户端
    //``      const char *errorMsg = "User not found\n";
    //``      send(task->peerfd, errorMsg, strlen(errorMsg), 0);
    //``      return;
    //``  }

    //``  // 获取目录路径
    //``  char path[1024] = {0};
    //``  getDirectoryPath(task->pconn, userInfo.userId, userInfo.curDirectory, path);

    // 向客户端发送路径
    sendn(task->peerfd, newDest, strlen(newDest));
}

