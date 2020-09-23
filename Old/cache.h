#ifndef CACHE_H
#define CACHE_H
#include <string.h>
#include <stdio.h>
#define CacheSize 1024 //Cache大小

struct Cache{
    char* name[CacheSize]; //每条的域名
    unsigned int ip[CacheSize]; //每条的ip地址
    int last[CacheSize]; //每条的lru值
    int ttl[CacheSize]; //每天的ttl
    int size; //现在cache中的size
};

int lru(struct Cache c){ //lru维护
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

void add_to_cache(char* name, unsigned int ip, int ttl, struct Cache c){ //向cache中添加条目
    if(c.size < CacheSize){
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

long long query_in_cache(char * name, struct Cache c){ //向cache中询问
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

void output(struct Cache c){
    for(int i = 0; i < c.size; i++){
        printf("%s %u %d",c.name[i],c.ip[i],c.ttl[i]);
    }
}

void maintain(struct Cache c){ //维护cache
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
