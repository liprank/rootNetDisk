#ifndef __HASH_TABLE_H__
#define __HASH_TABLE_H__
#include <stdio.h>  
#include <stdlib.h>
#include <string.h>  

  
#define MAX_SIZE 100  
#define EMPTY NULL  
#define QEMPTY -1
  
// 键值对结构体  
typedef struct {  
    char key[50];  
    void * value;  
} KeyValue;  

// 哈希表结构体  
typedef struct {  
    KeyValue table[MAX_SIZE];  
    int size;  
} HashTable;  

// hash函数
unsigned int hash(const char *key);
  
// 初始化哈希表  
void initHashTable(HashTable *ht);  
  
// 插入键值对  
void insert(HashTable *ht, const char *key, void * value);

 
// 查找值  
void * find(HashTable *ht, const char *key); 

// 删除键值对  
void erase(HashTable *ht, const char *key); 

// 打印哈希表内容（仅用于调试）  
void printHashTable(HashTable *ht);   

// 销毁哈希表
void destroyHashTable(HashTable *ht);


/****************************************loop_queue***********************************************/
//键值对结构体循环队列
typedef struct {  
    int Key;  
    int value;  
} qKeyValue;  

// 哈希表结构体  
typedef struct {  
    qKeyValue table[MAX_SIZE];  
    int size;  
} qHashTable;  

//hash函数
unsigned int qhash(int key);

//初始化哈希表
void qinitHashTable(qHashTable *ht);

//插入键值对，循环链表版
void qinsert(qHashTable *ht, int key, int value);

//查找值
void* qfind(qHashTable *ht, int key);

//删除键值对，循环队列版
void qerase(qHashTable *ht, int key); 

//打印哈希表内容
void qprintHashTable(qHashTable *ht);

//销毁哈希表
void qdestroyHashTable(qHashTable *ht);



#endif

