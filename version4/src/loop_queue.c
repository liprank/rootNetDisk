#include "../include/loop_queue.h"
#include <func.h>

// 循环队列初始化
queue_t* qInit() {
    queue_t* q = (queue_t*)calloc(1, sizeof(queue_t));
    if (!q) return NULL;

    q->capacity = WAIT_SEC;
    q->front = 0;
    q->rear = 0;
    q->size = 0;

    // 为 list 数组的每个元素分配内存
    for (int i = 0; i < WAIT_SEC; i++) {
        q->list[i].array = (int*)calloc(MAX_LEN, sizeof(int));
        if (!q->list[i].array) {
            // 如果分配失败，需要释放之前分配的所有内存
            for (int j = 0; j < i; j++) {
                free(q->list[j].array);
            }
            free(q);
            return NULL;
        }
    }

    return q;
}

// 判空
bool isQueueEmpty(queue_t* q) {
    return q->size == 0;
}

// 判满
bool isQueueFull(queue_t* q) {
    return q->size == q->capacity;
}

// 插入节点
void enQueue(queue_t* q, int* fdlist, int len) {
    if (isQueueFull(q)) {
        printf("queue is full\n");//这里出现错误
        return;
    }

    for (int i = 0; i < len; i++) { // 使用传入的长度 len
        q->list[q->rear].array[i] = fdlist[i];
    }
    q->list[q->rear].length = len; // 更新元素长度

    q->rear = (q->rear + 1) % q->capacity;
    q->size++;
}

// 删除节点
void deQueue(queue_t* q) {
    if (isQueueEmpty(q)) {
        printf("queue is empty\n");
        return;
    }

    // 释放 front 元素的内存
    //free(q->list[q->front].array); 不需要释放内存，只需要更新front指针
    q->front = (q->front + 1) % q->capacity;
    q->size--;
}

// 摧毁队列
void destroyQueue(queue_t* q) {
    if (!q) return;

    for (int i = 0; i < q->capacity; i++) {
        free(q->list[i].array);
    }
    free(q);
}

int* frontCheck(queue_t* q) {
    if (isQueueEmpty(q)) {
        printf("queue is empty\n");
        return NULL;
    }
    return q->list[q->front].array;
}

int* rearCheck(queue_t* q) {
    if (isQueueFull(q)) {
        printf("queue is full\n");
        return NULL;
    }
    return q->list[q->rear].array;
}

void clearValue(queue_t* q,int position,int value){

	printf("要清除的列：%d，要清除的值：%d\n",position,value);
	int len = q->list[position].length;
	for(int i = 0; i < len; i++){
		if(q->list[position].array[i] == value){
			q->list[position].array[i] = 0;	
		}
	}
}

void printQueue(queue_t* q){
	for(int i = 0; i < WAIT_SEC; i++){
		printf("第%d列：",i);
		int len = q->list[i].length;
		for(int j = 0; j < len; j++){
			printf("%d ",q->list[i].array[j]);
		}
		printf("\n");
	}
	printf("\n");
}

// 主函数用于测试
//int main() {
//    queue_t* q = qInit();
//    int temp[5] = {1, 2, 3, 4, 5};
//    enQueue(q, temp, 5);
//	clearValue(q,0,3);
//    int* front = frontCheck(q);
//    if (front) {
//        for (int i = 0; i < 5; i++) {
//            printf("%d ", front[i]);
//        }
//        printf("\n");
//    }
//
//
//	
//    deQueue(q);
//    // 继续测试其他操作...
//
//    // 测试完成后销毁队列
//    destroyQueue(q);
//
//    return 0;
//}
