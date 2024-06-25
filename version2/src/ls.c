/*
 *@author cqy created
 *file ls.c
 *data 2024-6-16
 *@brief 接收到ls命令之后，打印当前目录的所有文件信息
 */
#include "../include/thread_pool.h"

void lsCommand(task_t * task)
{
    //打开当前目录
    DIR*dir = opendir(".");
    if(dir == NULL){
        perror("opendir");
        exit(0);
    }

    //打印当前目录的信息
    struct dirent * entry;
    char buff[1024] = {0};
    while((entry = readdir(dir)) != NULL){

        if(entry->d_name[0]=='.'){
            continue;
        }
        printf("%s\n",entry->d_name);
        //将文件的信息拼接给buff
        strcat(buff, entry->d_name);
        strcat(buff, "\n");
    }
    
    sendn(task->peerfd, buff, strlen(buff));
    fflush(stderr);
    //关闭目录
    closedir(dir);
}


