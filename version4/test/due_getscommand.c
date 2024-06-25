#include "client.h"
#include <func.h>

void due_getscommand(int clientfd, train_t t)
{
    printf("start downloading\n");
    char filename[50] = {0};
    int filename_len = 0;
    off_t file_len = 0;

    // recvn(clientfd,&filename_len,sizeof(filename_len));//获取文件名的长度
    // printf("filename_len: %d\n",filename_len);

    // recvn(clientfd,filename,filename_len);//获取文件名
    strcpy(filename, t.buff);
    printf("filename: %s\n", filename);

    // 断点重传
    int existFlag = 0;
    DIR *dir = opendir(".");
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, filename) == 0)
        {
            existFlag = 1;
        }
    }
    printf("existFlag: %d\n", existFlag);
    if (existFlag == 1)
    {
        sendn(clientfd, &existFlag, sizeof(existFlag));

        int fd = open(filename, O_CREAT | O_RDWR, 0666);

        struct stat st;
        fstat(fd, &st);
        off_t curr_filelen = 0;
        curr_filelen = st.st_size;
        sendn(clientfd, &curr_filelen, sizeof(curr_filelen));
        printf("curr_filelen: %ld\n", curr_filelen);

        char buff[1000] = {0};

        lseek(fd, curr_filelen, SEEK_SET);

        recv(clientfd, &file_len, sizeof(file_len), MSG_WAITALL);

        int ret = 0;
        int len=0;
        while (true)
        {
            memset(buff,0,sizeof(buff));
            ret = recv(clientfd,&len,sizeof(len),MSG_WAITALL);//先接长度
            if(len == 0) {
                break;
            }
            //可以确定接收len个字节的长度
            ret = recv(clientfd, buff, len,MSG_WAITALL);//再接文件内容
            if(ret != 1000) {
                printf("ret: %d\n", ret);
            }
            //最后再写入本地
            write(fd, buff, ret);     
                    
        }
    }
    // 不需要断点重传
    else
    {
        sendn(clientfd, &existFlag, sizeof(existFlag));
        int fd = open(filename, O_CREAT | O_RDWR | O_TRUNC, 0666);

        recvn(clientfd, (char *)&file_len, sizeof(file_len)); // 获取文件长度
        printf("mmap file length: %ld\n", file_len);
        ftruncate(fd, file_len);

        if (file_len >= 1024 * 1000000)
        {
            // 最后接收文件内容
            char *pMap = (char *)mmap(NULL, file_len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
            if (pMap)
            { // 映射成功
                // clientfd代表的是内核态套接字的缓冲区
                // pMap代表的是内核文件缓冲区
                int ret = recv(clientfd, pMap, file_len, MSG_WAITALL);
                printf("ret: %d\n", ret);
            }
        }
        else
        {
            char buff[1000] = {0};
            int ret = 0;
            int len=0;
            while (true)
            {
                memset(buff,0,sizeof(buff));
                ret = recv(clientfd,&len,sizeof(len),MSG_WAITALL);//先接长度
                if(len == 0) {
                    break;
                }
                //可以确定接收len个字节的长度
                ret = recv(clientfd, buff, len,MSG_WAITALL);//再接文件内容
                if(ret != 1000) {
                    printf("ret: %d\n", ret);
                }
                //最后再写入本地
                write(fd, buff, ret);     
                    
            }
        }
    }
    printf("download over\n");
}