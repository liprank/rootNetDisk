#include "../include/thread_pool.h"
#include "../include/linkedList.h"
#include "../include/user.h"

void* start_routine(void* args){
    int listenfd=tcpInit("127.0.0.1","8080");

    int epfd = epoll_create1(0);
    ERROR_CHECK(epfd, -1, "epoll_create1");

    //对listenfd进行监听
    addEpollReadfd(epfd, listenfd);

    extern ListNode * userList;

    struct epoll_event * pEventArr = (struct epoll_event*)
        calloc(100, sizeof(struct epoll_event));

    while(true){
        int nready = epoll_wait(epfd, pEventArr, 100, -1);
        if(nready == -1 && errno == EINTR) {
            continue;
        } else if(nready == -1) {
            ERROR_CHECK(nready, -1, "epoll_wait");
        } else {
            //大于0
            for(int i = 0; i < nready; ++i) {
                int fd = pEventArr[i].data.fd;
                if(fd == listenfd) {//
                    extern info io;

                    int peerfd = accept(listenfd, NULL, NULL);
                    log_client_connection(peerfd);
                    printf("\n conn %d has conneted.\n", peerfd);

                    addEpollReadfd(epfd, peerfd);

                    user_info_t* user = (user_info_t*)calloc(1, sizeof(user_info_t));
                    user->sockfd = peerfd;

					printf("sockfd:%d\n",user->sockfd);
                    appendNode(&userList, user);

                }else{
                    int length = -1;
                    int ret = recvn(fd, &length, sizeof(length));
                    printf("recv length: %d\n", length);

                    //1.2 获取消息类型
                    int cmdType = -1;
                    ret = recvn(fd, &cmdType, sizeof(cmdType));
                    printf("recv cmd type: %d\n", cmdType);

                    //char username[20]={0};
                    //char token[300]={0};
                    //char buff[300]={0};
                    //recvn(fd,username,sizeof(username));
                    //recvn(fd,token,sizeof(token));
                    //jwtEncode(username,buff);
                    //if(strcmp(token,buff)!=0){
                     //   break;
                    //}
                    extern info io;

                    task_t *ptask = (task_t*)calloc(1, sizeof(task_t));
                    ptask->peerfd = fd;
                    ptask->epfd=epfd;
                    ptask->type= (CmdType)cmdType;
                    strcpy(ptask->data,io.t.buff);

                    MYSQL* conn = (MYSQL*)malloc(sizeof(MYSQL));//在堆空间中申请pconn，才不会受到栈空间的影响
                    MYSQL* pconn = mysql_init(conn);

                //可使用配置文件
                    pconn = mysql_real_connect(pconn,"localhost","root","1234","rootNetDisk",0,NULL,0);
                    if(pconn == NULL){
                        printf("%s\n",mysql_error(conn));
                        exit(0);
                    }
                    ptask->pconn=pconn;


                    if(cmdType==CMD_TYPE_GETS){
                        delEpollReadfd(epfd, fd);
                        getsCommand(ptask);  
                        addEpollReadfd(ptask->epfd, ptask->peerfd);

                    }else if(cmdType==CMD_TYPE_PUTS){
                        delEpollReadfd(epfd, fd);
                        putsCommand(ptask);  
                        addEpollReadfd(ptask->epfd, ptask->peerfd);

                    }
                }
            }
        }
    }


}

void create_new_thread(){
    pthread_t tid;
    int err = pthread_create(&tid, NULL, start_routine, NULL);
    if (err) {
        error(1, err, "pthread_create");
    }
}