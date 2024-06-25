/*
 *@author cqy created
 *file cd.c
 *data 2024-6-17
 *@brief 接收到cd命令之后，根据后面的task_data切换到相对应的目录
 */


#include "../include/thread_pool.h"
//#include "../include/cd.h"

char pre_cwd[100] = "";//上一次的目录
char cur_cwd[100] = ""; //当前工作目录

// 函数用于获取并返回前两个目录的路径  
char* get_first_two_dirs(char* cwd) {  
    // 确保 cwd 字符串以 '/' 结尾，如果不是，则添加  
    size_t len = strlen(cwd);  
    if (cwd[len - 1] != '/') {  
        cwd[len] = '/';  
        cwd[len + 1] = '\0';  
    }  

    // 查找第二个 '/' 的位置  
    char* second_slash = strchr(cwd + 1, '/');  
    if (!second_slash) {  
        // 如果没有找到第二个 '/', 则返回整个路径  
        return cwd;  
    }  

    // 查找第三个 '/' 的位置  
    char* third_slash = strchr(second_slash + 1, '/');  
    if (!third_slash) {  
        // 如果没有找到第三个 '/', 则只截取到第二个 '/'  
        *second_slash = '\0';  
        return cwd;  
    }  

    // 截取到第二个 '/'  
    *third_slash = '\0';  
    return cwd;  
}                                                     

void cdCommand(task_t * task)
{
    // 获取当前用户的家目录
    //获取当前工作目录
    char cur_dir[100] = {0};
    char* q = getcwd(cur_dir, sizeof(cur_dir));
    if(q == NULL){
        error(1, errno, "getcwd");
    }
    char* home_dir = get_first_two_dirs(cur_dir);
    if (home_dir == NULL) {
        // 无法获取家目录，发送错误消息
        char err[100] = "无法获取根目录";
        sendn(task->peerfd, err, strlen(err));
        exit(1); // 或者其他错误处理
    }

    // 构建完整的目录路径
    char server_cwd[100] = "/";
    char home_cwd[100] = "~"; //家目录

    snprintf(server_cwd, sizeof(server_cwd), "%s/root-net-disk/version2", home_dir);
    snprintf(home_cwd, sizeof(home_cwd), "%s/root-net-disk/version2/src", home_dir);

    //获取当前工作目录
    char cwd[100] = {0};
    char* p = getcwd(cwd, sizeof(cwd));
    if(p == NULL){
        error(1, errno, "getcwd");
    }
    //获取要改变的工作路径(拼接路径)
    char path[1100] = {0};
    if(cwd[strlen(cwd)-1] == '/'){
        //如果cwd以'/'结尾，则删除'/'
        cwd[strlen(cwd)-1] = '\0';
    }
    //然后添加一个'/'
    sprintf(path, "%s/%s", cwd, task->data);

    //判断task->data是不是".."
    if(strcmp(task->data,"..") == 0){
        char* last_slash = strrchr(cwd, '/');
        //再定义一个现在所在的目录，用于更新上次工作所在的目录
        char cwd_1[100] = {0};
        strcpy(cwd_1, cwd);
        if(strcmp(cwd, server_cwd) == 0 || last_slash == NULL){
            //已经在第一级（根）目录，cd失败
            char err[100] = "返回上一级目录失败，已在第一级目录";
            sendn(task->peerfd, err, strlen(err));
        }else{
            //将最后一个'/'置为字符终结符，返回上一级目录
            *last_slash = '\0';
            if (chdir(cwd) != 0){  
                perror("chdir() error");  
                exit(EXIT_FAILURE);
            }  
            //更新上一次和当前所在的目录
            strcpy(pre_cwd, cwd_1);
            strcpy(cur_cwd, cwd);
        }
        //判断是不是"."
    }else if(strcmp(task->data, ".") == 0){
        char err[100] = "目录没有变化";
        sendn(task->peerfd, err, strlen(err));

    }else if(strcmp(task->data, "-") == 0){
        //返回上一次操作的目录
        int ret = chdir(pre_cwd);
        if(ret == -1){
            char err[100] = "返回上一个工作的目录失败";
            sendn(task->peerfd, err, strlen(err));
        }else{
            //打印目录转换信息
            char err[100] = "返回上一个工作的目录";
            sendn(task->peerfd, err, strlen(err));

        }
        //更新之前和现在所在的目录
        strcpy(cur_cwd, pre_cwd);
        strcpy(pre_cwd, cwd);
    }else if(strcmp(task->data, "/") == 0){

        //返回根目录
        int ret = chdir(server_cwd);
        if(ret == -1){
            char err[100] = "返回根目录失败";
            sendn(task->peerfd, err, strlen(err));
        }else{
            char err[100] = "返回根目录";
            sendn(task->peerfd, err, strlen(err));
        }
        //更新之前和现在所在的的目录
        strcpy(pre_cwd, cwd);
        strcpy(cur_cwd, server_cwd);
    }else if(strcmp(task->data, "~") == 0 || strcmp(task->data, "") == 0){
        //返回家目录
        int ret = chdir(home_cwd);
        if(ret == -1){
            char err[100] = "返回家目录失败";
            sendn(task->peerfd, err, strlen(err));
        }else{
            char err[100] = "返回家目录";
            sendn(task->peerfd, err, strlen(err));
        }
        strcpy(pre_cwd, cwd);
        strcpy(cur_cwd, home_cwd);
    }else{
        int ret = chdir(path);
        if(ret == -1){
            char err[100] = "不符合cd的输入规则，请输入一个当前目录下的目录";
            sendn(task->peerfd, err, strlen(err));
        }else{
            //更新之前和现在所在的目录
            strcpy(pre_cwd, cwd);
            strcpy(cur_cwd, path);
        }     
    }
}

