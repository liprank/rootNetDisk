#include "../include/config.h"
#include "../include/thread_pool.h"
#include "../include/linkedList.h"
#include "../include/user.h"
#include "../include/loop_queue.h"
#include <time.h>

#define EPOLL_ARR_SIZE 100
pthread_mutex_t dblock;

int exitPipe[2];

//定义存储用户信息的链表
ListNode * userList = NULL;

user_info_t* user;//存储当前用户信息

int parentId = 0;//当前所在目录

void sigHandler(int num)
{
    printf("\n sig is coming.\n");
    //激活管道, 往管道中写一个1
    int one = 1;
    write(exitPipe[1], &one, sizeof(one));
}

int main(int argc, char ** argv)
{   //ip,port,threadNum
    printf("sizeof(CmdType):%lu\n", sizeof(CmdType));
    ARGS_CHECK(argc, 2);
    //创建匿名管道
    pipe(exitPipe);

    //fork之后，将创建了子进程
    pid_t pid = fork();
    if(pid > 0) {//父进程
        close(exitPipe[0]);//父进程关闭读端
        signal(SIGUSR1, sigHandler);
        wait(NULL);//等待子进程退出，回收其资源
        close(exitPipe[1]);
        printf("\nparent process exit.\n");
		pthread_mutex_destroy(&dblock);//删除数据库锁
        exit(0);//父进程退出
    }
    //子进程
    close(exitPipe[1]);//子进程关闭写端

	//增加配置文件
	HashTable ht;
	initHashTable(&ht);
	readConfig(argv[1],&ht);

    threadpool_t threadpool;
    memset(&threadpool, 0, sizeof(threadpool));

    //初始化线程池
    //threadpoolInit(&threadpool, atoi(argv[3]));

	pthread_mutex_init(&dblock, NULL);//初始化数据库锁

	threadpoolInit(&threadpool, atoi((const char*)find(&ht, THREAD_NUM)));	

    //启动线程池
    threadpoolStart(&threadpool);

    //创建监听套接字
    //int listenfd = tcpInit(argv[1], argv[2]);
    int listenfd = tcpInit((const char*)find(&ht,IP), (const char *)find(&ht,PORT));

    //创建epoll实例
    int epfd = epoll_create1(0);
    ERROR_CHECK(epfd, -1, "epoll_create1");

    //对listenfd进行监听
    addEpollReadfd(epfd, listenfd);
    addEpollReadfd(epfd, exitPipe[0]);

    struct epoll_event * pEventArr = (struct epoll_event*)
        calloc(EPOLL_ARR_SIZE, sizeof(struct epoll_event));

	queue_t* q = qInit();//建立循环队列
	int fdBuff[MAX_LEN] = {0}; 
	int length = 0;//记录fdBuff的长度

	qHashTable queueHt;//用于查找文件描述符上一次就绪的位置
	qinitHashTable(&queueHt);

	//time_t beginTime = 0;//开始时间
	//time_t curTime = 0;//现在的时间
	//int flag = 1;//确认是否是连接刚开始

    while(1) {
        int nready = epoll_wait(epfd, pEventArr, EPOLL_ARR_SIZE, 1000);//超时断开连接
        if(nready == -1 && errno == EINTR) {
            continue;
        } else if(nready == -1) {
            ERROR_CHECK(nready, -1, "epoll_wait");
        } else if(nready == 0){//超过1秒，没有事件就绪

			if(isQueueFull(q) == true){
				//获取当前超过30秒不活跃的文件描述符
				int* fdDeleteList = frontCheck(q);
				int deleteListLen = q->list[q->front].length;
				if(fdDeleteList != NULL){
					for(int i = 0; i < deleteListLen; i++){
						if(fdDeleteList[i] != 0){  //因为hash表清除的时候，是将之前出现过的值设为0
							int fdToDelete = fdDeleteList[i];
							//printf("超过30秒断开连接\n");
							sendn(fdToDelete,"exit",4);
							close(fdToDelete);
							epoll_ctl(epfd, EPOLL_CTL_DEL,fdToDelete,pEventArr);
						}
					}	
				}
				//出队
				deQueue(q);	
			}
			enQueue(q,NULL,0);
		}
		else {//在1秒内就绪的事件
            //大于0
            for(int i = 0; i < nready; ++i) {
                int fd = pEventArr[i].data.fd;
                if(fd == listenfd) {//对新连接进行处理
                    int peerfd = accept(listenfd, NULL, NULL);
                    //记录客户端连接信息
                    log_client_connection(peerfd);

                    printf("\n conn %d has conneted.\n", peerfd);
                    //将新连接添加到epoll的监听红黑树上
                    addEpollReadfd(epfd, peerfd);

					//添加用户节点
                    user = (user_info_t*)calloc(1, sizeof(user_info_t));
                    user->sockfd = peerfd;

					printf("sockfd:%d\n",user->sockfd);
                    appendNode(&userList, user);

                } else if(fd == exitPipe[0]) {
                    //线程池要退出
                    int howmany = 0;
                    //对管道进行处理
                    read(exitPipe[0], &howmany, sizeof(howmany));
                    //主线程通知所有的子线程退出
                    threadpoolStop(&threadpool);
                    //子进程退出前，回收资源
                    threadpoolDestroy(&threadpool);
                    close(listenfd);
                    close(epfd);
                    close(exitPipe[0]);
                    printf("\nchild process exit.\n");
                    exit(0);
                } else {//客户端的连接的处理
                   handleMessage(fd, epfd, &threadpool.que);
                }

/*************************超时断开连接********************************************/
				//客户端发送命令，表示活跃，加入循环链表中的集合当中去
				//获取当前时间
				bzero(fdBuff,MAX_LEN);
				for(int i = 0; i < nready; i++){//将就绪的文件描述符写入缓存
					//活跃的文件描述符最近放入的位置，存入hashtable
					int rear = q->rear;

					//在插入之前，先在hashtable中查找文件描述符是否已经存在
qprintHashTable(&queueHt);
					void* foundValue = qfind(&queueHt,fd);
					int position;
					if(foundValue != NULL){
						position = *(int*)foundValue;	
						clearValue(q,position,fd);	
						qerase(&queueHt,fd);

						printQueue(q);
					}

printf("要插入的文件描述符：%d\n",fd);
					qinsert(&queueHt,fd,rear);

qprintHashTable(&queueHt);
printQueue(q);

					if(fd != fdBuff[i] && fdBuff[i] == 0){
						fdBuff[i] = fd;
						length++;
					}
				}
	
				if(isQueueFull(q) == true){
					//获取当前超过30秒不活跃的文件描述符
					int* fdDeleteList = frontCheck(q);
					int deleteListLen = q->list[q->front].length;
					deQueue(q);
printf("deletelistlen1111: %d\n",deleteListLen);
					if(fdDeleteList != NULL){
						for(int i = 0; i < deleteListLen; i++){
							if(fdDeleteList[i] != 0){
								int fdToDelete = fdDeleteList[i];
								close(fdToDelete);
								epoll_ctl(epfd, EPOLL_CTL_DEL,fdToDelete,pEventArr);
							}
						}	
					}
				}

				
				enQueue(q,fdBuff,length);	
				length = 0;//更新fdBff的值
				
printf("the sizeof queue:%d\n",q->size);
/**************************************超时重传******************************/
			}
		} 
	}
	return 0;
}

