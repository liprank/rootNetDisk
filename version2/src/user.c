#include "../include/user.h"
#include "../include/thread_pool.h"
#include <stdio.h>
#include <string.h>
#include <shadow.h>

static void get_setting(char *setting,char *passwd)
{
    int i,j;
    //取出salt,i 记录密码字符下标，j记录$出现次数
    for(i = 0,j = 0; passwd[i] && j != 4; ++i) {
        if(passwd[i] == '$')
            ++j;
    }
    strncpy(setting, passwd, i);
}

void loginCheck1(user_info_t * user)
{
    printf("loginCheck1.\n");
    train_t t;
    int ret;
    memset(&t, 0, sizeof(t));
	printf("username:%s, %ld\n", user->name, strlen(user->name));
    struct spwd * sp = getspnam(user->name);

    if(sp == NULL) {// 用户不存在的情况下
        t.len = 0;   
        t.type = TASK_LOGIN_SECTION1_RESP_ERROR;
        ret = sendn(user->sockfd, &t, 8);
        printf("check1 send %d bytes.\n", ret);
        return;
    }

    //用户存在的情况下
    char setting[100] = {0};
    //保存加密密文
    strcpy(user->encrypted, sp->sp_pwdp);
    //提取setting
    get_setting(setting, sp->sp_pwdp);
    t.len = strlen(setting);
    t.type = TASK_LOGIN_SECTION1_RESP_OK;
    strncpy(t.buff, setting, t.len);
    //发送setting
    ret = sendn(user->sockfd, &t, 8 + t.len);
    printf("check1 send %d bytes.\n", ret);
}


void loginCheck2(user_info_t * user, const char * encrypted)
{
    int ret;
    train_t t;
    memset(&t, 0, sizeof(t));
    if(strcmp(user->encrypted, encrypted) == 0) {
        //登录成功
        user->status = STATUS_LOGIN;//更新用户登录成功的状态
        t.type = TASK_LOGIN_SECTION2_RESP_OK;
        t.len = strlen("/server/$ ");// 暂定将 /server/ 作为pwd传递给client 
        strcpy(t.buff, "/server/$ ");
        ret = sendn(user->sockfd, &t, 8 + t.len);
        printf("Login success.\n");
    } else {
        //登录失败, 密码错误
        t.type = TASK_LOGIN_SECTION2_RESP_ERROR;
        printf("Login failed.\n");
        ret = sendn(user->sockfd, &t, 8);
    }
    printf("check2 send %d bytes.\n", ret);
    return;
}




