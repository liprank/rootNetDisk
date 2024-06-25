#include "../include/thread_pool.h"
#include"../include/user.h"

// 从 users 表中查询 userId 和 curDirectory
int find_userId(MYSQL * pconn, const char* username, int* userId, int* curDirectory) {
    MYSQL_STMT* stmt;
    MYSQL_BIND bind_params[1];
    MYSQL_BIND bind_results[2];
    // unsigned long name_length = strlen(usr->username);
    unsigned long name_length = strlen(username);

    stmt = mysql_stmt_init(pconn);
    if (!stmt) {
        printf("rmCommand:find_userId:mysql_stmt_init() failed\n");
        return 0;
    }

    // 准备预处理查询语句
    const char* query = "SELECT userId, curDirectory FROM users WHERE userName = ?";
    if (mysql_stmt_prepare(stmt, query, strlen(query))) {
        printf("rmCommand:find_userId:mysql_stmt_prepare() failed\n");
        mysql_stmt_close(stmt);
        return 0;
    }

    // 初始化输入参数绑定数组
    memset(bind_params, 0, sizeof(bind_params));

    // 配置 name 参数的绑定信息
    bind_params[0].buffer_type = MYSQL_TYPE_STRING;
    bind_params[0].buffer = (char*)username;
    //bind_params[0].buffer = (char *)usr->username;
    bind_params[0].is_null = 0;
    bind_params[0].length = &name_length;

    // 绑定输入参数
    if (mysql_stmt_bind_param(stmt, bind_params)) {
        printf("rmCommand:find_userId:mysql_stmt_bind_param() failed\n");
        mysql_stmt_close(stmt);
        return 0;
    }

    // 初始化输出结果绑定数组
    memset(bind_results, 0, sizeof(bind_results));

    // 配置 id 参数的绑定信息
    bind_results[0].buffer_type = MYSQL_TYPE_LONG;
    bind_results[0].buffer = userId;
    bind_results[0].length = 0;

    // 配置 directory 参数的绑定信息
    bind_results[1].buffer_type = MYSQL_TYPE_LONG;
    bind_results[1].buffer = curDirectory;
    bind_results[1].length = 0;

    // 绑定输出结果
    if (mysql_stmt_bind_result(stmt, bind_results)) {
        printf("rmCommand:find_userId:mysql_stmt_bind_result() failed\n");
        mysql_stmt_close(stmt);
        return 0;
    }

    // 执行预处理查询语句
    if (mysql_stmt_execute(stmt)) {
        printf("rmCommand:find_userId:mysql_stmt_execute() failed\n");
        mysql_stmt_close(stmt);
        return 0;
    }

    // 获取查询结果
    if (mysql_stmt_fetch(stmt)) {
        printf("rmCommand:find_userId:mysql_stmt_fetch() failed\n");
        mysql_stmt_close(stmt);
        return 1;

    }
    mysql_stmt_free_result(stmt);

    // 清理
    mysql_stmt_close(stmt);

    return 1;
}

//查询fileId
int find_fileId(MYSQL* pconn, int parentId, int ownerId, const char* filename, int* fileId) {
    MYSQL_STMT* stmt;
    MYSQL_BIND bind_params[3];
    MYSQL_BIND bind_results[1];


    stmt = mysql_stmt_init(pconn);
    if (!stmt) {
        printf("rmCommand:find_fileId:mysql_stmt_init() failed\n");
        return 1;
    }

    // 准备预处理查询语句
    const char* query = "SELECT fileId FROM files WHERE parentId = ? AND ownerId = ? AND filename = ? AND fileType = 'r'";
    if (mysql_stmt_prepare(stmt, query, strlen(query))) {
        printf("rmCommand:find_fileId:mysql_stmt_prepare() failed\n");
        mysql_stmt_close(stmt);
        return 1;
    }

    // 初始化输入参数绑定数组
    memset(bind_params, 0, sizeof(bind_params));

    // 配置 parentId 参数的绑定信息
    bind_params[0].buffer_type = MYSQL_TYPE_LONG;
    bind_params[0].buffer = &parentId;
    bind_params[0].is_null = 0;
    bind_params[0].length = NULL;

    // 配置 ownerId 参数的绑定信息
    bind_params[1].buffer_type = MYSQL_TYPE_LONG;
    bind_params[1].buffer = &ownerId;
    bind_params[1].is_null = 0;
    bind_params[1].length = NULL;

    // 配置 filename 参数的绑定信息
    unsigned long length = strlen(filename);
    bind_params[2].buffer_type = MYSQL_TYPE_STRING;
    bind_params[2].buffer = (char*)filename;
    bind_params[2].is_null = 0;
    bind_params[2].length = &length;

    // 绑定输入参数
    if (mysql_stmt_bind_param(stmt, bind_params)) {
        printf("rmCommand:find_fileId:mysql_stmt_bind_param() failed\n");
        mysql_stmt_close(stmt);
        return 1;
    }

    // 初始化输出结果绑定数组
    memset(bind_results, 0, sizeof(bind_results));

    // 配置 id 参数的绑定信息
    bind_results[0].buffer_type = MYSQL_TYPE_LONG;
    bind_results[0].buffer = fileId;
    bind_results[0].buffer_length = sizeof(int);

    // 绑定输出结果
    if (mysql_stmt_bind_result(stmt, bind_results)) {
        printf("rmCommand:find_fileId:mysql_stmt_bind_result() failed\n");
        mysql_stmt_close(stmt);
        return 1;
    }

    // 执行预处理查询语句
    if (mysql_stmt_execute(stmt)) {
        printf("rmCommand:find_fileId:mysql_stmt_execute() failed\n");
        mysql_stmt_close(stmt);
        return 1;
    }

    // 获取查询结果
    if (mysql_stmt_fetch(stmt)) {
        printf("rmCommand:find_fileId:mysql_stmt_fetch() failed\n");
        mysql_stmt_close(stmt);
        return 1;
    }			

    // 清理
    mysql_stmt_close(stmt);
    return 0;

}

//删除文件
void rmfile(MYSQL* pconn, int fileId) {
    //删除parentId为fileId,ownerId为userId的文件fileId,filename，fileType
    MYSQL_STMT* stmt;
    MYSQL_BIND bind_params[1];

    stmt = mysql_stmt_init(pconn);
    if (!stmt) {
        printf("rmCommand:rmfile:mysql_stmt_init() failed\n");
        return;
    }

    // 准备预处理查询语句
    const char* query = "DELETE FROM files WHERE fileId = ? ";
    if (mysql_stmt_prepare(stmt, query, strlen(query))) {
        printf("rmCommand:rmfile:mysql_stmt_prepare() failed\n");
        mysql_stmt_close(stmt);
        return;
    }

    // 初始化输入参数绑定数组
    memset(bind_params, 0, sizeof(bind_params));

    // 配置 fileId 参数的绑定信息
    bind_params[0].buffer_type = MYSQL_TYPE_LONG;
    bind_params[0].buffer = &fileId;
    bind_params[0].is_null = 0;
    bind_params[0].length = NULL;



    if (mysql_stmt_bind_param(stmt, bind_params)) {
        printf("rmCommand:rmfile:mysql_stmt_bind_param() failed\n");
        mysql_stmt_close(stmt);
        return;
    }

    // 执行预处理查询语句
    if (mysql_stmt_execute(stmt)) {
        printf("rmCommand:rmfile:mysql_stmt_execute() failed\n");
        mysql_stmt_close(stmt);
        return;
    }
    mysql_stmt_close(stmt);
    return;
}
              

void rmCommand(task_t *task){
    extern user_info_t* user;

    // 确保data字段包含有效的目录路径
    if (strlen(task->data) == 0) {
        //fprintf(stderr, "路径错误！\n");
        char err[100] = "路径错误！";
        send(task->peerfd, err, strlen(err), 0);
        return;
    }

    // 检查是否为根目录
    if (strcmp(task->data, "/") == 0) {
       // fprintf(stderr, "无法删除根目录！\n");
        char err[100] = "无法删除根目录！";
        send(task->peerfd, err, strlen(err), 0);
        return;
    }

    //在users表中获得userId,curDirectory
    int userId, curDirectory, fileId;
    char fileName[256] = {0};
    strcpy(fileName, task->data);
    MYSQL *pconn = task->pconn;
    //查询useId和curDirectory -> ownerId和parentId
    if (find_userId(pconn, user->name, &userId, &curDirectory)) {// 查询成功
        printf("userId:%d,curDirectory:%d\n", userId, curDirectory);

        //查询fileId
        //成功则删除，失败则返回
        if(find_fileId(pconn, curDirectory, userId, fileName, &fileId)){
            char err[100] = "删除失败！";
            send(task->peerfd, err, strlen(err), 0);
        }
        else{
            //根据fileId删除文件
            rmfile(pconn, fileId);
            char err[100] = "删除成功!";
            send(task->peerfd, err, strlen(err), 0);
        }
    }
    else {
        printf("find_userId() failed\n");
    }

    return;

}

