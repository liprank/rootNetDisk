/*
 *@author cqy created
 *file ls.c
 *date 2024-6-19
 *@brief 接收到ls命令之后，打印当前目录的所有文件信息
 */
#include "../include/thread_pool.h"
#include "../include/user.h"

int yu_userId(MYSQL * pconn, user_info_t * user, int userId);
int yu_fileId(MYSQL * pconn, int parentId, int ownerId, int fileId);
int yu_parentId(MYSQL * pconn, int fileId, int ownerId, int parentId);

extern user_info_t * user;
extern int parentId;
void lsCommand(task_t * task)
{
    //extern ListNode * userList;
    //user_info_t * user = (user_info_t *)userList->val;

    int userId = 0; 
    userId= yu_userId(task->pconn, user, userId);
    int ownerId = userId;
    printf("ownerId = %d\n", ownerId);
    int fileId = 0;
    fileId = yu_fileId(task->pconn, parentId, ownerId, fileId);

    char sql[4096] = {0};
    //进行查询操作
    sprintf(sql,"select fileName from files where parentId = %d and  ownerId = %d",parentId, ownerId);
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

        //打印一行的属性列信息
        for(int i = 0; i < cols; ++i){
            printf("%s\t",pfield[i].name);
        }
        printf("\n");

        //打印具体的每一行数据
        MYSQL_ROW row;//MYSQL_ROW本质上是一个char**
        char cur[1024] = {0};

		int n = 0;
        while((row = mysql_fetch_row(result)) != NULL){
			n++;
            for(int i = 0; i < cols; ++i){
                printf("%s\t", row[i]);
                strcat(cur, row[i]);
            }
            printf("\n");
            strcat(cur, "\n");
        }
		if(n == 0){
			sprintf(cur,"当前目录为空\n");
		}
        sendn(task->peerfd, cur, strlen(cur));
    }
    //释放结果集中的数据
    mysql_free_result(result);
}
