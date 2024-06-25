#include "l8w8jwt/encode.h"
#include <func.h>
#include "../include/user.h"

int jwtEncode(unsigned char* userName,char* Token)
{
    char* jwt = NULL;//token
    size_t jwt_length = 0;//the length of token

    struct l8w8jwt_encoding_params params;
    l8w8jwt_encoding_params_init(&params);

    params.alg = L8W8JWT_ALG_HS512;//选择的加密算法
	//不变
    params.sub = "Gordon Freeman";//主题
    params.iss = "Black Mesa";//签发人
    params.aud = "Administrator";//受众

    params.iat = 0;//签发时间
    params.exp = 0x7fffffff; /* Set to expire after 10 minutes (600 seconds). */

    //params.secret_key = (unsigned char*)"yang";//每次需要
	
	printf("username:%s\n",userName);
	params.secret_key = (unsigned char*)calloc(300,sizeof(char));
	strcpy(params.secret_key,userName);

    params.secret_key_length = strlen(params.secret_key);

    params.out = &jwt;
    params.out_length = &jwt_length;

    int r = l8w8jwt_encode(&params);

    //printf("\n l8w8jwt example HS512 token: %s \n", r == L8W8JWT_SUCCESS ? jwt : " (encoding failure) ");
	strcpy(Token,jwt);

    /* Always free the output jwt string! */
    l8w8jwt_free(jwt);

    return 0;
}

//int main(){
//	unsigned char* username = "yang";
//	char Token[300] = {0};
//	jwtEncode(username,Token);
//	printf("Token:%s\n",Token);
//
//
//}
