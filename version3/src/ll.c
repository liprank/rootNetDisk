/*
 *@author cqy created
 *file ll.c
 *date 2024-6-22
 *@brief 接收到ls命令之后，打印当前目录的所有文件信息
 */
#include "../include/thread_pool.h"
#include "../include/user.h"

int yu_userId(MYSQL * pconn, user_info_t * user, int userId);
int yu_fileId(MYSQL * pconn, int parentId, int ownerId, int fileId);
int yu_parentId(MYSQL * pconn, int fileId, int ownerId, int parentId);
char* yu_userName(MYSQL * pconn, int userId, char* userName, size_t buf_size){
//查询操作
    char query[2048] = {0};
    sprintf(query,"select userName from users where userId = %d", userId);
    int ret = mysql_query(pconn, query);
    if(ret != 0){
        printf("(%d, %s)\n",mysql_errno(pconn), mysql_error(pconn));
        return NULL;
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
            strncpy(userName, row[0],buf_size - 1);
            userName[buf_size - 1] = '\0';
        }
    }
    mysql_free_result(result);
    printf("userName = %s\n",userName);
    return userName;
}

//将文件大小转换成以B,K,M,G为单位的
void format_file_size(char *dest, size_t dest_size, off_t size) {  
    const char *units[] = {"B", "K", "M", "G"};  
    unsigned long unit_index = 0;  
    double file_size_double = (double)size;  

    while (file_size_double >= 1024 && unit_index < (sizeof(units) / sizeof(units[0]) - 1)) {  
        file_size_double /= 1024;  
        unit_index++;  
    }  

    // 使用snprintf确保不会溢出  
    snprintf(dest, dest_size, "%.2f%s", file_size_double, units[unit_index]);  
}

extern user_info_t * user;
extern int parentId;
void llCommand(task_t * task)
{
    //extern ListNode * userList;
    //user_info_t * user = (user_info_t *)userList->val;

    int userId = 0; 
    userId= yu_userId(task->pconn, user, userId);
    int ownerId = userId;
    printf("ownerId = %d\n", ownerId);
    int fileId = 0;
    fileId = yu_fileId(task->pconn, parentId, ownerId, fileId);
    parentId = yu_parentId(task->pconn, fileId, ownerId, parentId);
    char userName[124];
    memset(userName, 0,sizeof(userName));
    yu_userName(task-> pconn, userId, userName, sizeof(userName));

    char sql[4096] = {0};
    //进行查询操作
    sprintf(sql,"select fileName, fileType, fileSize, modifyTime from files where parentId = %d and  ownerId = %d",parentId, ownerId);
    int ret = mysql_query(task->pconn, sql);
    if(ret != 0){
        printf("(%d, %s)\n",mysql_errno(task->pconn),mysql_error(task->pconn));
        return;
    }

    //查询成功的情况下，获取到结果集
    MYSQL_RES* result = mysql_store_result(task->pconn);
    if(result){
        //先获取属性列信息
        MYSQL_FIELD * pfield = mysql_fetch_fields(result);
        //获取结果集的行列信息
        int cols = mysql_num_fields(result);
        int rows = mysql_num_rows(result);
        printf("cols: %d, rows: %d\n",cols,rows);

        int maxWidth = 0;
        for (int i = 0; i < cols; i++) {
            int len = strlen(pfield[i].name);
            if (len > maxWidth) {
                maxWidth = len;
            }
        }
        //打印一行的属性列信息
        char buff[1024] = "";
        sprintf(buff,"user:%s\n",userName);
        printf("buff_now = %s\n",userName);

        for(int i = 0; i < cols; ++i){
            printf("%-12s",pfield[i].name);
            char bu[20] = "";
            //sprintf(bu, "%-*s", 12, pfield[i].name); // 使用-*来左对齐并填充空格  
            //sprintf(bu + strlen(bu), "%-*s", maxWidth, pfield[i].name); 
            sprintf(bu, "%-*s", maxWidth, pfield[i].name); 
            // 使用maxWidth进行左对齐（看起来像是右对齐，因为没有额外的空格） 
            strcat(bu, "  ");
            strcat(buff, bu);
        }
        printf("\n");
        strcat(buff, "\n");


        //打印具体的每一行数据
        MYSQL_ROW row;//MYSQL_ROW本质上是一个char**
        while((row = mysql_fetch_row(result)) != NULL){
            for(int i = 0; i < cols; i++){
                if(i != 2){
                    printf("%-12s", row[i]);
                    char cu[20] = "";
                    //sprintf(cu + strlen(cu), "%-*s", maxWidth, row[i]); 
                    sprintf(cu, "%-*s", maxWidth, row[i]); 
                    //sprintf(cu, "%-*s", 12, row[i]); // 使用-*来左对齐并填充空格  
                    strcat(cu, "  ");
                    strcat(buff, cu);
                }else{
                    char dest[20];
                    char *endptr; // 用于strtol的结束指针
                    int row_size = strtol(row[i], &endptr, 10); // 10表示基数为10的整数
                    format_file_size(dest, sizeof(dest), row_size);
                    printf("%-12s", row[i]);
                    char cu[20] = "";
                    //sprintf(cu + strlen(cu), "%-*s", maxWidth, row[i]); 
                    sprintf(cu, "%-*s", maxWidth, dest); 
                    //sprintf(cu, "%-*s", 12, row[i]); // 使用-*来左对齐并填充空格  
                    strcat(cu, "  ");
                    strcat(buff, cu);
                }
            }
            printf("\n");
            strcat(buff, "\n");
        }
        sendn(task->peerfd, buff, strlen(buff));
    }
    //释放结果集中的数据
    mysql_free_result(result);
}
