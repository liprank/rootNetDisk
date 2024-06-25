#include "../include/thread_pool.h"

// 检查目录是否为空  
int isDirEmpty(const char *path) {  
    DIR *dir = opendir(path); 
    // 打开目录失败，可能不是目录或没有权限
    if (dir == NULL) {    
        return -1;  
    }  
  
    struct dirent *entry;
    // 发现了其他文件或目录，因此目录不为空
    while ((entry = readdir(dir)) != NULL) {  
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {  
            continue;  
        }    
        closedir(dir);  
        return 0;  
    }  
  
    closedir(dir);

    // 目录为空  
    return 1;  
} 

// 删除空目录  
void deleteEmptyDir(const char *path, int peerfd) {
    if (isDirEmpty(path) == 1) {  //目录为空，删除
        if (rmdir(path) == -1) { 
            char err[100] = {0};
        sprintf(err, "删除 %s 失败！",path);
        printf("%s\n", err);
        send(peerfd, err, strlen(err), 0);
        }  
    } else if (isDirEmpty(path) == -1) {  //打开目录失败
        char err[100] = {0};
        sprintf(err, "无法打开 %s ,检查是否为空目录！",path);
        printf("%s\n", err);
        send(peerfd, err, strlen(err), 0);
    } else if (isDirEmpty(path) == 0) {   //不是空目录 
        char err[100] = {0};
        sprintf(err, " %s 不是空目录，无法删除！",path);
        printf("%s\n", err);
        send(peerfd, err, strlen(err), 0);
    }
} 

// rmdirCommand 函数
void rmdirCommand(task_t *task) {
    if (task == NULL || task->type != CMD_TYPE_RMDIR) {
        char err[100] = {0};
        sprintf(err, "无法打开 %s ",task->data);
        printf("%s\n", err);
        send(task->peerfd, err, strlen(err), 0);
        return;
    }

    // 确保data字段包含有效的目录路径
    if (strlen(task->data) == 0) {
        char err[100] = {0};
        sprintf(err, "无法打开 %s ",task->data);
        printf("%s\n", err);
        send(task->peerfd, err, strlen(err), 0);
        return;
    }

    // 检查是否为根目录
    if (strcmp(task->data, "/") == 0) {
        char err[100] = {0};
        sprintf(err, "无法打开 %s ",task->data);
        printf("%s\n", err);
        send(task->peerfd, err, strlen(err), 0);
        return;
    }

    // 尝试删除空目录  
    deleteEmptyDir(task->data,task->peerfd);
}
