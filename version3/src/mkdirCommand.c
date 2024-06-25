#include "../include/thread_pool.h"
#include"../include/user.h"
//输入格式：XX/XX/XX或XX/XX/XX/
//如果此目录文件已有，则不创建并提醒
//如果此目录文件没有，则创建，并提示创建成功
extern user_info_t* user;
// 判断目录是否存在
int directory_exist(MYSQL *pcoon, int ownerId, int parentId,const char *directoryName) {
    const char *query_file = "SELECT * FROM files WHERE parentId =? AND ownerId =? AND fileName =? AND fileType = 'd'";
    MYSQL_STMT *stmt_file = mysql_stmt_init(pcoon);
    if (stmt_file == NULL) {
        printf("mkdirCommand:directory_exist:mysql_stmt_init() failed\n");
        return 1;
    }

    if (mysql_stmt_prepare(stmt_file, query_file, strlen(query_file))) {
        printf("mkdirCommand:directory_exist:mysql_stmt_prepare() failedn\n");
        mysql_stmt_close(stmt_file);
        return 1;
    }

    MYSQL_BIND bind_file[3];
    int length = strlen(directoryName);
    memset(bind_file, 0, sizeof(bind_file));
    bind_file[0].buffer_type = MYSQL_TYPE_LONG;
    bind_file[0].buffer = &parentId;
    bind_file[0].length = NULL;
    bind_file[0].is_null = 0;
    bind_file[1].buffer_type = MYSQL_TYPE_LONG;
    bind_file[1].buffer = &ownerId;
    bind_file[1].length = NULL;
    bind_file[1].is_null = 0;
    bind_file[2].buffer_type = MYSQL_TYPE_STRING;
    bind_file[2].buffer = (char *)directoryName;
    bind_file[2].length = &length;
    bind_file[2].is_null = 0;

    if (mysql_stmt_bind_param(stmt_file, bind_file)) {
        printf("mkdirCommand:directory_exist:mysql_stmt_bind_param() failed\n");
        mysql_stmt_close(stmt_file);
        return 1;
    }

    if (mysql_stmt_execute(stmt_file)) {
        printf("mkdirCommand:directory_exist:mysql_stmt_execute() failed");
        mysql_stmt_close(stmt_file);
        return 1;
    }

    if (mysql_stmt_store_result(stmt_file)) {
        printf("mkdirCommand:directory_exist:mysql_stmt_store_result() failed\n");
        mysql_stmt_close(stmt_file);
        return 1;
    }

    int count = mysql_stmt_num_rows(stmt_file);
    mysql_stmt_free_result(stmt_file);
    mysql_stmt_close(stmt_file);
    return count;
}

int insert_into_files(MYSQL *pcoon, int parentId, int ownerId, const char *fileName) {
    const char *query_file = "INSERT INTO files (parentId, ownerId, fileName, fileType, modifyTime) VALUES (?, ?, ?, ?, NOW())";
    MYSQL_STMT *stmt_file = mysql_stmt_init(pcoon);
    if (stmt_file == NULL) {
        printf("mkdirCommand:insert_into_files:mysql_stmt_init() failed\n");    
        return 0;
    }

    if (mysql_stmt_prepare(stmt_file, query_file, strlen(query_file)) != 0) {
        printf("mkdirCommand:insert_into_files:mysql_stmt_prepare() failed\n");
        mysql_stmt_close(stmt_file);
        return 0;  // 返回失败
    }

    char fileType = 'd';
    MYSQL_BIND bind_file[4];
    memset(bind_file, 0, sizeof(bind_file));
    bind_file[0].buffer_type = MYSQL_TYPE_LONG;
    bind_file[0].buffer = &parentId;
    bind_file[0].buffer_length = sizeof(parentId);
    bind_file[1].buffer_type = MYSQL_TYPE_LONG;
    bind_file[1].buffer = &ownerId;
    bind_file[1].buffer_length = sizeof(ownerId);
    bind_file[2].buffer_type = MYSQL_TYPE_STRING;
    bind_file[2].buffer = (char *)fileName;
    bind_file[2].buffer_length = strlen(fileName);
    bind_file[3].buffer_type = MYSQL_TYPE_STRING;
    bind_file[3].buffer = &fileType;
    bind_file[3].buffer_length = sizeof(fileType);

    if (mysql_stmt_bind_param(stmt_file, bind_file) != 0) {
        printf("mkdirCommand:insert_into_files:mysql_stmt_bind_param() failed\n");
        mysql_stmt_close(stmt_file);
        return 0;
    }

    if (mysql_stmt_execute(stmt_file) != 0) {
        printf("mkdirCommand:insert_into_files:mysql_stmt_execute() failed\n");
        mysql_stmt_close(stmt_file);
        return 0;  // 返回失败
    } else {
        mysql_stmt_close(stmt_file);
        return 1;  // 返回成功
    }
}



// 从 users 表中查询 userId 和 curDirectory
int get_user_info1(MYSQL *pcoon, const char *username, int *userId, int *curDirectory) {
    MYSQL_STMT *stmt;
    MYSQL_BIND bind_params[1];  
    MYSQL_BIND bind_results[2];
    // unsigned long name_length = strlen(usr->username);
    unsigned long name_length = strlen(username);

    stmt = mysql_stmt_init(pcoon);
    if (!stmt) {
        printf("mkdirCommand:get_user_info:mysql_stmt_init() failed\n");
        return 0;
    }

    // 准备预处理查询语句
    const char *query = "SELECT userId, curDirectory FROM users WHERE userName = ?";
    if (mysql_stmt_prepare(stmt, query, strlen(query))) {
        printf("mkdirCommand:get_user_info:mysql_stmt_prepare() failed\n");
        mysql_stmt_close(stmt);
        return 0;
    }

    // 初始化输入参数绑定数组
    memset(bind_params, 0, sizeof(bind_params));

    // 配置 name 参数的绑定信息
    bind_params[0].buffer_type = MYSQL_TYPE_STRING; 
    bind_params[0].buffer = (char *)username;
    //bind_params[0].buffer = (char *)usr->username;
    bind_params[0].is_null = 0;
    bind_params[0].length = &name_length;

    // 绑定输入参数
    if (mysql_stmt_bind_param(stmt, bind_params)) {
        printf("mkdirCommand:get_user_info:mysql_stmt_bind_param() failed\n");
        mysql_stmt_close(stmt);
        return 0;
    }

    // 初始化输出结果绑定数组
    memset(bind_results, 0, sizeof(bind_results));

    // 配置 id 参数的绑定信息
    bind_results[0].buffer_type = MYSQL_TYPE_LONG;
    bind_results[0].buffer =userId ;
    bind_results[0].length = 0;

    // 配置 directory 参数的绑定信息
    bind_results[1].buffer_type = MYSQL_TYPE_LONG;
    bind_results[1].buffer = curDirectory;
    bind_results[1].length = 0;

    // 绑定输出结果
    if (mysql_stmt_bind_result(stmt, bind_results)) {
        printf("mkdirCommand:get_user_info:mysql_stmt_bind_result() failed\n");
        mysql_stmt_close(stmt);
        return 0;
    }

    // 执行预处理查询语句
    if (mysql_stmt_execute(stmt)) {
        printf("mkdirCommand:get_user_info:mysql_stmt_execute() failed\n");
        mysql_stmt_close(stmt);
        return 0;
    }

    // 获取查询结果
    if (mysql_stmt_fetch(stmt)) {
      printf("mkdirCommand:get_user_info:mysql_stmt_fetch() failed\n");
        mysql_stmt_close(stmt);
        return 0;
    } 
 
    // 清理
    mysql_stmt_close(stmt);

    return 1;
}

void mkdirCommand(task_t *task) {
    char str[128] = "";
    int userId, curDirectory;

    if (get_user_info1(task->pconn, user->name, &userId, &curDirectory)) {// 查询成功
        if (directory_exist(task->pconn, userId, curDirectory, task->data)) {//判断是否重复
            sprintf(str, "Directory already exist.\n");
            sendn(task->peerfd,str,strlen(str));
        }
        else {
            //创建目录
            if (insert_into_files(task->pconn, curDirectory, userId, task->data)) {
                sprintf(str, "Crectory success.\n");
                sendn(task->peerfd,str,strlen(str));
            }
            else {
                sprintf(str, "Make directory.\n");
                sendn(task->peerfd,str,strlen(str));
            }
        }
    }
    else {
        sprintf(str,"mkdirCommand:get_user_info filed.\n");
        sendn(task->peerfd,str,strlen(str));
    }

    return ;
}



