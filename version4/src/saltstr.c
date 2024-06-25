#include "../include/user.h"
#include <func.h>
#define STR_LEN 8

//TODO:与固定的4位拼接
void GenerateStr(char* salt){
	char str[STR_LEN+1] = {0};
	int i,flag;
	srand(time(NULL));
	for(i = 0;i < STR_LEN;i++){
		flag=rand()%3;
		switch(flag){
		case 0:str[i] = rand()%26 + 'a'; break;
		case 1:str[i] = rand()%26 + 'A'; break;
		case 2:str[i] = rand()%26 + '0'; break;
		}
	}
	char temp[12] = "$6$"; 
	strcat(temp,str);
	strcat(temp,"$");
	strcpy(salt,temp);
	
	return;
}


//int main(){
//	char salt[12] = {0};
//	GenerateStr(salt);
//
//	printf("salt:%s\n",salt);
//
//}
