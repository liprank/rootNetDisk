#include "../include/thread_pool.h"
/**
 * @ cat.c
 * @ author zpw create
 * @ date 6-17
 * @ 实现catCommand接口
 * @ todo 实现对目录和文件的判断处理 
 * @ 6.18以实现
 */ 

#define MAXLINE 4096

void catCommand(task_t *task){
    int fd = open(task->data, O_RDONLY);

    struct stat st;
    if (fstat(fd, &st) == -1 || fd == -1) { // 使用 fstat 获取文件状态  
        char err[MAXLINE] = "ERROR: Please enter the correct file name";
        send(task->peerfd, err, strlen(err), 0);
        printf("%s", err);
        // perror("fstat");  
    } 
    
    // 检查是否为目录  
    if (S_ISDIR(st.st_mode)) {  
        // fprintf(stderr, "%s is a directory, not a file.\n",task->data);  
        char err[64] = {0};
        sprintf(err, "%s is a directory, not a file.", task->data);
        printf("%s\n", err);
        send(task->peerfd, err, strlen(err), 0);
        // exit(EXIT_FAILURE);  
    } 

    //if (fd == -1) {
    //    // perror("Error");
    //    char err[MAXLINE] = "ERROR: Please enter the correct file name";
    //    send(task->peerfd, err, strlen(err), 0);
    //    printf("%s", err);
    //    //exit(EXIT_FAILURE);
    //}

    char buffer[MAXLINE] = {0};
    size_t bytes_read;
    while ((bytes_read = read(fd, buffer, MAXLINE)) > 0) {
        printf("%s", buffer);
        send(task->peerfd, buffer, strlen(buffer), 0);
    }

    //if (bytes_read == -1) {
    //    perror("Error");
    //    char err[100] = "请输入文件";
    //    sendn(task->peerfd, err, strlen(err));
    //    printf("%s", err);
    //}

    close(fd);
}
