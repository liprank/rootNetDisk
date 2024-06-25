#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define MAX_LEN 10
#define WAIT_SEC 30

typedef struct elements_s {
    int* array;//用指针代替固定长度数组，固定长度数组无法直接赋值
    int length;
} element_t;

typedef struct queue_s {
    int front;
    int rear;
    int capacity; // 最大容量
    int size;     // 当前大小
    element_t list[WAIT_SEC]; // 队列中存储集合，记录当前活跃的文件描述符
} queue_t;


//循环队列初始化
queue_t* qInit(); 

//判空
bool isQueueEmpty(queue_t* q);

//判满
bool isQueueFull(queue_t* q);

//插入节点
void enQueue(queue_t* q,int* fdlist, int len);

//删除节点
void deQueue(queue_t* q);

//摧毁队列
void destroyQueue(queue_t* q);

//打印队列
void printQueue(queue_t* q);

int* frontCheck(queue_t* q);

int* rearCheck(queue_t* q);

void clearValue(queue_t* q,int position,int value);
