#include "../include/user.h"
#include "../include/thread_pool.h"
#include "../include/mysql.h"
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

void loginCheck1(MYSQL* pconn,user_info_t* user)
{
    printf("loginCheck1.\n");
    train_t t;
    int ret;
    memset(&t, 0, sizeof(t));
	printf("username:%s, %ld\n", user->name, strlen(user->name));
    //struct spwd * sp = getspnam(user->name);//获取用户名
	//查询数据库

	bool flag = isUserExist(pconn,user->name);
	if(flag == true){
		printf("[root] 用户存在\n");
		//获取用户的所有信息,放入user结构体中
		getAllmessage(pconn,user);	
		//发送盐值
		t.len = strlen(user->salt);
		strncpy(t.buff,user->salt,t.len);
	
		//如果用户名存在，返回成功
		//加密密码和盐值已经存在数据库中,获取盐值，发送给客户端
		
		t.type = CMD_TYPE_USERNAME_RESP_OK;
		sendn(user->sockfd,&t,8 + t.len);
	
		printf("salt = %s\n",user->salt);
	}else{
		printf("[root] 用户不存在\n");
		t.len = 0;   
        t.type = CMD_TYPE_USERNAME_RESP_ERROR;
        ret = sendn(user->sockfd, &t, 8);
        printf("check1 send %d bytes.\n", ret);
        return;

	}

	////用户存在的情况下
    //char setting[100] = {0};
    ////保存加密密文
    //strcpy(user->encrypted, sp->sp_pwdp);
    ////提取setting
    //get_setting(setting, sp->sp_pwdp);
    //t.len = strlen(setting);
    //t.type = CMD_TYPE_USERNAME_RESP_OK;
    //strncpy(t.buff, setting, t.len);
    ////发送setting
    //ret = sendn(user->sockfd, &t, 8 + t.len);
    //printf("check1 send %d bytes.\n", ret);
}


void loginCheck2(user_info_t* user,const char * encrypted)
{
    int ret;
    train_t t;
    memset(&t, 0, sizeof(t));

	//在数据库中查询密码是否正确
	if(strcmp(encrypted,user->encrypted) == 0){
		user->status = STATUS_LOGIN;
		t.type = CMD_TYPE_ENCODE_RESP_OK;

	    t.len = strlen("/rootNetDisk/$ ");// 暂定将 /server/ 作为pwd传递给client 
        strcpy(t.buff, "/rootNetDisk/$ ");

        ret = sendn(user->sockfd, &t, 8 + t.len);
        printf("[root] Login success.\n");

		//进行加密，发送token
		bzero(&t,sizeof(t));
		char buff[300] = {0};	
		jwtEncode((unsigned char*)user->name,buff);//获取Token
		t.len = strlen(buff);
		strcpy(t.buff,buff);
		ret = sendn(user->sockfd,&t,8+t.len);

    } else {
        //登录失败, 密码错误
        t.type = CMD_TYPE_ENCODE_RESP_ERROR;
        printf("[root] Login failed.\n");
        ret = sendn(user->sockfd, &t, 8);
    }

    printf("check2 send %d bytes.\n", ret);
    return;

    //if(strcmp(user->encrypted, encrypted) == 0) {
    //    //登录成功
    //    user->status = STATUS_LOGIN;//更新用户登录成功的状态
    //    t.type = CMD_TYPE_ENCODE_RESP_OK;
    //    t.len = strlen("/server/$ ");// 暂定将 /server/ 作为pwd传递给client 
    //    strcpy(t.buff, "/server/$ ");
    //    ret = sendn(user->sockfd, &t, 8 + t.len);
    //    printf("Login success.\n");
    //} else {
    //    //登录失败, 密码错误
    //    t.type = CMD_TYPE_ENCODE_RESP_ERROR;
    //    printf("Login failed.\n");
    //    ret = sendn(user->sockfd, &t, 8);
    //}
    //printf("check2 send %d bytes.\n", ret);
    //return;
}

//add enrollcheck1,检查用户名是否存在，存在为error，否则为ok
void enrollCheck1(MYSQL* pconn, user_info_t * user)
{
    printf("enrollCheck1.\n");
    train_t t;
    int ret;
    memset(&t, 0, sizeof(t));
	printf("username:%s, %ld\n", user->name, strlen(user->name));
    //struct spwd * sp = getspnam(user->name);//check user from mysql

//connect mysql and check user is exist or not
    //if(sp == NULL) {// 用户不存在的情况下
    //    t.len = 0;   
    //    t.type = CMD_TYPE_USERNAME_RESP_OK;
    //    ret = sendn(user->sockfd, &t, 8);
    //    printf("check1 send %d bytes.\n", ret);
    //    return;
    //}

	bool flag = isUserExist(pconn,user->name);
	if(flag == false){
		printf("用户不存在,可以注册\n");
		//发送注册可行的信号
		t.type = CMD_TYPE_USERNAME_RESP_OK_EN;
		sendn(user->sockfd,&t,8);	
		return;
	}else{
		printf("用户存在，不可以注册\n");
		t.type = CMD_TYPE_USERNAME_RESP_ERROR_EN;
		sendn(user->sockfd,&t,8);
		return;
	}

   // //用户存在的情况下
   // char setting[100] = {0};
   // //保存加密密文
   // strcpy(user->encrypted, sp->sp_pwdp);
   // //提取setting
   // get_setting(setting, sp->sp_pwdp);
   // t.len = strlen(setting);
   // t.type = CMD_TYPE_USERNAME_RESP_ERROR;
   // strncpy(t.buff, setting, t.len);
   // //发送setting
   // ret = sendn(user->sockfd, &t, 8 + t.len);
   // printf("check1 send %d bytes.\n", ret);
}

void enrollCheck2(MYSQL* pconn, const char* encrypted,user_info_t* user){
	printf("enrollCheck2.\n");

	int ret;
	train_t t;
	bzero(&t,sizeof(t));

	//接收用户输入的密码，可进行长度检验
	//生成盐值，接收密码，并进行加密
	if(strlen(encrypted) < 8){
		t.type = CMD_TYPE_ENCODE_RESP_ERROR_EN;	

		t.len = strlen("密码不足8位，请重新设置");
		strcpy(t.buff,"密码不足8位，请重新设置");

		ret = sendn(user->sockfd,&t.type,4);//直接发送命令类型
		printf("密码不足8位\n");	
	}else{
		t.type = CMD_TYPE_ENCODE_RESP_OK_EN;
		printf("密码符合要求。\n");
		ret = sendn(user->sockfd,&t.type,4);

		//进行加密，发送token
		bzero(&t,sizeof(t));
		char buff[300] = {0};	
		jwtEncode((unsigned char*)user->name,buff);//获取Token
		t.len = strlen(buff);
		strcpy(t.buff,buff);
		ret = sendn(user->sockfd,&t,8+t.len);


		//加密并写入数据库
		strcpy(user->encrypted,encrypted);
		//TODO：生成随机盐值
		GenerateStr(user->salt);
		strcpy(user->encrypted,crypt(encrypted,user->salt));

		writeIntoSql(pconn,user);

	}

	printf("check 2 send %d bytes.\n",ret);
	return;
}


