#ifndef PROCESS_H
#define PROCESS_H
#include<stdio.h>
#include<string.h>
#include "hash.h"
#include "cache.h"
#include <Winsock2.h>  
  
#pragma comment(lib,"Ws2_32.lib")//����Sockets��ؿ�

char buf[100005];
char tmp[100005];
char ret[100005];
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
extern struct Cache c;
struct sockaddr_in local;//������ַ��ؽṹ��  
struct sockaddr_in from;//�ͻ��˵�ַ��ؽṹ��  
struct sockaddr_in server;
int server_len;
SOCKET so;
int init_socket(){
    SOCKET socket1;  
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 1), &wsaData);
    server.sin_family = AF_INET;  
    server.sin_port = htons(53); 
    server.sin_addr.s_addr = inet_addr("192.168.1.1"); //DNS server
    server_len = sizeof(server);
    local.sin_family = AF_INET;  
    local.sin_port = htons(53);  
    local.sin_addr.s_addr = INADDR_ANY;
    socket1 = socket(AF_INET, SOCK_DGRAM, 0);  
    bind(socket1, (struct sockaddr*)&local, sizeof(local));
    return socket1;
}

void Recv()
{
//	char buf[1024];
    int fromlen = sizeof(from);
    int query_len;
	while((query_len = recvfrom(so, buf, sizeof(buf), 0, (struct sockaddr*)&from, &fromlen)) == SOCKET_ERROR);
	bool query;
	bool in_cache = false;
	memcpy(tmp, buf,sizeof(buf));
	int now_ptr = 0;
	int qid = (*(unsigned short *)(tmp + now_ptr));
	now_ptr += sizeof(unsigned short);
	int qflag = (*(unsigned short *)(tmp + now_ptr));
	now_ptr += sizeof(unsigned short);
	if (qflag & (0x80)) query = false;
	else query = true;
	if (query)
	{
		printf("query\n");
		/* \0 */
		int len = 0;
		while (*(tmp + now_ptr + len) != 0)
		{
			int word = *(tmp + now_ptr + len);
			*(tmp + now_ptr + len) = '.';
			len += word + 1;
		}
		long long ip = find_IP(tmp + now_ptr + 1, len - 1);
		if(ip == -1)
		{
			ip = query_in_cache(tmp + now_ptr + 1, c);
			if(ip != -1) in_cache = true;
		}
		if (ip != -1)
		{
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
			sendto(so, buf, sizeof(buf), 0, (struct sockaddr*)&server, server_len);
		}
	}
	else 
	{
		printf("keyi.\n");
	}
}

#endif

