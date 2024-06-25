/**
 * @puts.c
 * @author zzl
 * @date 2024-6-20
 * @brief 将服务器端的文件发送的客户端,断点续传，大文件传输
 * @version 3.1
 */

#include "../include/thread_pool.h"
#include "../include/linkedList.h"
#include "../include/user.h"
#include <time.h>

void getsCommand(task_t *task)
{
    printf("execute gets command.\n");

    typedef struct mytrain{
        int len;
        char buff[1000];
    } mytrain_t;

    char filename[50] = {0};
    char local_filename[100] = {0};
    int peerfd = task->peerfd;
    int existFlag = 0;
    int curr_userid = 0;
    char sha1_in_mysql[41] = {0};

    strcpy(filename, task->data);
    printf("%s\n", filename);

    MYSQL *pconn = task->pconn;

    extern user_info_t *user;
    printf("%s\n", user->name);

    char query0[100] = {0};
    sprintf(query0, "select userId from users where userName='%s'", user->name);
    int ret = mysql_query(pconn, query0);
    if (ret != 0)
    {
        printf("its me!query0!\n");
        printf("(%d,%s)\n", mysql_errno(pconn), mysql_error(pconn));
        return;
    }

    MYSQL_RES *result2 = mysql_use_result(pconn);
    if (result2)
    {
        MYSQL_ROW row;
        row = mysql_fetch_row(result2);
        curr_userid = atoi(row[0]); // 获取用户id
        printf("userid: %d\n", curr_userid);
    }

    mysql_free_result(result2);
    // debug here ok
    char query[200] = {0};
    sprintf(query, "select fileHash from files where ownerId='%d' and fileName='%s'", curr_userid, filename);
    ret = mysql_query(pconn, query);
    if (ret != 0)
    {
        printf("its me!query!\n");
        printf("(%d,%s)\n", mysql_errno(pconn), mysql_error(pconn));
        return;
    }

    MYSQL_RES *result = mysql_use_result(pconn);
    if (result)
    {
        MYSQL_ROW row;
        row = mysql_fetch_row(result);
        strcpy(sha1_in_mysql, row[0]);
        printf("%s\n", row[0]);
        printf("%s\n", sha1_in_mysql);
    }

    mysql_free_result(result);

    printf("%s\n", sha1_in_mysql);
    sprintf(local_filename, "%s/%s", "../data", sha1_in_mysql);
    int fd = open(local_filename, O_RDWR);
    if (fd == -1)
    {
        char errmesg[100] = {0};
        sprintf(errmesg, "Error opening file: %s", strerror(errno));
        printf("%s\n", errmesg);
    }

    recv(peerfd, &existFlag, sizeof(existFlag), MSG_WAITALL); // 接收客户端发送的存在证明，1表示存在
    printf("existFlag: %d\n", existFlag);

    if (existFlag == 1)
    {
        // 断点续传
        off_t curr_filelen = 0;
        recv(peerfd, &curr_filelen, sizeof(curr_filelen), MSG_WAITALL);
        printf("curr_filelen: %ld\n", curr_filelen);

        struct stat st;
        fstat(fd, &st);
        sendn(peerfd, (char *)&st.st_size, sizeof(st.st_size)); // 发送文件长度

        lseek(fd, curr_filelen, SEEK_SET); // 设置文件offset

        mytrain_t mt;
        while (1)
        {
            memset(&mt, 0, sizeof(mt));
            int ret = read(fd, mt.buff, sizeof(mt.buff));
            if (ret != 1000)
            {
                printf("read ret: %d\n", ret);
            }
            if (ret == 0)
            {
                // 文件已经读取完毕
                mt.len = ret;
                sendn(peerfd, (char *)&mt.len, sizeof(mt.len));
                break;
            }
            mt.len = ret;
            ret = sendn(peerfd, &mt, 4 + mt.len);
            if (ret == -1)
            {
                break;
            }
            if (ret != 1004)
            {
                printf("send ret: %d\n", ret);
            }
        }
        printf("send file over.\n");
    }
    else
    {
        // 不用断点续传

        // 发送文件长度
        struct stat st;
        memset(&st, 0, sizeof(st));
        fstat(fd, &st);
        printf("filelength: %ld\n", st.st_size); // off_t
        printf("sizeof(st.st_size): %ld\n", sizeof(st.st_size));
        sendn(peerfd, (char *)&st.st_size, sizeof(st.st_size));

        if (st.st_size >= 1024 * 1000000)
        { // 大文件
            // 最后发送文件内容
            int ret = sendfile(peerfd, fd, NULL, st.st_size);
            printf("send %d bytes.\n", ret);
        }
        else
        {
            // 正常文件
            mytrain_t mt;
            
            while (true)
            {
                memset(&mt, 0, sizeof(mt));

                int ret = read(fd, mt.buff, sizeof(mt.buff));
                if (ret != 1000){
                    printf("read ret: %d\n", ret);

                }
                if (ret == 0){
                    // 文件已经读取完毕
                    mt.len = ret;
                    sendn(peerfd, (char *)&mt.len, sizeof(mt.len));
                    break;

                }
                mt.len = ret;
                ret = sendn(peerfd, &mt, 4 + mt.len);
                if (ret == -1){
                    break;

                }
                if (ret != 1004){
                    printf("send ret: %d\n", ret);

                }
            }

            printf("send file over.\n");
        }
    }

    mysql_close(pconn);
    close(fd); // 关闭文件
    printf("gets command over\n");
    return;
}
