/*
 *@author cqy created
 *file ll.c
 *data 2024-6-18
 *@brief 接收到ll命令之后，打印当前目录的所有文件信息
 */
#include "../include/thread_pool.h"
//设置文件类型和权限字符串str
void set_type_mode(mode_t mode, char* str){
    switch(mode & S_IFMT){
    case S_IFBLK:   str[0] = 'b';    break;//块设备
    case S_IFCHR:   str[0] = 'c';    break;//字符设备
    case S_IFDIR:   str[0] = 'd';    break;//目录
    case S_IFIFO:   str[0] = 'p';    break;//管道
    case S_IFLNK:   str[0] = 'l';    break;//链接
    case S_IFREG:   str[0] = '-';    break;//常规文件
    case S_IFSOCK:  str[0] = 's';    break;//套接字
    default:        str[0] = '?';    break;//未知类型
    }

    //根据文件权限设置字符串后续字符
    //设置拥有者权限
    str[1] = (mode & 0400) ? 'r' : '-';
    str[2] = (mode & 0200) ? 'w' : '-';
    str[3] = (mode & 0100) ? 'x' : '-';

    //设置拥有者组权限
    str[4] = (mode & 0400) ? 'r' : '-';
    str[5] = (mode & 0200) ? 'w' : '-';
    str[6] = (mode & 0100) ? 'x' : '-';

    //设置其他人权限
    str[7] = (mode & 0400) ? 'r' : '-';
    str[8] = (mode & 0200) ? 'w' : '-';
    str[9] = (mode & 0100) ? 'x' : '-';

}

//获取格式化的时间字符串
void get_time_str(time_t mtime, char* time_str){
    //
    // char mon_arr[][10] ={
    //     "1月","2月","","","","",
    //     "","","","","","",""
    // };
    struct tm * tm_val = localtime(&mtime);
    sprintf(time_str,
            "%04d-%02d-%02d %02d:%02d",
            tm_val->tm_year + 1900, //年
            tm_val->tm_mon + 1,  //月
            tm_val->tm_mday, //日
            tm_val->tm_hour, //时
            tm_val->tm_min); //分
}

//将文件大小转换成以B,K,M,G为单位的
void format_file_size(char *dest, size_t dest_size, off_t size) {  
    const char *units[] = {"B", "K", "M", "G"};  
    unsigned long unit_index = 0;  
    double file_size_double = (double)size;  

    while (file_size_double >= 1024 && unit_index < (sizeof(units) / sizeof(units[0]) - 1)) {  
        file_size_double /= 1024;  
        unit_index++;  
    }  

    // 使用snprintf确保不会溢出  
    snprintf(dest, dest_size, "%.2f%s", file_size_double, units[unit_index]);  
}

//qsort的compar参数，以字符排序
int compare_filenames(const void *a, const void *b) {  
    return strcmp(*(const char *const *)a, *(const char *const *)b);  
} 

//ll实现函数
void llCommand(task_t * task)
{
    //打开当前目录
    DIR*dir = opendir(".");
    if(dir == NULL){
        perror("opendir");
        exit(0);
    }

    //打印当前目录的信息
    struct dirent * entry;
    //排序
    
    char buff[2048]= {0};
    size_t buff_len = 0;
    while((entry = readdir(dir)) != NULL){
        struct stat stat_buf;
        char full_path[1024];
        snprintf(full_path, sizeof(full_path), "./%s", entry->d_name);
        //获取文件详细信息
        int ret = stat(full_path, &stat_buf);
        if(ret == -1){
            char err[100] = "无法获取文件信息";
            sendn(task->peerfd, err, strlen(err));
            continue;
        }
        //定义字符数组保存类型及权限信息
        char mode_str[1024] = {0};
        set_type_mode(stat_buf.st_mode, mode_str);

        //定义字符数组保存时间信息
        char time_str[1024] = {0};
        get_time_str(stat_buf.st_mtim.tv_sec, time_str);

        struct passwd *pw = getpwuid(stat_buf.st_uid);
        const char *username = pw ? pw->pw_name : "UNKNOWN";
        struct group *gr = getgrgid(stat_buf.st_gid);
        const char *groupname = gr ? gr->gr_name : "UNKNOWN";
        char file_size_str[32]; // 假设这个数组足够大来存储格式化的文件大小
        format_file_size(file_size_str, sizeof(file_size_str), stat_buf.st_size);
        // 将输出追加到buff中，确保不会溢出
        buff_len += snprintf(buff + buff_len, sizeof(buff) - buff_len,
                             "%s %lu %s %s %8s %s %s\n",
                             mode_str,                          //文件类型与权限
                             (unsigned long)stat_buf.st_nlink,  //硬链接数
                             username,                          //拥有者名
                             groupname,                         //拥有者组名
                             file_size_str,   //文件大小
                             time_str,                          //最后修改时间字符串
                             entry->d_name);                    //文件名
        if(buff_len >= sizeof(buff) - 100){ //保留空间以防万一
            sendn(task->peerfd, buff, buff_len);//发送给客户端
            buff_len = 0;   //重置buff_len
            buff[0] = '\0'; //清空buff
        }
    }

    //发送剩下的buff内容(如果有)
    if(buff_len > 0){
        sendn(task->peerfd, buff, buff_len);
        fflush(stderr);
    }
    //关闭目录
    closedir(dir);
}


