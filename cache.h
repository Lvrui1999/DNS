#include <string.h>
#define CacheSize 1024

typedef struct Cache{
    char* name[CacheSize];
    int ip[CacheSize];
    int last[CacheSize];
    int size;
};

int lru(Cache c){
    int i;
    int pos = 0;
    int pval = -1;
    for(i = 0; i < c.size; i++){
        if(c.last[i] > pval){
            pval = c.last[i];
            pos = i;
        }
    }
    return pos;
}

void add(char* name, int ip, Cache c){
    if(c.size < CacheSize){
        c.size++;
        c.name[c.size] = name;
        c.ip[c.size] = ip;
    }
    else{
        int pos = lru(c);
        c.name[pos] = name;
        c.ip[pos] = ip;
    }
}

long long query(char * name, Cache c){
    int i;
    for(i = 0; i < c.size; i++){
        if(strcmp(name,c.name[i]) == 0){
            int j;
            for(j = 0; j < c.size; j++){
                c.last[j]++;
            }
            c.last[i] = 0;
            return c.ip[i];
            
        }
    }
    return -1;
}