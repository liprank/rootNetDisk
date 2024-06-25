#include "../include/thread_pool.h"
//输入格式：XX/XX/XX或XX/XX/XX/
//如果此目录文件已有，则不创建并提醒改名
//如果此目录文件没有，则创建，并提示创建成功
void mkdirCommand(task_t * task)
{
    char str[256]="";
    int ret= mkdir(task->data,0777);
    if(ret){//失败如何
        sprintf(str,"mkdirCommand-mkdir failed:%s",strerror(errno));
        printf("%s\n",str);
        sendn(task->peerfd,str,strlen(str)); 
        return;
    }

    sprintf(str,"Directory create successfully!");
    printf("%s\n",str);
    sendn(task->peerfd,str,strlen(str));
    return;
}
