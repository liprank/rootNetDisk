#include "../include/thread_pool.h"

// 辅助函数：将CmdType转换为字符串  
const char* cmdtype_to_str(CmdType type) {  
    switch (type) {  
        case CMD_TYPE_PWD: return "pwd";  
        case CMD_TYPE_LS: return "ls";  
        case CMD_TYPE_CD: return "cd";  
        case CMD_TYPE_MKDIR: return "mkdir";  
        case CMD_TYPE_RMDIR: return "rmdir";  
        case CMD_TYPE_RM: return "rm";  
        case CMD_TYPE_PUTS: return "puts";  
        case CMD_TYPE_GETS: return "gets";
        case CMD_TYPE_TREE: return "tree";
        case CMD_TYPE_CAT: return "cat";
        case CMD_TYPE_LL: return "ll";
        case CMD_TYPE_USERNAME: return "用户名";
        case CMD_TYPE_ENCODE: return "密码";//更改命令类型
        default: return "Unknown Command";  
    }  
} 

// 客户端断开连接时的日志函数
void log_client_notconnection(int sockfd) {
    time_t now;
    char time_buffer[80];
    struct tm *timeinfo;

    time(&now);
    timeinfo = localtime(&now);
    strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d %H:%M:%S", timeinfo);

    openlog("服务端", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_USER);
    syslog(LOG_INFO, "客户端 %d 断开连接时间 %s ", sockfd, time_buffer);
    closelog();
}


// 客户端连接建立时的日志函数
void log_client_connection(int peerfd) {
    time_t now;
    char time_buffer[80];
    struct tm *timeinfo;

    time(&now);
    timeinfo = localtime(&now);
    strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d %H:%M:%S", timeinfo);

    openlog("服务端", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_USER);
    syslog(LOG_INFO, "客户端 %d 请求连接时间 %s ", peerfd, time_buffer);
    closelog();
}

// 假设的客户端发送消息时的日志函数
void log_client_operation(task_t * task) {
    time_t now;
    char time_buffer[80];
    struct tm *timeinfo;

    time(&now);
    timeinfo = localtime(&now);
    strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d %H:%M:%S", timeinfo);

    openlog("服务端", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_USER);
    char cmd[1024]={0};
    snprintf(cmd, sizeof(cmd), "指令内容：%s %s ", cmdtype_to_str(task->type), task->data);
    syslog(LOG_INFO, "来自客户端 %d 的操作: %s at %s", task->peerfd, cmd, time_buffer);
    closelog();
}

