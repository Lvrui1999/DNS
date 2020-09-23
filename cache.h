#ifndef CACHE_H
#define CACHE_H
#include <string.h>
#include <stdio.h>
#include <time.h>
#include "hash.h"
#define Cachesize 512 //Cache����
#define MAX_DOMAIN_LENGTH 512

extern const int mod,p;

extern struct Cache c;

bool hash_occupy[Cachesize << 7];
int hash_id[Cachesize << 7]; 

struct Cache{
    int id[Cachesize];//cache��ÿ��������Ӧ��ϣֵ
    char name[Cachesize][MAX_DOMAIN_LENGTH];//cache��ÿ��������
    unsigned int ip[Cachesize]; //cache��ÿ���ip��ַ
    int last[Cachesize]; //cache��ÿ��lruֵ
    int ttl[Cachesize]; //cache��ÿ���ttl
    int ti[Cachesize];  //cache��ÿ������ʱ��
    int size; //��ǰcache��С
};

int lru(){ //lru�㷨
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



void output_cache(){
	int i;
	printf("================================i am cache===============================\n");
	printf("c.size=%d\n",c.size);
    for(i = 0; i < c.size; i++){
        printf("%s %u %d\n",c.name[i],c.ip[i],c.ttl[i]);
    }
	printf("=========================================================================\n");
}


void add_to_cache(char* name, unsigned int ip, int ttl, int now_time){ //����Ŀ����cache��
    if(c.size < Cachesize){
        int hash_num = get_hash(name,strlen(name));
        while(hash_occupy[hash_num]){
            hash_num++;
            hash_num %= mod;
            if(!hash_num){
                hash_num += mod;
            }
        }
        hash_occupy[hash_num] = 1;
        hash_id[hash_num] = c.size;
        strcpy(c.name[c.size],name);
        c.id[c.size] = hash_num;
        c.ip[c.size] = ip;
        c.ttl[c.size] = ttl;
        c.ti[c.size] = now_time;
        c.size++;
    }
    else{
        int pos = lru(c);
        int last = c.id[pos];
        hash_occupy[last] = 0;
        int hash_num = get_hash(name,strlen(name));
        while(hash_occupy[hash_num]){
            hash_num++;
            hash_num %= mod;
            if(!hash_num){
                hash_num += mod;
            }
        }
        hash_occupy[hash_num] = 1;
        hash_id[hash_num] = pos;
        strcpy(c.name[pos],name);
        c.id[pos] = hash_num;
        c.ip[pos] = ip;
        c.ttl[pos] = ttl;
        c.ti[pos] = now_time;
    }
}

long long query_in_cache(char * name){ //��cache�в�ѯ
    int hash_num = get_hash(name,strlen(name));
    while(hash_occupy[hash_num]){
        if(strcmp(c.name[hash_id[hash_num]],name) == 0){
            if(c.ttl[hash_id[hash_num]] < time(0) - c.ti[hash_id[hash_num]]){
                return -1;
            }
            int res_ttl = c.ttl[hash_id[hash_num]] - (int)time(0) + c.ti[hash_id[hash_num]];
            return c.ip[hash_id[hash_num]] | ((long long)res_ttl << 32);
        }
        hash_num++;
        hash_num %= mod;
        if(!hash_num){
            hash_num += mod;
        }
    }
    return -1;
}


#endif
