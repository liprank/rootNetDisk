#include "../include/thread_pool.h"

  
// 直接删除文件  
void deleteFile(const char* path, int peerfd) {  
    if (unlink(path) == -1) { 
        char err[100] = {0};
        sprintf(err, "删除文件 %s 失败！",path);
        printf("%s\n", err);
        send(peerfd, err, strlen(err), 0);
        return;
    }  
} 

// 递归删除目录
void deleteDir(const char* path, int peerfd) {
    DIR* pdir = opendir(path);
    if (pdir == NULL) { 
        char err[100] = {0};
        sprintf(err, "无法打开 %s ",path);
        printf("%s\n", err);
        send(peerfd, err, strlen(err), 0);
        return;
    }

    struct dirent* pdirent;
    errno = 0;
    while ((pdirent = readdir(pdir)) != NULL) {
        if (strcmp(pdirent->d_name, ".") == 0 || strcmp(pdirent->d_name, "..") == 0) {
            continue;
        }

        char subpath[1024];
        snprintf(subpath, sizeof(subpath), "%s/%s", path, pdirent->d_name);
        if (pdirent->d_type == DT_DIR) {
            deleteDir(subpath,peerfd);
        } else if (pdirent->d_type == DT_REG || pdirent->d_type == DT_LNK) {
            unlink(subpath);
        }
    }

    if (errno != 0) {
        char err[100] = {0};
        sprintf(err, "删除文件 %s 失败！",path);
        printf("%s\n", err);
        send(peerfd, err, strlen(err), 0);
    }

    closedir(pdir);

    // 尝试删除目录
    if (rmdir(path) == -1) {
        char err[100] = {0};
        sprintf(err, "删除文件 %s 失败！",path);
        printf("%s\n", err);
        send(peerfd, err, strlen(err), 0);
    }
}

// rmCommand 函数
void rmCommand(task_t *task) {
    if (task == NULL || task->type != CMD_TYPE_RM) {
       // fprintf(stderr, "目录不存在或指令无效！\n");
        char err[100] = "目录不存在或指令无效！";
        send(task->peerfd, err, strlen(err), 0);
        return;
    }

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

  
    struct stat st;  
    if (stat(task->data, &st) == -1) {
        char err[100] = {0};
        sprintf(err, "删除文件 %s 失败！",task->data);
        printf("%s\n", err);
        send(task->peerfd, err, strlen(err), 0);
        return;  
    } 
    if (S_ISDIR(st.st_mode)) {
        deleteDir(task->data, task->peerfd);
    } else {
        deleteFile(task->data, task->peerfd);
    }
}
