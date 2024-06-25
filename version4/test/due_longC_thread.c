#include "client.h"
#include<func.h>

void* start_routine(void* args) {
    short port=(short)args;
    printf("%hd\n",port);

    int nclientfd = socket(AF_INET,SOCK_STREAM,0);

    struct sockaddr_in addr;//服务端地址
    memset(&addr, 0, sizeof(addr));//初始化
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    int ret = connect(nclientfd,(struct sockaddr *)&addr,sizeof(addr));
    ERROR_CHECK(ret,-1,"connect");
    
    //extern client_user_info* clientUser;
    //sendn(nclientfd,(char*)clientUser,sizeof(client_user_info));
    ///////////////////////////////////////////////////////////////////////////////

    while(true){
        extern info io;

        if(io.t.type==CMD_TYPE_PUTS){
            
            if(io.existFlag==1){
                sendn(nclientfd,&io.t,4+4+io.t.len);//发送命令
                due_putscommand(nclientfd,io.t);
                io.existFlag=0;
            }
            

        }else if(io.t.type==CMD_TYPE_GETS){
            
            if(io.existFlag==1){
                sendn(nclientfd,&io.t,4+4+io.t.len);//发送命令
                due_getscommand(nclientfd,io.t);
                io.existFlag=0;
            }
            

        }

    }
    return NULL;

}

void create_duePG_thread(short port){
    pthread_t tid;
    int err = pthread_create(&tid, NULL, start_routine, (void*)port);
    if (err) {
        error(1, err, "pthread_create");
    }

}