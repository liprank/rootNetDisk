/*
*@file bussiness.c
*@author ymz
*@date 6-18
*@brief 新增rm、tree、cat、cp命令 -> 新增user login 命令 -> 修改命令接口访问数据库
*/

#include "../include/linkedList.h"
#include "../include/thread_pool.h"
#include "../include/user.h"
//主线程调用:处理客户端发过来的消息
void handleMessage(int sockfd, int epfd, task_queue_t * que)
{
    //消息格式：cmd content
    //1.1 获取消息长度
    int length = -1;
    int ret = recvn(sockfd, &length, sizeof(length));
    printf("recv length: %d\n", length);

    //1.2 获取消息类型
    int cmdType = -1;
    ret = recvn(sockfd, &cmdType, sizeof(cmdType));
    printf("recv cmd type: %d\n", cmdType);
    task_t *ptask = (task_t*)calloc(1, sizeof(task_t));
    ptask->peerfd = sockfd;
    ptask->epfd=epfd;
    ptask->type= (CmdType)cmdType;
    if(length > 0) {
        //1.3 获取消息内容
        ret = recvn(sockfd, ptask->data, length);
        if(ret > 0) {
            //往线程池中添加任务
            if(ptask->type == CMD_TYPE_PUTS||ptask->type==CMD_TYPE_GETS) {
                //是上传文件任务，就暂时先从epoll中删除监听
                //delEpollReadfd(epfd, sockfd);
            }
            taskEnque(que, ptask);
        }
    } else if(length == 0){
        taskEnque(que, ptask);
    }

    if(ret == 0) {//连接断开的情况
        printf("\nconn %d is closed.\n", sockfd);
        
        //记录客户端断开连接信息
        log_client_notconnection(sockfd);
        
        delEpollReadfd(epfd, sockfd);
        close(sockfd);
    }
}

//注意：此函数可以根据实际的业务逻辑，进行相应的扩展

info io;

void doTask(task_t * task)
{
    assert(task);

    short port=8080;

    switch(task->type) {
    case CMD_TYPE_PWD:  
        pwdCommand(task);   break;
    case CMD_TYPE_LS:
        lsCommand(task);    break;
    case CMD_TYPE_LL:
        llCommand(task);    break;
    case CMD_TYPE_CD:
        cdCommand(task);    break;
    case CMD_TYPE_MKDIR:
        mkdirCommand(task);  break;
	case CMD_TYPE_RM:
		rmCommand(task);	break;
    case CMD_TYPE_RMDIR:
        rmdirCommand(task);  break;
    case CMD_TYPE_PUTS:
        io.t.type=task->type;
        strcpy(io.t.buff,task->data);
        sendn(task->peerfd,(char*)&port,sizeof(port));
        break;
    case CMD_TYPE_GETS:
        io.t.type=task->type;
        strcpy(io.t.buff,task->data);
        sendn(task->peerfd,(char*)&port,sizeof(port));
        break; 
	case CMD_TYPE_TREE:
		treeCommand(task);	break;
	case CMD_TYPE_CAT:
		catCommand(task);	break;
    case CMD_TYPE_NOTCMD:
        notCommand(task);   break;

//fix cmdtype
    case CMD_TYPE_USERNAME:
        userLoginCheck1(task); break;
    case CMD_TYPE_ENCODE:
        userLoginCheck2(task); break;

//add enroll
	case CMD_TYPE_USERNAME_EN: 
		userEnrollCheck1(task); break;
	case CMD_TYPE_ENCODE_EN: 
		userEnrollCheck2(task); break;

    }
}

//每一个具体任务的执行，交给一个成员来实现

//add function

//外部变量(userList是在main.c文件中定义的)
extern user_info_t* user;//当前user的信息在mian.c中定义

void userLoginCheck1(task_t * task) {
    printf("userLoginCheck1.\n");
    //ListNode * pNode = userList;
    //while(:epNode != NULL) {
    //    user_info_t * user = (user_info_t *)pNode->val;
    //    if(user->sockfd == task->peerfd) {
    //        //拷贝用户名
    //        strcpy(user->name, task->data);
    //        loginCheck1(pconn);//从数据库中查找
    //        return;
    //    }
    //}

	while(user != NULL){
		if(user->sockfd == task->peerfd){
			strcpy(user->name,task->data);
			loginCheck1(task->pconn,user);//接收用户名，检查数据库
			return;
		}
	}
}

void userLoginCheck2(task_t * task) {
    printf("userLoginCheck2.\n");
    //ListNode * pNode = userList;
    //while(pNode != NULL) {
    //    user_info_t * user = (user_info_t *)pNode->val;
    //    if(user->sockfd == task->peerfd) {
    //        //拷贝加密密文
    //        loginCheck2(pconn, task->data);//从数据库查找
    //        return;
    //    }
    //}

	//TODO:user为当前用户信息，注意！！
	if(user != NULL){
		if(user->sockfd == task->peerfd){
			printf("task->data:%s\n",task->data);
			loginCheck2(user,task->data);//检查登录密码是否正确
		}
	}
}

//add user enroll
//用户名不存在，生成salt，发送给client
void userEnrollCheck1(task_t * task){
	printf("userEnrollCheck1.\n");
	//ListNode * pNode = userList;
    //while(pNode != NULL) {
    //    user_info_t * user = (user_info_t *)pNode->val;
    //    if(user->sockfd == task->peerfd) {
    //        //拷贝用户名
    //        strcpy(user->name, task->data);
    //        enrollCheck1(pconn);
    //        return;
    //    }
    //}

	if(user != NULL){
		if(user->sockfd == task->peerfd){
			strcpy(user->name,task->data);
			enrollCheck1(task->pconn,user);
		}
	}

}

//接收用户设置的密码，检查密码，符合要求返回成功，设置盐值，加密密码，写入数据库
void userEnrollCheck2(task_t * task){
    printf("userEnrollCheck2.\n");
    //ListNode * pNode = userList;
    //while(pNode != NULL) {
    //    user_info_t * user = (user_info_t *)pNode->val;
    //    if(user->sockfd == task->peerfd) {
    //        //拷贝加密密文
    //        enrollCheck2(pconn, task->data);
    //        return;
    //    }
    //}

	if(user != NULL){
		if(user->sockfd == task->peerfd){
			printf("用户设置的密码:%s\n",task->data);	
			enrollCheck2(task->pconn,task->data,user);
		}
	}
}

