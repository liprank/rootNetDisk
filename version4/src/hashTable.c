#include "../include/hashTable.h" 

// hash函数
unsigned int hash(const char *key) {  
    unsigned int hashVal = 0;  
    while (*key != '\0') {  
        hashVal = (hashVal << 5) + hashVal + *key++;  
    }  
    return hashVal % MAX_SIZE;  
}  
  
// 初始化哈希表  
void initHashTable(HashTable *ht) {  
    ht->size = 0;  
    for (int i = 0; i < MAX_SIZE; i++) {  
        strcpy(ht->table[i].key, "");  
        ht->table[i].value = EMPTY;  
    }  
}  
  
// 插入键值对  
void insert(HashTable *ht, const char *key, void * value) {  
    unsigned int index = hash(key);  
    // 线性探测解决冲突  
    while (ht->table[index].value != EMPTY) {  
        index = (index + 1) % MAX_SIZE;  
        if (strcmp(ht->table[index].key, key) == 0) {  
            // 如果键已存在，更新值  
            ht->table[index].value = value;  
            return;  
        }  
    }  
    // 插入新键值对  
    strcpy(ht->table[index].key, key);  
    ht->table[index].value = value;  
    ht->size++;  
}  
  
// 查找值  
void * find(HashTable *ht, const char *key) {  
    unsigned int index = hash(key);  
    while (ht->table[index].value != EMPTY) {  
        if (strcmp(ht->table[index].key, key) == 0) {  
            return ht->table[index].value;  
        }  
        index = (index + 1) % MAX_SIZE;  
    }  
    return EMPTY; // 键不存在  
}  
  
// 删除键值对  
void erase(HashTable *ht, const char *key) {  
    unsigned int index = hash(key);  
    while (ht->table[index].value != EMPTY) {  
        if (strcmp(ht->table[index].key, key) == 0) {  
            strcpy(ht->table[index].key, "");  
            free(ht->table[index].value);
            ht->table[index].value = EMPTY;  
            ht->size--;  
            return;  
        }  
        index = (index + 1) % MAX_SIZE;  
    }  
}  
  
// 打印哈希表内容（仅用于调试）  
void printHashTable(HashTable *ht) {  
    printf("hashtable's content:\n");
    for (int i = 0; i < MAX_SIZE; i++) {  
        if (ht->table[i].value != EMPTY) {  
            printf("Key: %s, Value: %s\n", ht->table[i].key, (const char*)ht->table[i].value);  
        }  
    }  
    printf("\n");
}  

void destroyHashTable(HashTable *ht) {
    for(int i = 0; i < MAX_SIZE; i++) {
        if(ht->table[i].value != EMPTY) {
            strcpy(ht->table[i].key, "");  
            free(ht->table[i].value);
            ht->table[i].value = EMPTY;  
            ht->size--;  
        }
    }
}
/***************************** 哈希表操作 ***********************************/

// 简单的哈希函数，您可能需要根据实际情况调整
unsigned int qhash(int key) {
    return key % MAX_SIZE;
}

// 初始化哈希表
void qinitHashTable(qHashTable *ht) {
    ht->size = 0;
    for (int i = 0; i < MAX_SIZE; ++i) {
        ht->table[i].Key = QEMPTY;  // 初始化 Key 为 EMPTY
        ht->table[i].value = QEMPTY;
    }
}

// 插入键值对，使用链表解决冲突
void qinsert(qHashTable *ht, int key, int value) {
    if (ht->size >= MAX_SIZE) {
        fprintf(stderr, "Hash table is full.\n");
        return;
    }
    
    unsigned int index = qhash(key);
    // 查找空位或现有项
    while (ht->table[index].Key != QEMPTY && ht->table[index].Key != key) {
        index = (index + 1) % MAX_SIZE;  // 线性探测
    }
    
    if (ht->table[index].Key == QEMPTY) {
        ht->table[index].Key = key;
        ht->table[index].value = value;
        ht->size++;
    } else {
        // 如果键已存在，更新值
        ht->table[index].value = value;
    }
}

// 删除键值对
void qerase(qHashTable *ht, int key) {
    unsigned int index = qhash(key);
    while (ht->table[index].Key != QEMPTY) {
        if (ht->table[index].Key == key) {
            ht->table[index].Key = QEMPTY;
            ht->table[index].value = QEMPTY;
            ht->size--;
            return;
        }
        index = (index + 1) % MAX_SIZE;  // 线性探测
    }
}

void* qfind(qHashTable *ht, int key) {
    unsigned int index = qhash(key);  // 从哈希表获取初始索引
    int original_index = index;       // 保存原始索引以检测循环

    while (ht->table[index].Key != QEMPTY) {
        if (ht->table[index].Key == key) {
            // 找到键，返回值的指针
            return (void*)&ht->table[index].value;
        }
        index = (index + 1) % MAX_SIZE;  // 线性探测解决冲突
        if (index == original_index) {
            // 如果我们回到了起始索引，表示已经遍历完整个哈希表
            break;
        }
    }

    // 没有找到键，返回NULL
    return NULL;
}

// 打印哈希表内容
void qprintHashTable(qHashTable *ht) {
    for (int i = 0; i < MAX_SIZE; ++i) {
        if (ht->table[i].Key != QEMPTY) {
            printf("Key: %d, Value: %d\n", ht->table[i].Key, ht->table[i].value);
        }
    }
}

// 销毁哈希表（目前只是清空哈希表）
void qdestroyHashTable(qHashTable *ht) {
    qinitHashTable(ht);  // 重置哈希表
}

