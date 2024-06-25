#include "client.h"
#include <func.h>

void due_putscommand(int clientfd,train_t t){

	typedef struct mytrain{
        int len;
        char buff[1000];
    } mytrain_t;

	char filename[50] = {0};
	int existFlag = 0;
	int sha1_existFlag = 0;
	char file_sha1[41] = {0};

	strcpy(filename, t.buff);
	int fd = open(filename, O_RDWR);

	generate_sha1(fd, file_sha1);//因为里面读了，所以要设置回开头
	sendn(clientfd, file_sha1, 40); // 发送sha1码

	lseek(fd,0,SEEK_SET);

	struct stat st;
	bzero(&st, sizeof(st));
	fstat(fd, &st);

	sendn(clientfd, (char *)&st.st_size, sizeof(st.st_size)); // 发送文件长度

	recvn(clientfd, (char *)&sha1_existFlag, sizeof(sha1_existFlag)); // 收取sha1_existFlag
	printf("sha1_existFlag: %d\n",sha1_existFlag);

	if (sha1_existFlag == 1)
	{
		recvn(clientfd, (char *)&existFlag, sizeof(existFlag)); // 收取existFlag
		if (existFlag == 0)
		{
			printf("puts command over, do not have to upload\n");
			return;
		}
		else if (existFlag == 1)
		{
			// 断点续传
			off_t curr_filelen = 0;
			recv(clientfd, &curr_filelen, sizeof(curr_filelen), MSG_WAITALL); // 接收服务器上已存在文件大小

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
					sendn(clientfd, (char *)&mt.len, sizeof(mt.len));
					break;
				}
				mt.len = ret;
				ret = sendn(clientfd, &mt, 4 + mt.len);
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
	}
	else if (sha1_existFlag == 0)
	{
		if (st.st_size >= 1024 * 1000000)
		{ // 大文件
			// 最后发送文件内容
			int ret = sendfile(clientfd, fd, NULL, st.st_size);
			printf("send %d bytes.\n", ret);
		}else{

			//printf("normal upload\n");
			mytrain_t mt;
            
            while (1){
                memset(&mt, 0, sizeof(mt));
                int ret = read(fd, mt.buff, sizeof(mt.buff));
				printf("read %d bytes\n",ret);
                if (ret != 1000){
                    printf("read ret: %d\n", ret);
                }

                if (ret == 0){
                    // 文件已经读取完毕
                    mt.len = ret;
                    sendn(clientfd, (char *)&mt.len, sizeof(mt.len));
                    break;
                }

                mt.len = ret;
                ret = sendn(clientfd, &mt, 4 + mt.len);
				printf("send %d bytes\n",ret);
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
	close(fd);
	printf("puts command over\n");

}
