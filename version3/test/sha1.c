#include <func.h>
#include"client.h"
#include<openssl/sha.h>

char* generate_sha1(int fd,char *result)
{
    char buff[1000]={0};
    int ret=0;

    SHA_CTX ctx;
    SHA1_Init(&ctx);
    while(1){
        memset(buff,0,sizeof(buff));

        ret=read(fd,buff,sizeof(buff));
        if(ret==0){
            break;
        }

        SHA1_Update(&ctx,buff,ret);
    }

    unsigned char sha[20]={0};
    SHA1_Final(sha,&ctx);

    for(int i=0;i<20;i++){
        char frag[3]={0};
        sprintf(frag,"%02x",sha[i]);
        strcat(result,frag);
    }
    
    printf("sha1:%s\n",result);

    return result;
}

