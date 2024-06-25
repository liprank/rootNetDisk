/*
 * @puts.c 
 * @author zzl  
 * @date 2024-6-20 
 * @brief 将客户端的本地文件上传到服务器（这里是接收客户端发送的文件）,文件秒传，断点续传，大文件传输
 * @version 3.1
 */
#include "../include/thread_pool.h"
#include "../include/linkedList.h"
#include "../include/user.h"
#include<time.h>

void putsCommand(task_t * task) {
    printf("executing puts command.\n");
    int peerfd=task->peerfd;
    char filename[50]={0};
    char file_sha1[41]={0};//留着接收客户端发送的sha1码
    off_t file_len=0;
    int existFlag=0;
    int sha1_existFlag=0;
    int curr_dir=0;
    int curr_userid=0;

    strcpy(filename,task->data); //先获取文件名

    recvn(peerfd,file_sha1,40);//接收sha1码

    recvn(peerfd,(char*)&file_len,sizeof(file_len));//接收文件长度
    printf("file_len: %ld\n",file_len);

    //建立数据库连接，与sha1_of_files数据库中的sha1码比较
    MYSQL* pconn=task->pconn;

    const char* query="select sha1 from sha1_of_files";
    int ret=mysql_query(pconn,query);
    if(ret!=0){
        printf("(%d,%s)\n",mysql_errno(pconn),mysql_error(pconn));
        return;
    }

    MYSQL_RES *result=mysql_use_result(pconn);
    if(result){
        MYSQL_ROW row;
        char sha1_in_mysql[41]={0};
        while((row=mysql_fetch_row(result))!=NULL){
            snprintf(sha1_in_mysql,40,"%s",row[0]);
            if(strcmp(sha1_in_mysql,file_sha1)==0){//判断sha1码是否存在，设置existFlag
                sha1_existFlag=1;
                break;
            }
            memset(sha1_in_mysql,0,sizeof(sha1_in_mysql));
        }
    }

    mysql_free_result(result);

    extern ListNode * userList;
    user_info_t * user = (user_info_t *)userList->val;   

    char query0[100]={0};
    sprintf(query0,"select userId,curDirectory from users where userName='%s'",user->name);
    ret=mysql_query(pconn,query0);
    if(ret!=0){
        printf("(%d,%s)\n",mysql_errno(pconn),mysql_error(pconn));
        return;
    }

    MYSQL_RES *result2=mysql_use_result(pconn);
    if(result2){
        MYSQL_ROW row;
        row=mysql_fetch_row(result2);
        curr_dir=atoi(row[1]);
        curr_userid=atoi(row[0]);
    }

    mysql_free_result(result2);

//创建stmt
    MYSQL_STMT *stmt=mysql_stmt_init(pconn);
    if(stmt==NULL){
        printf("error:%s\n",mysql_error(pconn));
        mysql_close(pconn);
        return;
    }

    const char* sql2="INSERT INTO files(parentId,ownerId,fileName,fileType,fileSize,fileHash,modifyTime) VALUES(?,?,?,?,?,?,?)";
    ret=mysql_stmt_prepare(stmt,sql2,strlen(sql2));
    if(ret){
        printf("(%d,%s)\n",mysql_stmt_errno(stmt),mysql_stmt_error(stmt));
        mysql_close(pconn);
        return;
    }

    int parentId=0;
    int ownerId=0;
    char fileName[20]={0};
    char fileType=0;
    off_t fileSize=0;
    char fileHash[40]={0};

    unsigned long fileName_len=0;
    unsigned long fileHash_len=0;
    unsigned long fileType_len=0;
    

    MYSQL_TIME time_value;
    struct tm tm_time={0};

    time_t rawtime;
    time(&rawtime);
    localtime_r(&rawtime,&tm_time);

    time_value.year=0;
    time_value.month=0;
    time_value.day=0;
    time_value.hour=tm_time.tm_hour;
    time_value.minute=tm_time.tm_min;
    time_value.second=tm_time.tm_sec;
    time_value.neg=0;

    MYSQL_BIND bind[7];
    memset(bind,0,sizeof(bind));
    bind[0].buffer_type=MYSQL_TYPE_LONG;
    bind[0].buffer=&parentId;
    bind[0].is_null=0;
    bind[0].length=NULL;

    bind[1].buffer_type=MYSQL_TYPE_LONG;
    bind[1].buffer=&ownerId;
    bind[1].is_null=0;
    bind[1].length=NULL;

    bind[2].buffer_type=MYSQL_TYPE_VAR_STRING;
    bind[2].buffer=fileName;
    bind[2].is_null=0;
    bind[2].length=&fileName_len;

    bind[3].buffer_type=MYSQL_TYPE_STRING;
    bind[3].buffer=&fileType;
    bind[3].is_null=0;
    bind[3].length=&fileType_len;

    bind[4].buffer_type=MYSQL_TYPE_LONG;
    bind[4].buffer=&fileSize;
    bind[4].is_null=0;
    bind[4].length=NULL;

    bind[5].buffer_type=MYSQL_TYPE_VAR_STRING;
    bind[5].buffer=fileHash;
    bind[5].is_null=0;
    bind[5].length=&fileHash_len;

    bind[6].buffer_type = MYSQL_TYPE_TIME;  
    bind[6].buffer = (char *)&time_value;  
    bind[6].is_null = 0;  
    bind[6].length = 0; // 对于TIME，这个通常被忽略  

    ret=mysql_stmt_bind_param(stmt,bind);
    if(ret){
        printf("(%d,%s)\n",mysql_stmt_errno(stmt),mysql_stmt_error(stmt));
        mysql_close(pconn);
        return;
    }

    parentId=curr_dir;
    ownerId=curr_userid;
    strcpy(fileName,filename);
    fileName_len=strlen(fileName);
    //fileType='f';
    printf("%c\n",fileType);
    fileSize=file_len;
    strcpy(fileHash,file_sha1);
    fileHash_len=strlen(file_sha1);

    fileType_len=sizeof(fileType);

    ret=mysql_stmt_execute(stmt);
    if(ret){
        printf("(%d,%s)\n",mysql_stmt_errno(stmt),mysql_stmt_error(stmt));
        mysql_close(pconn);
        return;
    }

    printf("QUERY OK,%ld row affected.\n",mysql_stmt_affected_rows(stmt));

    mysql_stmt_close(stmt);

    //如果存在则仅添加files表项（秒传）,判断是否需要断点续传
    if(sha1_existFlag==1){
        sendn(peerfd,(char*)&sha1_existFlag,sizeof(sha1_existFlag));

        char local_filename[60]={0};
        strcpy(local_filename,file_sha1);
        sprintf(local_filename,"%s/%s","../data",filename);

        int fd=open(local_filename,O_RDWR);

        struct stat st;
		fstat(fd,&st);
		off_t curr_filelen = 0;
		curr_filelen = st.st_size;		
        
		printf("curr_filelen: %ld\n",curr_filelen);

        if(fd>0&&curr_filelen<file_len){
            existFlag=1;
            sendn(peerfd,(char*)&existFlag,sizeof(existFlag));//发送存在证明，断点续传
            sendn(peerfd,&curr_filelen,sizeof(curr_filelen));

			char buff[1000] = {0};
					
			lseek(fd,curr_filelen,SEEK_SET);

            int ret = 0;
            int len=0;
            while (true)
            {
                memset(buff,0,sizeof(buff));
                ret = recv(peerfd,&len,sizeof(len),MSG_WAITALL);//先接长度
                if(len == 0) {
                    break;
                }
                //可以确定接收len个字节的长度
                ret = recv(peerfd, buff, len,MSG_WAITALL);//再接文件内容
                if(ret != 1000) {
                    printf("ret: %d\n", ret);
                }
                //最后再写入本地
                write(fd, buff, ret);     
                        
            }


        }else{
            sendn(peerfd,(char*)&existFlag,sizeof(existFlag));
        }
        close(fd);

    }else{
        //如果不存在则添加sha1_of_files表项与files表项，并在server本地新建以sha1码为文件名的文件

        MYSQL_STMT *stmt2=mysql_stmt_init(pconn);
        if(stmt==NULL){
            printf("error:%s\n",mysql_error(pconn));
            mysql_close(pconn);
            return;
        }

        const char* sql3="INSERT INTO sha1_of_files(sha1) VALUES(?)";
        ret=mysql_stmt_prepare(stmt2,sql3,strlen(sql3));
        if(ret){
            printf("(%d,%s)\n",mysql_stmt_errno(stmt2),mysql_stmt_error(stmt2));
            mysql_close(pconn);
            return;
        }

        MYSQL_BIND bind2[1];
        memset(bind2,0,sizeof(bind2));
        bind2[0].buffer_type=MYSQL_TYPE_VAR_STRING;
        bind2[0].buffer=fileHash;
        bind2[0].is_null=0;
        bind2[0].length=&fileHash_len;

        //printf("running here\n");
        ret=mysql_stmt_bind_param(stmt2,bind2);
        if(ret){
            printf("(%d,%s)\n",mysql_stmt_errno(stmt2),mysql_stmt_error(stmt2));
            mysql_close(pconn);
            return;
        }

        strcpy(fileHash,file_sha1);
        fileHash_len=strlen(file_sha1);

        //printf("%s\n",fileHash);
        //printf("filehash_len: %lu\n",fileHash_len);

        ret=mysql_stmt_execute(stmt2);
        if(ret){
            printf("(%d,%s)\n",mysql_stmt_errno(stmt2),mysql_stmt_error(stmt2));
            mysql_close(pconn);
            return;
        }

        printf("QUERY OK,%ld row affected.\n",mysql_stmt_affected_rows(stmt2));

        mysql_stmt_close(stmt2);

        sendn(peerfd,(char*)&sha1_existFlag,sizeof(sha1_existFlag));//发送sha1存在证明，这里为0

        char local_filename[60]={0};
        sprintf(local_filename,"%s/%s","../data",file_sha1);
        int fd=open(local_filename,O_RDWR|O_CREAT,0666);

        if(file_len>=1024*1000000){
             //再接收文件本身
            ftruncate(fd, file_len);
            //最后接收文件内容
            char * pMap = (char*)mmap(NULL, file_len, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);

            if(pMap) {//映射成功
                //clientfd代表的是内核态套接字的缓冲区
                //pMap代表的是内核文件缓冲区
                int ret = recv(peerfd, pMap, file_len, MSG_WAITALL);
                printf("ret: %d\n", ret);

            }
        }else{
            char buff[1000] = {0};
            int ret = 0;
            int len=0;
            while (true)
            {
                memset(buff,0,sizeof(buff));
                ret = recv(peerfd,&len,sizeof(len),MSG_WAITALL);//先接长度
                if(len == 0) {
                    break;
                }
                //可以确定接收len个字节的长度
                ret = recv(peerfd, buff, len,MSG_WAITALL);//再接文件内容
                if(ret != 1000) {
                    printf("ret: %d\n", ret);
                }
                //最后再写入本地
                write(fd, buff, ret);     
                    
            }
            close(fd);
            
        }      
    }   

    printf("Puts command over.\n");

    return;
}
