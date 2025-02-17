#ifndef HASH_H
#define HASH_H

#include <stdio.h>
#include <string.h>
#define MAX_DOMAIN_LENGTH 512 
#define Total_Record 6000
#define Backup_start_num 5001 
#define MAX_FILE_LENGTH 128
#define bool int
#define true 1
#define false 0
#pragma once

struct HASH_TABLE
{
	char Name[MAX_DOMAIN_LENGTH + 5];//域名
	unsigned int IP_encoded;//压缩后的IP
	int Next_id;//链地址法标识,下一个表项
	bool occupied;//是否被占用标志
};
struct HASH_TABLE Hash_table[Total_Record + 5];

extern char PATH[MAX_FILE_LENGTH];
extern const int p, mod;
//将压缩后的IP地址(unsigned int IP_encoded)
//还原为字符串格式(char *s)
void decode(unsigned int IP_encoded, char *s)
{
	int len = 0, i = 0;
	int addr[4];
	for (; i < 4; i++)
	{
		addr[i] = (IP_encoded << (8 * i) ) >> 24;
	}
	
	for (i = 3; i >= 0; i--)
	{
		while (addr[i])
		{
			s[len++] = addr[i] % 10 + '0';
			addr[i] /= 10;
		}
		if (i == 0) break;
		s[len++] = '.';
	}
	
	for (i = 0; i < len; i++)
		if (i < len - i + 1)
		{
			char t = s[i];
			s[i] = s[len - i - 1];
			s[len - i - 1] = t;
		}
	s[len++] = '\0';
}
//将字符串格式的IP地址(char *s)压缩为一个无符号32位整数
unsigned int encode(char *s, int len)
{
	int i = 0;
	int IP_encoded = 0, num = 0;
	for (; i < len; i++)
	{
		if (s[i] == '.')
		{
			IP_encoded = (IP_encoded << 8) | num;
			num = 0;
		}
		else num = num * 10 + s[i] - '0';
	}
	IP_encoded = (IP_encoded << 8) | num;
	return IP_encoded;
}
char IP_str[MAX_DOMAIN_LENGTH + 5], DN_str[MAX_DOMAIN_LENGTH + 5];
//求字符串s的哈希值
int get_hash(char *s, int len)
{
	int i = 0;
	int Hash_num = 0;
	for (; i < len ; i++)
		Hash_num = (1ll * Hash_num * p % mod + s[i]) % mod;
	return Hash_num + 1;
}
//加载本地的"域名-IP地址"转换表
void load_dns_table()
{
	freopen(PATH,"r",stdin);
	int Backup = Backup_start_num + 1;
	while (scanf("%s%s",IP_str,DN_str)!=EOF)
	{
		int IP_encoded = encode(IP_str, strlen(IP_str));
		int Hash_number = get_hash(DN_str, strlen(DN_str));
		if (Hash_table[Hash_number].occupied == false)
		{
			Hash_table[Hash_number].occupied = true;
			memcpy(Hash_table[Hash_number].Name, DN_str, sizeof(DN_str));
			Hash_table[Hash_number].IP_encoded = IP_encoded;
		}
		//用链地址法来避免冲突
		else
		{
			Hash_table[Backup] = Hash_table[Hash_number];
			Hash_table[Hash_number].Next_id = Backup;
			Hash_table[Hash_number].occupied = true;
			memcpy(Hash_table[Hash_number].Name, DN_str, sizeof(DN_str));
			Hash_table[Hash_number].IP_encoded = IP_encoded;
			Backup++;
		}
	}
}
//检查哈希效果
void hash_test()
{
	int i = 1;
	int used=0;
	int cnt_success = 0;
	int cnt_fail = 0;
	int success_num = 0;
	for (;i <= mod; i++)
	{
		int x = i;
		int depth = 1;
		while (Hash_table[x].occupied == true)
		{
			cnt_success += depth;
			success_num++;
			x = Hash_table[x].Next_id;
			depth++;
		}
		cnt_fail += depth;
		used+=(depth>1);
	}
	printf("%d\n",used);
	//查找成功的平均查找次数与查找失败的平均查找次数
	printf("%lf %lf\n",1.0 * cnt_success / success_num, 1.0 * cnt_fail / mod);
}
//在域名-IP地址转换表中查找IP
long long find_IP(char *s, int len)
{
	int Hash_number = get_hash(s, len);
	int now = Hash_number;
	while (Hash_table[now].occupied == true)
	{
		if (strcmp(Hash_table[now].Name,s) == 0) return Hash_table[now].IP_encoded;
		now = Hash_table[now].Next_id;
	}
	return -1;
}
#endif
