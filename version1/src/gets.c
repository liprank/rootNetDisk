/**  
 * @puts.c 
 * @author zzl  
 * @date 2024-6-17 
 * @brief 将服务器端的文件发送的客户端
 * @version 1.0
 */

#include "../include/thread_pool.h"

void getsCommand(task_t * task) {
    printf("execute gets command.\n");
    char filename[50]={0};
    char local_filename[100]={0};
    int filename_len=0;
    int peerfd=task->peerfd;

    strcpy(filename,task->data);
    if(strlen(filename)==0){
        printf("filename illegal\n");
        return;
    }
    filename_len=strlen(filename);

    sprintf(local_filename,"%s/%s","../data",filename);
    int fd = open(local_filename, O_RDWR);
    if(fd==-1){
        char errmesg[100]={0};
        sprintf(errmesg, "Error opening file: %s", strerror(errno));
        printf("%s\n",errmesg);
    }

    
    //先发送文件名
    
    sendn(peerfd, (char*)&filename_len,sizeof(filename_len));
    sendn(peerfd, (char*)&filename,strlen(filename));

    //发送文件长度
    struct stat st;
    memset(&st, 0, sizeof(st));
    fstat(fd, &st);
    printf("filelength: %ld\n", st.st_size);//off_t
    printf("sizeof(st.st_size): %ld\n", sizeof(st.st_size));
    sendn(peerfd, (char*)&st.st_size, sizeof(st.st_size));
    
    //最后发送文件内容
    int ret = sendfile(peerfd, fd, NULL, st.st_size);
    printf("send %d bytes.\n", ret);
    close(fd);//关闭文件

    return;
}
