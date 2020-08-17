#include <stdio.h>
#include "cache.h"
#include <Winsock2.h>   
  
#pragma comment(lib,"Ws2_32.lib")//连接Sockets相关库  
  
void main()  
{  
    SOCKET socket1;  
    WSADATA wsaData;  
    if (WSAStartup(MAKEWORD(2, 1), &wsaData)) //初始化  
    {  
        printf("Winsock can't establish!\n");  
        WSACleanup();  
        return;  
    }  
    printf("Start Socket\n");  
    struct sockaddr_in local;//本机地址相关结构体  
    struct sockaddr_in from;//客户端地址相关结构体  
    struct sockaddr_in server;  
    int len = sizeof(server);  
    server.sin_family = AF_INET;  
    server.sin_port = htons(53); ///server的监听端口   
    server.sin_addr.s_addr = inet_addr("192.168.1.1"); ///server的地址  
    int fromlen = sizeof(from);  
    local.sin_family = AF_INET;  
    local.sin_port = htons(53); ///监听端口   
    local.sin_addr.s_addr = INADDR_ANY; ///本机   
    socket1 = socket(AF_INET, SOCK_DGRAM, 0);  
    bind(socket1, (struct sockaddr*)&local, sizeof(local));//绑定SOCKET，此步关键  
    char buffer[1024] = "\0";  
    if (recvfrom(socket1, buffer, sizeof(buffer), 0, (struct sockaddr*)&from, &fromlen) != SOCKET_ERROR)//阻塞接受客户端的请求  
    {  
        printf("Start\n");  
    }  
    while (1)  
    {  
        sendto(socket1, buffer, sizeof(buffer), 0, (struct sockaddr*)&from, fromlen);//发数据给客户端，由于是  
        // Sleep(200);  




        
    }  
    closesocket(socket1);  
    WSACleanup();  
}