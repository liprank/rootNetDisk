#include "../include/thread_pool.h"
#include "../include/mysql.h"
//子线程连接数据库，需要互斥访问

MYSQL* mysqlconnect(){
	MYSQL* conn = (MYSQL*)malloc(sizeof(MYSQL));//在堆空间中申请pconn，才不会受到栈空间的影响
	MYSQL* pconn = mysql_init(conn);

//可使用配置文件
	pconn = mysql_real_connect(pconn,"localhost","root","1234","rootNetDisk",0,NULL,0);
	if(pconn == NULL){
		printf("%s\n",mysql_error(conn));
		exit(0);
	}

	return pconn;
}

bool isUserExist(MYSQL* pconn,char* username){

	//进行查询操作
	
	char query[100] = {0}; 
	sprintf(query,"select exists (select 1 from users where username = '%s')",username);
	int ret = mysql_query(pconn, (const char*)query);
	if(ret != 0) {
		printf("(%d, %s)\n", 
			   mysql_errno(pconn),
			   mysql_error(pconn));
		return EXIT_FAILURE;
	} 
	MYSQL_RES * result = mysql_use_result(pconn);
	if(result) {
		//打印具体的每一行数据
		MYSQL_ROW row = mysql_fetch_row(result);
		if(strcmp(row[0],"1") == 0){
			mysql_free_result(result);//释放结果集
			return true;
		}else {
			mysql_free_result(result);
			return false;
		}
	}
	
	return 0;
}

//int main(){
//	MYSQL* pconn = mysqlconnect();
//	int flag = isUserExist(pconn,"yang");
//	if(flag == 0){
//		printf("用户不存在\n");
//	}else{
//		printf("用户存在\n");
//	}
//}

void getAllmessage(MYSQL* pconn,user_info_t* user){
	

	//进行查询操作
	char query[100]; 
	printf("%s\n",user->name);
	
	sprintf(query,"select salt,cryptPasswd from users where username='%s'",user->name);	
	int ret = mysql_query(pconn, query);
    if(ret != 0) {
        printf("(%d, %s)\n", 
               mysql_errno(pconn),
               mysql_error(pconn));
    } 

    MYSQL_RES * result = mysql_store_result(pconn);
    if(result) {
        //打印具体的每一行数据
        MYSQL_ROW row = mysql_fetch_row(result);//MSYQL_ROW本质上是一个char**
		strcpy(user->salt,row[0]);
		strcpy(user->encrypted,row[1]);
    }
    //释放结果集中的数据
    mysql_free_result(result);
    //关闭连接
	return;
}

//int main()
//{
//	MYSQL* pconn = mysqlconnect();
//	user_info_t* test = (user_info_t*)malloc(sizeof(user_info_t));
//	bzero(test,sizeof(*test));
//	strcpy(test->name,"yang");
//
//	//bool flag = isUserExist(pconn,"yang");
//	//if(flag==1){
//	//	printf("用户存在\n");
//	//}else{
//	//	printf("用户不存在\n");
//	//}
//	getAllmessage(pconn,test);
//	printf("salt:%s encode:%s\n",test->salt,test->encrypted);
//    return 0;
//}

void writeIntoSql(MYSQL* pconn,user_info_t* user){
	char query[200];	
	
	sprintf(query,"INSERT INTO users(userName,salt,cryptPasswd) values('%s','%s','%s')",user->name,user->salt,user->encrypted);
	int ret = mysql_query(pconn,query);
	if(ret != 0) {
        printf("(%d, %s)\n", 
               mysql_errno(pconn),
               mysql_error(pconn));
    } 

	return;
}

//int main(){
//	MYSQL* pconn = mysqlconnect();
//	user_info_t* test = (user_info_t*)malloc(sizeof(user_info_t));
//	bzero(test,sizeof(*test));
//	strcpy(test->name,"ll2");
//	strcpy(test->salt,"1234");
//	strcpy(test->encrypted,"5678");
//
//	writeIntoSql(pconn,test);	
//    return 0;
//}


