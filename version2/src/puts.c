/**  
 * @puts.c 
 * @author zzl  
 * @date 2024-6-18 
 * @brief 将客户端的本地文件上传到服务器（这里是接收客户端发送的文件）
 * @version 2.1
 */
#include "../include/thread_pool.h"

void putsCommand(task_t * task) {
    printf("executing puts command.\n");
    int peerfd=task->peerfd;
    char filename[50]={0};
    char local_filename[100]={0};
    off_t file_len=0;
    int existFlag=0;

    //先获取文件名
    strcpy(filename,task->data);

    //if(strlen(filename)==0){
    //    printf("illegal filename\n");
    //    send(peerfd,"illegal filename\n",strlen("illegal filename\n"),0);
    //    return;
    //}

    sprintf(local_filename,"%s/%s","../data",filename);
    int fd=open(local_filename,O_RDWR);
    if(fd==-1&&errno==ENOENT){//如果文件不存在，要求客户端从头开始传
        sendn(peerfd,(char*)&existFlag,sizeof(existFlag));//发送existFlag,0代表不存在

        fd=open(local_filename,O_RDWR|O_CREAT,0666);
        //再接文件长度
        recvn(peerfd,(char*)&file_len,sizeof(file_len));
        
        printf("filelen=%ld\n",file_len);

        if(file_len>=1024*1000000){
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
        }else{
            char buff[1000] = {0};
            off_t currSize = 0;
            off_t left = file_len;
            int ret=0;
            while(left > 0) {
                left = file_len - currSize;
                if(left >= 1000) {
                    ret = recvn(peerfd, buff, sizeof(buff));//再接文件内容
                } else if(left < 1000 && left > 0){
                    ret = recvn(peerfd, buff, left);
                } else {
                    break;
                }
                //最后再写入本地
                ret = write(fd, buff, ret);
                currSize += ret;
            }
        }      
    }else if(fd>0){
        existFlag=1;
        sendn(peerfd,(char*)&existFlag,sizeof(existFlag));//服务器上文件已存在

        struct stat st;
        fstat(fd,&st);
        off_t curr_filelen=st.st_size;

        sendn(peerfd,(char*)&curr_filelen,sizeof(curr_filelen));//发送当前文件长度

        recvn(peerfd,(char*)&file_len,sizeof(file_len));//接收文件长度（全部）
        printf("filelen=%ld\n",file_len);

        ftruncate(fd, file_len);
        lseek(fd,curr_filelen,SEEK_SET);

        //循环接收
        char buff[1000] = {0};
        off_t currSize = 0;
        off_t left = file_len;
        int ret=0;
        while(left > 0) {
            left = file_len - currSize;
            if(left >= 1000) {
                ret = recvn(peerfd, buff, sizeof(buff));//再接文件内容
            } else if(left < 1000 && left > 0){
                ret = recvn(peerfd, buff, left);
            } else {
                break;
            }
            //最后再写入本地
            ret = write(fd, buff, ret);
            currSize += ret;
        }
    }

    

    close(fd);
    printf("Puts command over.\n");

    return;
}