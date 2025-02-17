#ifndef HASH_H
#define HASH_H

#include <stdio.h>
#include <string.h>
#define DNS_Length 300
#define Total_Record 6000
#define Backup_start_num 5001 
#define MAX_FILE_LENGTH 128
#define bool int
#define true 1
#define false 0

struct HASH_TABLE
{
	char Name[DNS_Length + 5];
	unsigned int IP_encoded;
	int Next_id;
	int occupied;
};
struct HASH_TABLE Hash_table[Total_Record + 5];

extern char PATH[MAX_FILE_LENGTH];
extern const int p, mod;
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
char IP_str[DNS_Length + 5], DN_str[DNS_Length + 5];
int get_hash(char *s, int len)
{
	int i = 0;
	int Hash_num = 0;
	for (; i < len ; i++)
		Hash_num = (1ll * Hash_num * p % mod + s[i]) % mod;
	return Hash_num + 1;
}
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
	printf("%lf %lf\n",1.0 * cnt_success / success_num, 1.0 * cnt_fail / mod);
}
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
