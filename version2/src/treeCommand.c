#include "../include/thread_pool.h"
//直接tree命令，不显示隐藏文件(.除外)
//tree -a 显示隐藏文件(..除外)
//tree xx/xx/xx 显示此路径tree,以/开头的是绝对路径
//按首字母排序
//├,─,└
int treeMode;//tree=0,tree -a=1,tree xx/xx/xx=2
char line[4096]="";
char str[256]="";

char grap1[]="├── ";
char grap2[]="│   ";
char grap3[]="└── ";
char grap4[]="    ";

int dirs=0;
int files=0;

int alphasort_no_case(const struct dirent **a,const struct dirent **b) {
    return strcasecmp((*a)->d_name, (*b)->d_name);
}


int sel(const struct dirent *entry){
    if(treeMode==0||treeMode==2){
        if(entry->d_name[0]=='.'){
            return 0;
        }
        return 1;
    }
    if(treeMode==1){
        if(entry->d_name[0]=='.'&&(entry->d_name[1]=='.')||(entry->d_name[1]==0)){
            return 0;   
        }
        return 1;
    }
    return 1;
}

void NamePrint(task_t * task,char* s,int n,int igno){
    int igno1=igno;
    char patt[4096]="";
    char path[4096]="";

    struct dirent**entryList;

    for(int i=0;i<n-1;igno1>>=1,i++){
        if(igno1&0x1){
            sprintf(patt,"%s%s",patt,grap4);
            continue;
        }
        sprintf(patt,"%s%s",patt,grap2);
    }

    int count=scandir(s,&entryList,sel,alphasort_no_case);
    if(count<0){
        sprintf(str,"treeCommand-scandir failed:%s",strerror(errno));
        printf("%s\n",str);
        sendn(task->peerfd,str,strlen(str));
        return;
    }

    for(int i=0;i<count;i++){
        if(i<count-1)
            sprintf(line,"%s%s%s\n     ",patt,grap1,entryList[i]->d_name);
        else
            sprintf(line,"%s%s%s\n     ",patt,grap3,entryList[i]->d_name);
        sendn(task->peerfd,line,strlen(line));
        if(entryList[i]->d_type==DT_DIR){
            dirs++;
            sprintf(path,"%s/%s",s,entryList[i]->d_name);
            if(i<count-1)
                NamePrint(task,path,n+1,igno);
            else
                NamePrint(task,path,n+1,igno|(1<<n-1));
        }else{
            files++;
        }
    }

    free(entryList);
    return;
}


void treeCommand(task_t * task)
{
    dirs=0;
    files=0;
    char path[4096]="";
    if(task->data[0]==0){
        treeMode=0;
    }
    else if(task->data[0]=='-'&&task->data[1]=='a'){
        treeMode=1;
    }
    else if(task->data[0]!='-'){
        treeMode=2;

    }
    else{
        sprintf(str,"Wrong instruction format for tree!");
        printf("%s\n",str);
        sendn(task->peerfd,str,strlen(str));
    }

    if(task->data[0]=='/'){
        sprintf(path,"%s",task->data);
    }else{
        char*ret=getcwd(path,sizeof(path));
        if(ret==NULL){
            sprintf(str,"treeCommand-getcwd failed:%s",strerror(errno));
            printf("%s\n",str);
            sendn(task->peerfd,str,strlen(str));
        }
        if(task->data[0]!='-')
            sprintf(path,"%s/%s",path,task->data);
    }
    sendn(task->peerfd,".\n    ",2);
    NamePrint(task,path,1,0);


    sprintf(str,"%d directories, %d files",dirs,files);
    sendn(task->peerfd,str,strlen(str));
    return;
}
