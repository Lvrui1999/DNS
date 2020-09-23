#ifndef PROCESS_H
#define PROCESS_H
#include<stdio.h>
#include<string.h>
#include <time.h>
#include "hash.h"
#include "cache.h"
#include "idtrans.h"
#include <Winsock2.h>  
  
#pragma comment(lib,"Ws2_32.lib")

char buf[10005];
char tmp[10005];
char ret[10005];
int at_end[10005];
char send_to_query[MAX_DOMAIN_LENGTH];
char domain_name[MAX_DOMAIN_LENGTH];
extern struct Cache c;
extern int debuglevel; 

struct sockaddr_in local;//本地socket
struct sockaddr_in from;//客户端socket
struct sockaddr_in server;
int server_len;
extern SOCKET so;
SOCKET socket2;
//初始化socket
int init_socket(){
    SOCKET socket1;  
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 1), &wsaData);
    server.sin_family = AF_INET;  
    server.sin_port = htons(53); 
    server.sin_addr.s_addr = inet_addr("10.3.9.4"); //DNS server
    server_len = sizeof(server);
    local.sin_family = AF_INET;  
    local.sin_port = htons(53);  
    local.sin_addr.s_addr = inet_addr("127.0.0.1");
    socket1 = socket(AF_INET, SOCK_DGRAM, 0);  
    bind(socket1, (struct sockaddr*)&local, sizeof(local));
	socket2 = socket(AF_INET, SOCK_DGRAM, 0);
	bind(socket2, (struct sockaddr*)&local, sizeof(local));
	
	unsigned int non_block = 1;
	ioctlsocket(socket1, FIONBIO, (u_long *)&non_block);
	ioctlsocket(socket2, FIONBIO, (u_long *)&non_block);
    return socket1;
}

void get_url(char *s, int *pnow_ptr, int len, char *domain_name, int *pdomain_len)//解析可能存在的CNAME类型导致的递归问题
{
	int now_ptr = *pnow_ptr;
	int domain_len = *pdomain_len;
	int i;
	while ((unsigned char)*(s + now_ptr + len) != 0)
	{
		if (((unsigned char)*(s + now_ptr + len) & 0xc0) != 0xc0)
		{
			int word = *(s + now_ptr + len);
			if(domain_len) *(domain_name + domain_len++) = '.';
			for (i = len + 1; i <= len + word; i++) *(domain_name + domain_len++) = *(s + now_ptr + i);
			len += word + 1;
		}
		else
		{
			char tmp[305];
			int tmp_len = 0;
			int temp = ((*(s + now_ptr + len) & 0x3f) << 8 )| (*(s + now_ptr + len + 1));
			get_url(s, &temp, 0, tmp, &tmp_len);
			if(domain_len) *(domain_name + domain_len++) = '.';
			for (i = 0; i < tmp_len; i++) *(domain_name + domain_len++) = *(tmp + i);
			len += 2;
			if (at_end[now_ptr + len -1]) break;
		}
	}
	domain_name[domain_len] = '\0';
	*pdomain_len = domain_len;
	*pnow_ptr += len;
}

//收到本地消息
void Recv_from_local()
{
//	char buf[1024];
    int fromlen = sizeof(from);
    int server_len = sizeof(server);
    int query_len;
	query_len = recvfrom(so, buf, sizeof(buf), 0, (struct sockaddr*)&from, &fromlen);
	if(query_len == -1) return;
	bool query, in_cache = false;
	memcpy(tmp, buf, sizeof(buf));
	int i = 0, j = 0;

	if (debuglevel >= 1)
	{
		printf("recv from local:");
		for(;i<query_len;i++) printf("%x ",(unsigned char)tmp[i]);puts("");
	}

	int now_ptr = 0;
	int qid = htons((*(unsigned short *)(tmp + now_ptr)));
	now_ptr += sizeof(unsigned short);
	int qflag = htons((*(unsigned short *)(tmp + now_ptr)));
	now_ptr += sizeof(unsigned short);
	now_ptr += sizeof(unsigned long long);
	/* \0 */
	int len = 0;
	while (*(tmp + now_ptr + len) != 0)
	{
		int word = *(tmp + now_ptr + len);
		*(tmp + now_ptr + len) = '.';
		len += word + 1;
	}
	i = now_ptr + 1;
	j = 0;
	for (; j < len - 1; i++, j++) send_to_query[j] = tmp[i];//获取域名
	send_to_query[j] = '\0';

	if (debuglevel >= 1)
	{
		printf("recv query:");
		for (j = 0; j< len -1; j++) printf("%c",send_to_query[j]);puts("");
	}

	long long ip = find_IP(send_to_query, len - 1);
	long long ttl = 0;
	if(ip == -1)
	{
		long long res = query_in_cache(send_to_query);
		ip = res & 0xffffffff;
		ttl = res >> 32;
		if (debuglevel >= 1)
			printf("res:%lld ip:%lld ttl:%lld\n",res,ip,ttl);
		if(res != -1) in_cache = true;
	}
	if (ip != -1 && ip != 0xffffffff)
	{

		if (debuglevel >= 0)
		{
			printf("find ip:%lld in ",ip);
			printf("%s\n",in_cache?"cache":"table");
		}

		memcpy(ret, buf, sizeof(buf));
		if (ip == 0)//屏蔽
		{
			int ret_len = 0;
			
			ret_len += sizeof(unsigned short);//qid
			if (in_cache)//非权威答案
			{
				*(unsigned short *)(ret + ret_len) = htons((unsigned short)0x8083);
			}
			else//权威答案
			{
				*(unsigned short *)(ret + ret_len) = htons((unsigned short)0x8483);
			}
			ret_len += sizeof(unsigned short);//qflag
			ret_len += sizeof(unsigned short);//qdcount

			*(unsigned short *)(ret + ret_len) = (unsigned short)0x0000;//ancount
			
			sendto(so, ret, ret_len, 0, (struct sockaddr*)&from, fromlen);
		}
		else//查询
		{
			int ret_len = 0;

			ret_len += sizeof(unsigned short);//qid
			if (in_cache)//非权威答案
			{
				*(unsigned short *)(ret + ret_len) = htons((unsigned short)0x8080);
			}
			else//权威答案
			{
				*(unsigned short *)(ret + ret_len) = htons((unsigned short)0x8480);
			}
			ret_len += sizeof(unsigned short);//qflag
			ret_len += sizeof(unsigned short);//qdcount
			*(unsigned short *)(ret + ret_len) = htons((unsigned short)0x0001); //ancount

			ret_len = query_len;//在查询报后缀响应内容
			*(unsigned short *)(ret + ret_len) = htons((unsigned short)0xc00c); //name
			ret_len += sizeof(unsigned short);
			*(unsigned short *)(ret + ret_len) = htons((unsigned short)0x0001); //type
			ret_len += sizeof(unsigned short);
			*(unsigned short *)(ret + ret_len) = htons((unsigned short)0x0001); //class
			ret_len += sizeof(unsigned short);
			if (in_cache)
				*(unsigned long *)(ret + ret_len) = htonl((unsigned long)ttl); //ttl
			else
				*(unsigned long *)(ret + ret_len) = htonl((unsigned long)0x0002a300); //ttl
			ret_len += sizeof(unsigned long);
			*(unsigned short *)(ret + ret_len) = htons((unsigned short)0x0004); //hdlength
			ret_len += sizeof(unsigned short);
			*(unsigned long *)(ret + ret_len) = htonl((unsigned long)ip); //hdata
			ret_len += sizeof(unsigned long);

			sendto(so, ret, ret_len, 0, (struct sockaddr*)&from, fromlen);
		}
	}
	else//中继
	{
		//向DNS服务器提出询问请求
		if (debuglevel >= 0)
			printf("Unknown domain name. Send query to DNS server.\n");
		//ID转换
		int new_ID = get_new_ID((int)qid, from);
		if (debuglevel >= 1)
			printf("%d->%d\n",qid,new_ID);
		*(unsigned short *)(buf) = htons((unsigned short)new_ID);
		if (debuglevel >= 2)
		{
			printf("send to server:");
			for(i = 0; i < query_len; i++) printf("%x ",(unsigned char)buf[i]);puts("");
		}
		sendto(socket2, buf, query_len, 0, (struct sockaddr*)&server, sizeof(server));
	}
}
//收到服务器消息
void Recv_from_server()
{
    int fromlen = sizeof(from);
    int server_len = sizeof(server);
    int query_len = recvfrom(socket2, buf, sizeof(buf), 0, (struct sockaddr*)&server, &server_len);
    if (query_len == -1) return;
	memcpy(tmp, buf, sizeof(buf));
	int i = 0, j = 0;

	if(debuglevel >= 2)
	{
		printf("recv from server:");
		for(;i<query_len;i++) printf("%x ",(unsigned char)tmp[i]);puts("");
	}

	int now_ptr = 0;
	int qid = htons((*(unsigned short *)(tmp + now_ptr)));//qid
	now_ptr += sizeof(unsigned short);
	int qflag = htons((*(unsigned short *)(tmp + now_ptr)));//qflag
	now_ptr += sizeof(unsigned short);
	int qcount = htons((*(unsigned short *)(tmp + now_ptr)));//qdcount
	now_ptr += sizeof(unsigned short);
	int acount = htons((*(unsigned short *)(tmp + now_ptr)));//ancount
	now_ptr += sizeof(unsigned short);
	now_ptr += sizeof(unsigned int);//nscount arcount
	
	int len = 0;
	i = now_ptr + 1;
	j = 0;
	while (*(tmp + now_ptr + len) != 0)
	{
		int word = *(tmp + now_ptr + len);
		if(j) send_to_query[j++] = '.';
		for (i = len + 1; i <= len + word; i++) send_to_query[j++] = tmp[now_ptr + i];
		len += word + 1;
	}
	//获取域名
	send_to_query[j] = '\0';

	now_ptr += len + 1;
	now_ptr += sizeof(unsigned int);

	if (debuglevel >= 1)
		printf("query:%s\n",send_to_query);

	while (acount--)
	{
		len = 0;

		//printf("%x\n",(unsigned char)*(tmp + now_ptr + len));
		int domain_len = 0;
		at_end[now_ptr + len - 1] = 1;
		get_url(tmp, &now_ptr, len, domain_name, &domain_len);
		if (debuglevel >= 1)
			printf("domain name:%s\n",domain_name);

		int atype = htons((*(unsigned short *)(tmp + now_ptr)));//atype
		now_ptr += sizeof(unsigned short);
		int aclass = htons((*(unsigned short *)(tmp + now_ptr)));//aclass
		now_ptr += sizeof(unsigned short);
		int ttl = htonl((*(unsigned int *)(tmp + now_ptr)));//ttl
		now_ptr += sizeof(unsigned int);
		int rdlength = htons((*(unsigned short *)(tmp + now_ptr)));//rdlength
		now_ptr += sizeof(unsigned short);
		if (atype == 1)//如果是A类型，存入cache中
		{
			unsigned int IP = htonl((*(unsigned int *)(tmp + now_ptr)));

			if (debuglevel >= 1)
			{
				printf("save %s in cache\n",domain_name);
				printf("IP:%u ttl:%d\n",IP,ttl); 
			}

			int now_time = (int)time(0);
			add_to_cache(domain_name, IP, ttl, now_time);

			if(debuglevel >= 2) output_cache(c);
		}
		now_ptr += rdlength;
	}
	
	//ID转换
	struct sockaddr_in client = ID_table[qid].client;
	*(unsigned short *)(buf) = htons((unsigned short)ID_table[qid].old_ID);
	
	if (debuglevel >= 1)
		printf("id:%d\n",qid);
	if (debuglevel >= 2)
	{
		printf("send to local:");
		for(i = 0;i<query_len;i++) printf("%x ",(unsigned char)buf[i]);puts("");
	}
	 
	sendto(so, buf, query_len, 0, (struct sockaddr*)&client, sizeof(client));
	//释放ID
    release_ID(qid);
}
#endif
