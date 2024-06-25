/**  
 * @puts.c 
 * @author zzl  
 * @date 2024-6-17 
 * @brief 将客户端的本地文件上传到服务器（这里是接收客户端发送的文件）
 * @version 0.9
 */
#include "../include/thread_pool.h"

void putsCommand(task_t * task) {
    printf("executing puts command.\n");
    int peerfd=task->peerfd;
    char filename[50]={0};
    char local_filename[100]={0};
    off_t file_len=0;

    //先获取文件名
    strcpy(filename,task->data);

    if(strlen(filename)==0){
        printf("illegal filename\n");
        send(peerfd,"illegal filename\n",strlen("illegal filename\n"),0);
        return;
    }

    sprintf(local_filename,"%s/%s","../data",filename);
    int fd=open(local_filename,O_CREAT|O_RDWR|O_TRUNC,0666);
    if(fd==-1)error(1,errno,"open");

    //再接文件长度
    recvn(peerfd,(char*)&file_len,sizeof(file_len));
    
    printf("filelen=%ld",file_len);
    
    //再接收文件本身

    ftruncate(fd, file_len);

    //最后接收文件内容
    char * pMap = mmap(NULL, file_len, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    if(pMap) {//映射成功
        //clientfd代表的是内核态套接字的缓冲区
        //pMap代表的是内核文件缓冲区
        int ret = recv(peerfd, pMap, file_len, MSG_WAITALL);
        printf("ret: %d\n", ret);
    }

    close(fd);
    printf("Puts command over.\n");

    return;
}