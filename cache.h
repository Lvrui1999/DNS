#ifndef CACHE_H
#define CACHE_H
#include <string.h>
#define Cache_Size 1024

struct Cache{
    char* name[Cache_Size];
    unsigned int ip[Cache_Size];
    int last[Cache_Size];
    int ttl[Cache_Size];
    int size;
};

int lru(struct Cache c){
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

void add(char* name, unsigned int ip, int ttl, struct Cache c){
    if(c.size < Cache_Size){
        c.name[c.size] = name;
        c.ip[c.size] = ip;
        c.ttl[c.size] = ttl;
        c.size++;
    }
    else{
        int pos = lru(c);
        c.name[pos] = name;
        c.ip[pos] = ip;
        c.ttl[pos] = ttl;
    }
}

long long query_in_cache(char * name, struct Cache c){
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

void maintain(struct Cache c){
    int i,j;
    for(i = 0; i < c.size; i++){
        c.ttl[i]--;
        if(!(c.ttl[i] > 0)){
            for(j = i + 1; j < c.size; j++){
                c.ip[j - 1] = c.ip[j];
                c.last[j - 1] = c.last[j];
                c.name[j - 1] = c.name[j];
                c.ttl[j - 1] = c.ttl[j];
            }
            c.size--;
            i--;
        }
    }
}
#endif
