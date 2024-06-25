#ifndef __CONFIG_H__
#define __CONFIG_H__

#include "hashtable.h"

#define IP "ip"
#define PORT "port"

void readConfig(const char* filename, HashTable * ht);

#endif

