#include "../include/thread_pool.h"
#include "../include/user.h"
//直接tree命令
//按首字母排序,不区分大小写
//├,─,└

char tree[1000] = "";
char line[256]="";
int dirs = 0;
int files = 0;
char grap1[] = "├── ";
char grap2[] = "│   ";
char grap3[] = "└── ";
char grap4[] = "    ";
extern user_info_t * user;

int NamePrint(task_t* task,MYSQL* pcoon, int parentId, int ownerId, int n, int igno) {
	int len;
	int igno1 = igno;
	char patt[256] = "";

	for (int i = 0; i < n - 1; igno1 >>= 1, i++) {
		if (igno1 & 0x1) {
			sprintf(patt, "%s%s", patt, grap4);
			continue;
		}
		sprintf(patt, "%s%s", patt, grap2);
	}
	//查找parentId为fileId,ownerId为userId的文件fileId,filename，fileType
	int fileId;
	char fileName[21];
	char fileType[2];
	MYSQL_STMT* stmt;
	MYSQL_BIND bind_params[2];
	MYSQL_BIND bind_results[3];

	stmt = mysql_stmt_init(pcoon);
	if (!stmt) {
		printf("treeCommand:NamePrint:cycle:%d,mysql_stmt_init() failed\n", n);
		return 0;
	}

	// 准备预处理查询语句
	const char* query = "SELECT fileId, fileName, fileType FROM files WHERE parentId = ? AND ownerId = ? ORDER BY fileName ASC";
	if (mysql_stmt_prepare(stmt, query, strlen(query))) {
		printf("treeCommand:NamePrint:cycle:%d,mysql_stmt_prepare() failed\n", n);
		mysql_stmt_close(stmt);
		return 0;
	}

	// 初始化输入参数绑定数组
	memset(bind_params, 0, sizeof(bind_params));

	// 配置 parentId 参数的绑定信息
	bind_params[0].buffer_type = MYSQL_TYPE_LONG;
	bind_params[0].buffer = &parentId;
	bind_params[0].is_null = 0;
	bind_params[0].length = NULL;

	// 配置 ownerId 参数的绑定信息
	bind_params[1].buffer_type = MYSQL_TYPE_LONG;
	bind_params[1].buffer = &ownerId;
	bind_params[1].is_null = 0;
	bind_params[1].length = NULL;

	if (mysql_stmt_bind_param(stmt, bind_params)) {
		printf("treeCommand:NamePrint:cycle:%d,mysql_stmt_bind_param() failed\n",n);
		mysql_stmt_close(stmt);
		return 0;
	}

	// 初始化输出结果绑定数组
	memset(bind_results, 0, sizeof(bind_results));

	// 配置 id 参数的绑定信息
	bind_results[0].buffer_type = MYSQL_TYPE_LONG;
	bind_results[0].buffer = &fileId;
	bind_results[0].buffer_length = sizeof(fileId);

	// 配置 name 参数的绑定信息
	bind_results[1].buffer_type = MYSQL_TYPE_STRING;
	bind_results[1].buffer = fileName;
	bind_results[1].buffer_length = sizeof(fileName);

	// 配置 type 参数的绑定信息
	bind_results[2].buffer_type = MYSQL_TYPE_STRING;
	bind_results[2].buffer = fileType;
	bind_results[2].buffer_length = sizeof(fileType);

	// 执行预处理查询语句
	if (mysql_stmt_execute(stmt)) {
		printf("treeCommand:NamePrint:cycle:%d,mysql_stmt_execute() failed\n", n);
		mysql_stmt_close(stmt);
		return 0;
	}

	//绑定输出结果
	if (mysql_stmt_bind_result(stmt, bind_results)) {
		printf("treeCommand:NamePrint:cycle:%d,mysql_stmt_bind_result() failed\n",n);
		mysql_stmt_close(stmt);
		return 0;
	}

	// 存储查询结果
	if (mysql_stmt_store_result(stmt)) {
		printf("treeCommand:NamePrint:cycle:%d,mysql_stmt_store_result() failed\n", n);
		mysql_stmt_close(stmt);
		return 0;
	}
	int count = mysql_stmt_num_rows(stmt);
	if (count == 0) {
		mysql_stmt_free_result(stmt);
		mysql_stmt_close(stmt);
		return 2;
	}


	for (int i = 0; i < count; i++) {
		// 取出查询结果
		if (mysql_stmt_fetch(stmt)) {
			printf("treeCommand:NamePrint:cycle:%d,mysql_stmt_fetch() failed\n", n);
			mysql_stmt_close(stmt);
			return 0;
		}
		if (i < count - 1)
			sprintf(line, "%s%s%s\n", patt, grap1, fileName);
		else
			sprintf(line, "%s%s%s\n", patt, grap3, fileName);
        strcat(tree,line);
		if (strcmp(fileType, "d")) {
			files++;
		}
		else {

			dirs++;
			if (i < count - 1)
				NamePrint(task,pcoon, fileId, ownerId, n + 1, igno);
			else
				NamePrint(task,pcoon, fileId, ownerId, n + 1, igno | (1 << n - 1));
		}
	}

	mysql_stmt_free_result(stmt);
	mysql_stmt_close(stmt);
	return 1;
}


// 从 users 表中查询 userId 和 curDirectory
int get_user_info(MYSQL* pcoon, const char* username, int* userId, int* curDirectory) {
	MYSQL_STMT* stmt;
	MYSQL_BIND bind_params[1];
	MYSQL_BIND bind_results[2];
	// unsigned long name_length = strlen(usr->username);
	unsigned long name_length = strlen(username);

	stmt = mysql_stmt_init(pcoon);
	if (!stmt) {
		printf("treeCommand:get_user_info:mysql_stmt_init() failed\n");
		return 0;
	}

	// 准备预处理查询语句
	const char* query = "SELECT userId, curDirectory FROM users WHERE userName = ?";
	if (mysql_stmt_prepare(stmt, query, strlen(query))) {
		printf("treeCommand:get_user_info:mysql_stmt_prepare() failed\n");
		mysql_stmt_close(stmt);
		return 0;
	}

	// 初始化输入参数绑定数组
	memset(bind_params, 0, sizeof(bind_params));

	// 配置 name 参数的绑定信息
	bind_params[0].buffer_type = MYSQL_TYPE_STRING;
	bind_params[0].buffer = (char*)username;
	//bind_params[0].buffer = (char *)usr->username;
	bind_params[0].is_null = 0;
	bind_params[0].length = &name_length;

	// 绑定输入参数
	if (mysql_stmt_bind_param(stmt, bind_params)) {
		printf("treeCommand:get_user_info:mysql_stmt_bind_param() failed\n");
		mysql_stmt_close(stmt);
		return 0;
	}

	// 初始化输出结果绑定数组
	memset(bind_results, 0, sizeof(bind_results));

	// 配置 id 参数的绑定信息
	bind_results[0].buffer_type = MYSQL_TYPE_LONG;
	bind_results[0].buffer = userId;
	bind_results[0].length = 0;

	// 配置 directory 参数的绑定信息
	bind_results[1].buffer_type = MYSQL_TYPE_LONG;
	bind_results[1].buffer = curDirectory;
	bind_results[1].length = 0;

	// 绑定输出结果
	if (mysql_stmt_bind_result(stmt, bind_results)) {
		printf("treeCommand:get_user_info:mysql_stmt_bind_result() failed\n");
		mysql_stmt_close(stmt);
		return 0;
	}

	// 执行预处理查询语句
	if (mysql_stmt_execute(stmt)) {
		printf("treeCommand:get_user_info:mysql_stmt_execute() failed\n");
		mysql_stmt_close(stmt);
		return 0;
	}


	// 获取查询结果
	if (mysql_stmt_fetch(stmt)) {
		printf("treeCommand:get_user_info:mysql_stmt_fetch() failed\n");
		mysql_stmt_close(stmt);
		return 0;
	}
	mysql_stmt_free_result(stmt);

	// 清理
	mysql_stmt_close(stmt);

	return 1;
}

void treeCommand(task_t* task) {
	char str[128] = "";
    dirs=0;
    files=0;
	//在users表中获得userId,curDirectory
	int userId, curDirectory;
	int len = 0;
	if (get_user_info(task->pconn, user->name, &userId, &curDirectory)) {// 查询成功
		//在files表中查找fileId为curDirectory的filename
		//从files文件中查找parentId为curDirectory,ownerId为userId的fileId,filename
		//若fileType为d,则进入该目录,并递归调用NamePrint函数
		//若fileType为f,则打印文件名
		//若没有下级目录或文件，结束递归
		sprintf(line,".\n");
        strcat(tree,line);
        int ret=NamePrint(task,task->pconn, curDirectory, userId, 1, 0);
		if(ret==1){	
        sprintf(line, "%d directories, %d files\n",  dirs, files);
        strcat(tree,line);
        len=strlen(tree);
        sendn(task->peerfd, &len, sizeof(int));
		sendn(task->peerfd, tree,len);
		}
		else if(ret==0) {
            printf("treeCommand:NamePrint() failed\n");
			sprintf(str, "Show files tree failed!\n");
            len=strlen(str);
            sendn(task->peerfd, &len, sizeof(int));
			sendn(task->peerfd, str,len);
		
		}
        else{
            sprintf(str, "No files in this directory!\n");
        len=strlen(str);
        sendn(task->peerfd, &len, sizeof(int));
			sendn(task->peerfd, str,len);
        }
	}
	else {
		printf("treeCommand:get_user_info() failed\n");
        sprintf(str,"Show files tree failed!\n");
		len=strlen(str);
        sendn(task->peerfd, &len, sizeof(int));
			sendn(task->peerfd, str,len);
	}

	return ;
}
