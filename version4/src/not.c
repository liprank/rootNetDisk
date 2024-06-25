#include "../include/thread_pool.h"

#define MAXLINE 64

void notCommand(task_t* task){
    char buff[MAXLINE] = {0};

    int len = 0;
    sprintf(buff,"Sorry, this one no invaild command.\n");
    printf("%s", buff);
    len = strlen(buff);

    //send(task->peerfd,&len,sizeof(len),0);
    sendn(task->peerfd,buff,len);
}

