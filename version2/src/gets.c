/**  
 * @puts.c 
 * @author zzl  
 * @date 2024-6-18
 * @brief 将服务器端的文件发送的客户端,断点续传，大文件传输
 * @version 2.1
 */

#include "../include/thread_pool.h"

void getsCommand(task_t * task) {
    printf("execute gets command.\n");
    char filename[50]={0};
    char local_filename[100]={0};
    int filename_len=0;
    int peerfd=task->peerfd;
    int existFlag=0;

    strcpy(filename,task->data);
    printf("%s\n",filename);

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

    recv(peerfd,&existFlag,sizeof(existFlag),MSG_WAITALL);//接收客户端发送的存在证明，1表示存在
    printf("existFlag: %d\n",existFlag);

    if(existFlag==1){
        //断点续传
        off_t curr_filelen=0;
        recv(peerfd,&curr_filelen,sizeof(curr_filelen),MSG_WAITALL);
        printf("curr_filelen: %ld\n",curr_filelen);
        
        struct stat st;
        fstat(fd,&st);
        sendn(peerfd, (char*)&st.st_size, sizeof(st.st_size));//发送文件长度

        lseek(fd,curr_filelen,SEEK_SET);//设置文件offset

        char buff[1000]={0};
        int len=0;
        int ret=0;
        while((ret=read(fd,buff,sizeof(buff)-1))>0){
            len=send(peerfd,buff,strlen(buff),0);
            printf("send %d bytes\n",len);
            memset(buff,0,sizeof(buff));
        }
        printf("send file over.\n");

    }else{
        //不用断点续传
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
        
        if(st.st_size>=1024*1000000){//大文件
            //最后发送文件内容
            int ret = sendfile(peerfd, fd, NULL, st.st_size);
            printf("send %d bytes.\n", ret);

        }else{
            //正常文件
            char buff[1000]={0};
            int len=0;
            int ret=0;
            while((ret=read(fd,buff,sizeof(buff)-1))>0){
                len=send(peerfd,buff,strlen(buff),0);
                printf("send %d bytes\n",len);
                memset(buff,0,sizeof(buff));
            }
            printf("send file over.\n");
        }
    }
    
    close(fd);//关闭文件
    printf("gets command over\n");
    return;
}
