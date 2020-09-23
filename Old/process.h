#ifndef PROCESS_H
#define PROCESS_H
#include<stdio.h>
#include<string.h>
#include "hash.h"
#include "cache.h"
#include "idtrans.h"
#include <Winsock2.h>  
  
#pragma comment(lib,"Ws2_32.lib")//����Sockets��ؿ�

char buf[10005];
char tmp[10005];
char ret[10005];
char send_to_query[10005];
char domain_name[10005];
extern struct Cache c;
/*
void trans()
{
	int len=0;
	*(unsigned int *)(buf+len)=(unsigned int)0x0001175e;
	len+=sizeof(unsigned int);
	*(unsigned int *)(buf+len)=(unsigned int)0x00000100;
	len+=sizeof(unsigned int);
	*(unsigned int *)(buf+len)=(unsigned int)0x00000000;
	len+=sizeof(unsigned int);
	*(unsigned int *)(buf+len)=(unsigned int)0x77777703;
	len+=sizeof(unsigned int);
	*(unsigned int *)(buf+len)=(unsigned int)0x69616205;
	len+=sizeof(unsigned int);
	*(unsigned int *)(buf+len)=(unsigned int)0x63037564;
	len+=sizeof(unsigned int);
	*(unsigned int *)(buf+len)=(unsigned int)0x00006d6f;
	len+=sizeof(unsigned int);
	*(unsigned int *)(buf+len)=(unsigned int)0x00010001;
	len+=sizeof(unsigned int);
}*/

struct sockaddr_in local;//������ַ��ؽṹ��  
struct sockaddr_in from;//�ͻ��˵�ַ��ؽṹ��  
struct sockaddr_in server;
int server_len;
extern SOCKET so;
SOCKET socket2;
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
	printf("11");
	socket2 = socket(AF_INET, SOCK_DGRAM, 0);
	bind(socket2, (struct sockaddr*)&local, sizeof(local));
	
	unsigned int non_block = 1;
	ioctlsocket(socket1, FIONBIO, (u_long *)&non_block);
	ioctlsocket(socket2, FIONBIO, (u_long *)&non_block);
    return socket1;
}

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
	printf("recv from local:");
	for(;i<query_len;i++) printf("%d ",tmp[i]);puts("");
	int now_ptr = 0;
	unsigned short qid = (*(unsigned short *)(tmp + now_ptr));
	qid = (qid << 8) | (qid >> 8);
	now_ptr += sizeof(unsigned short);
	int qflag = (*(unsigned short *)(tmp + now_ptr));
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
	for (; j < len - 1; i++, j++) send_to_query[j] = tmp[i];
	send_to_query[j] = '\0';
	printf("recv query:");
	for (j = 0; j< len -1; j++) printf("%c",send_to_query[j]);puts("");
	long long ip = find_IP(send_to_query, len - 1);
	if(ip == -1)
	{
		ip = query_in_cache(send_to_query, c);// logic
		if(ip != -1) in_cache = true;
	}
	if (ip != -1)
	{
		printf("find ip:%lld in ",ip);
		printf("%s\n",in_cache?"cache":"table");
		memcpy(ret, buf, sizeof(buf));
		if (ip == 0)
		{
			int ret_len = 0;
			
//				*(unsigned short *)(ret + ret_len) = (unsigned short)qid;
			ret_len += sizeof(unsigned short);
			//flag
			if (in_cache)
			{
				*(unsigned short *)(ret + ret_len) = htons((unsigned short)0x8083);
			}
			else
			{
				*(unsigned short *)(ret + ret_len) = htons((unsigned short)0x8483);
			}
			ret_len += sizeof(unsigned short);
//				*(unsigned short *)(ret + ret_len) = (unsigned short)0x0000;
			ret_len += sizeof(unsigned short);
			*(unsigned short *)(ret + ret_len) = (unsigned short)0x0000;
			
			sendto(so, ret, ret_len, 0, (struct sockaddr*)&from, fromlen);
		}
		else
		{
			int ret_len = 0;
			
//				*(unsigned short *)(ret + ret_len) = (unsigned short)qid;
			ret_len += sizeof(unsigned short);
			//flag
			if (in_cache)
			{
				*(unsigned short *)(ret + ret_len) = htons((unsigned short)0x8080);
			}
			else
			{
				*(unsigned short *)(ret + ret_len) = htons((unsigned short)0x8480);
			}
			ret_len += sizeof(unsigned short);
//				*(unsigned short *)(ret + ret_len) = (unsigned short)0x0000;
			ret_len += sizeof(unsigned short);
			*(unsigned short *)(ret + ret_len) = htons((unsigned short)0x0001); //ancount
			
			ret_len = query_len;
			*(unsigned short *)(ret + ret_len) = htons((unsigned short)0xc00c); //name
			ret_len += sizeof(unsigned short);
			*(unsigned short *)(ret + ret_len) = htons((unsigned short)0x0001); //type
			ret_len += sizeof(unsigned short);
			*(unsigned short *)(ret + ret_len) = htons((unsigned short)0x0001); //class
			ret_len += sizeof(unsigned short);
			*(unsigned long *)(ret + ret_len) = htonl((unsigned long)0x0002a300); //ttl
			ret_len += sizeof(unsigned long);
			*(unsigned short *)(ret + ret_len) = htons((unsigned short)0x0004); //hdlength
			ret_len += sizeof(unsigned short);
			*(unsigned long *)(ret + ret_len) = htonl((unsigned long)ip); //hdata
			ret_len += sizeof(unsigned long);
			
			sendto(so, ret, ret_len, 0, (struct sockaddr*)&from, fromlen);
		}
	}
	else
	{
		printf("Unknown domain name. Send query to DNS server.\n");
		int new_ID = get_new_ID((int)qid, from);
		printf("%d->%d\n",qid,new_ID);
		*(unsigned short *)(buf) = htons((unsigned short)new_ID);
		printf("send to server:");
		for(i = 0; i < query_len; i++) printf("%d ",buf[i]);puts("");
		sendto(socket2, buf, query_len, 0, (struct sockaddr*)&server, sizeof(server));
	}
}
void Recv_from_server()
{
    int fromlen = sizeof(from);
    int server_len = sizeof(server);
    int query_len = recvfrom(socket2, buf, sizeof(buf), 0, (struct sockaddr*)&server, &server_len);
    if (query_len == -1) return;
	memcpy(tmp, buf, sizeof(buf));
	int i = 0, j = 0;
	printf("recv from server:");
	for(;i<query_len;i++) printf("%d ",tmp[i]);puts("");
	int now_ptr = 0;
	unsigned short qid = (*(unsigned short *)(tmp + now_ptr));
	qid = (qid << 8) | (qid >> 8);
	now_ptr += sizeof(unsigned short);
	int qflag = (*(unsigned short *)(tmp + now_ptr));
	now_ptr += sizeof(unsigned short);
	int qcount = (*(unsigned short *)(tmp + now_ptr));
	now_ptr += sizeof(unsigned short);
	int acount = (*(unsigned short *)(tmp + now_ptr));
	now_ptr += sizeof(unsigned short);
	now_ptr += sizeof(unsigned int);
	
	int len = 0;
	while (*(tmp + now_ptr + len) != 0)
	{
		int word = *(tmp + now_ptr + len);
		*(tmp + now_ptr + len) = '.';
		len += word + 1;
	}
	i = now_ptr + 1;
	j = 0;
	for (; j < len - 1; i++, j++) send_to_query[j] = tmp[i];
	send_to_query[j] = '\0';
	now_ptr += len;
	now_ptr += sizeof(unsigned int);
	
	while (acount--)
	{
		len = 0;
		if (*(tmp + now_ptr + len) == 0xc0)
		{
			memcpy(domain_name,send_to_query,sizeof(send_to_query));
			now_ptr += 2;
		}
		else
		{
			while (*(tmp + now_ptr + len) != 0)
			{
				int word = *(tmp + now_ptr + len);
				*(tmp + now_ptr + len) = '.';
				len += word + 1;
			}
			i = now_ptr + 1;
			j = 0;
			for (; j < len - 1; i++, j++) domain_name[j] = tmp[i];
			domain_name[j] = '\0';
			now_ptr += len;
		}
		int atype = (*(unsigned short *)(tmp + now_ptr));
		now_ptr += sizeof(unsigned short);
		int aclass = (*(unsigned short *)(tmp + now_ptr));
		now_ptr += sizeof(unsigned short);
		int ttl = (*(unsigned int *)(tmp + now_ptr));
		now_ptr += sizeof(unsigned int);
		int rdlength = (*(unsigned short *)(tmp + now_ptr));
		now_ptr += sizeof(unsigned short);
		if (atype == 1)
		{
			unsigned int IP = (*(unsigned int *)(tmp + now_ptr));
			add_to_cache(domain_name, IP, ttl, c);
		}
		now_ptr += rdlength;
	}
	
	struct sockaddr_in client = ID_table[qid].client;
	*(unsigned short *)(buf) = htons((unsigned short)ID_table[qid].old_ID);
	
	printf("id:%d\n",qid);
	printf("send to local:");
	for(i = 0;i<query_len;i++) printf("%d ",buf[i]);puts("");
	 
	sendto(so, buf, query_len, 0, (struct sockaddr*)&client, sizeof(client));
    release_ID(qid);
}
#endif
