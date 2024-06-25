/*
 *@author cqy created
 *file mycd.c
 *data 2024-6-21
 *@brief 接收到cd命令之后，根据后面的task_data切换到相对应的目录
 */


#include "../include/thread_pool.h"
#include "../include/user.h"
//定义上一次的目录和当前目录
int pre_cwd = 0;
int cur_cwd = 0;
// 函数用于获取并返回前两个目录的路径  
extern int parentId;
extern user_info_t* user;
int yu_userId(MYSQL * pconn, user_info_t * user, int userId){
    //查询操作
    char query[2048] = {0};
    sprintf(query,"select userId from users where userName = '%s'",user->name);

    int ret = mysql_query(pconn, query);
    if(ret != 0){
        printf("(%d, %s)\n",mysql_errno(pconn), mysql_error(pconn));
        return EXIT_FAILURE;
    }
    //查询成功的情况下，获取到结果集
    //mysql_use_result并不存储结果集
    MYSQL_RES * result = mysql_store_result(pconn);
    if(result){
        //获取结果集的行列信息
        int cols = mysql_num_fields(result);
        int rows = mysql_num_rows(result);
        printf("cols:%d, rows:%d\n",cols,rows);
        MYSQL_ROW row;
        if((row = mysql_fetch_row(result)) != NULL){
            // 使用strtol来安全地将字符串转换为整数
            char *endptr; // 用于strtol的结束指针
            userId = strtol(row[0], &endptr, 10); // 10表示基数为10的整数
        }
    }
    printf("yu_userId = %d\n",userId);
    mysql_free_result(result);
    return userId;
}

int yu_fileId(MYSQL * pconn, int parentId, int ownerId, int fileId){
    //查询操作
    char query[2048] = {0};
    sprintf(query,"select fileId from files where parentId = %d and ownerId = %d",parentId,ownerId);
    int ret = mysql_query(pconn, query);
    if(ret != 0){
        printf("(%d, %s)\n",mysql_errno(pconn), mysql_error(pconn));
        return EXIT_FAILURE;
    }
    //查询成功的情况下，获取到结果集
    //mysql_use_result并不存储结果集
    MYSQL_RES * result = mysql_store_result(pconn);
    if(result){
        //获取结果集的行列信息
        int cols = mysql_num_fields(result);
        int rows = mysql_num_rows(result);
        printf("cols:%d, rows:%d\n",cols,rows);
        MYSQL_ROW row;
        if((row = mysql_fetch_row(result)) != NULL){
            // 使用strtol来安全地将字符串转换为整数
            char *endptr; // 用于strtol的结束指针
            fileId = strtol(row[0], &endptr, 10); // 10表示基数为10的整数
        }
    }
    mysql_free_result(result);
    printf("yu_fileId = %d\n",fileId);
    return fileId;
}

int yu_parentId(MYSQL * pconn, int fileId, int ownerId, int parentId){
    //查询操作
    char query[2048] = {0};
    sprintf(query,"select parentId from files where fileId = %d and ownerId = %d",fileId,ownerId);
    int ret = mysql_query(pconn, query);
    if(ret != 0){
        printf("(%d, %s)\n",mysql_errno(pconn), mysql_error(pconn));
        return EXIT_FAILURE;
    }
    //查询成功的情况下，获取到结果集
    //mysql_use_result并不存储结果集
    MYSQL_RES * result = mysql_store_result(pconn);
    if(result){
        //获取结果集的行列信息
        int cols = mysql_num_fields(result);
        int rows = mysql_num_rows(result);
        if(cols != 1 ||rows != 1){
            printf("cols and rows error.\n");
            return EXIT_FAILURE;
        }
            printf("cols:%d, rows:%d\n",cols,rows);
        MYSQL_ROW row;
        if((row = mysql_fetch_row(result)) != NULL){
            // 使用strtol来安全地将字符串转换为整数
            char *endptr; // 用于strtol的结束指针
            parentId = strtol(row[0], &endptr, 10); // 10表示基数为10的整数
        }
    }
    mysql_free_result(result);
    printf("yu_parentId = %d\n",parentId);
    return parentId;
}


void cdCommand(task_t * task)
{

    //extern ListNode * userList;
    //extern user_info_t * user = (user_info_t *)userList->val;
    int userId = 0;
    int fileId = 0;
    userId = yu_userId(task->pconn, user,userId);
    //定义一个现在所在的目录，用于更新上次工作所在的目录
    int ownerId = userId;
    int cwd = parentId;

    //判断task->data是不是".."
    if(strcmp(task->data,"..") == 0){
        if(parentId == 0){
            //已经在第一级（根）目录，cd失败
            char err[100] = "返回上一级目录失败，已在第一级目录";
            sendn(task->peerfd, err, strlen(err));
        }else{
            fileId = parentId;
            parentId = yu_parentId(task->pconn, fileId, ownerId, parentId);
            printf("par_fileId = %d\n",parentId);
            pre_cwd = cwd;
            cur_cwd = parentId;
        }
        //判断是不是"."
    }else if(strcmp(task->data, ".") == 0){
    }else if(strcmp(task->data, "-") == 0){
        //返回上一次操作的目录
        parentId = pre_cwd;
        //更新之前和现在所在的目录
        pre_cwd = cwd;
        cur_cwd = parentId;

    }else if(strcmp(task->data, "/") == 0){
        //返回根目录（不知道根目录是啥，先用家目录代替）
        parentId = 0;
        //更新之前和现在所在的目录
        pre_cwd = cwd;
        cur_cwd = parentId;
    }else if(strcmp(task->data, "~") == 0 || strcmp(task->data, "") == 0){
        //返回家目录
        parentId = 0;
        //更新之前和现在所在的目录
        pre_cwd = cwd;
        cur_cwd = parentId;
    }else{
        //查询操作
        char query[2048] = {0};
        printf("fileName = %s, wonerId = %d\n", task->data, ownerId);
        sprintf(query,"select fileId from files where fileName = '%s' and ownerId = %d",task->data,ownerId);
        printf("query = %s\n",query);
        int ret = mysql_query(task->pconn, query);
        printf("ret = %d",ret);
        if(ret != 0){
            char err[100] = {0};
            sprintf(err,"(%d, %s)\n",mysql_errno(task->pconn), mysql_error(task->pconn));
            sendn(task->peerfd, err, strlen(err));
            return;
        }
        //查询成功的情况下，获取到结果集
        //mysql_use_result并不存储结果集
        MYSQL_RES * result = mysql_store_result(task->pconn);
        if(result){
            //获取结果集的行列信息
            int cols = mysql_num_fields(result);
            int rows = mysql_num_rows(result);

            if(cols != 1 ||rows != 1){
                printf("cols and rows error.\n");
                char err[100] = "不符合cd的格式，请输入一个当前目录下的目录。\n";
                sendn(task->peerfd, err, strlen(err));
                return;
            }
            MYSQL_ROW row;
            if((row = mysql_fetch_row(result)) != NULL){
        printf("row[0] = %s\n", row[0]);
                // 使用strtol来安全地将字符串转换为整数
                char *endptr; // 用于strtol的结束指针
                parentId = strtol(row[0], &endptr, 10); // 10表示基数为10的整数
            }
        }
        printf("parentId = %d\n", parentId);
        pre_cwd = cwd;
        cur_cwd = parentId;
        mysql_free_result(result);
    }     
}

