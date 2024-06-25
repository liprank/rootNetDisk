#include "../include/thread_pool.h"
/**
 * @ file pwd.c
 * @ author zpw created
 * @ data 2024-06-15
 * @ biref 显示目前所在的路径
 */

// 1.0
////  函数参数：路径存放的buff 长度
//int pwd(char* buff, size_t size){
//    if(getcwd(buff, size) == NULL)   // 直接在buff中调用getcwd
//    {
//        ERROR_CHECK(-1, errno, "getcwd");
//    }
//    
//    printf("current working directory is %s\n", buff);
//    return 0;
//}

void pwdCommand(task_t * task){
    char* buff = getcwd(NULL, 0);
    if(buff == NULL){
        strcpy(buff, "路径异常");
    }

    sendn(task->peerfd, buff, strlen(buff));

    printf("%s\n", buff);

    fflush(stderr);
}

