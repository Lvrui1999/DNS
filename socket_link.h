#include <Winsock2.h>
#include <stdio.h>
#pragma comment(lib,"ws2_32.lib")



int init(){
	WSADATA wsd;
    SOCKET s;
    SOCKADDR_IN sRecvAddr,sSendAddr;
    USHORT uPort = 1401;
    CHAR szBuf[1024] = { 0 };
    int nBufLen = 1024,nResult = 0, nSenderAddrSize = sizeof(sSendAddr);
    nResult = WSAStartup(MAKEWORD(2, 2), &wsd);
    if (nResult != NO_ERROR)
    {
        // printf("WSAStartup failed:%d\n", WSAGetLastError());
        return -1;
    }

    s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (s == INVALID_SOCKET)
    {
        // printf("socket failed:%d\n", WSAGetLastError());
        return -2;
    }
    sRecvAddr.sin_family = AF_INET;
    sRecvAddr.sin_port = htons(uPort);
    sRecvAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
    nResult = bind(s, (SOCKADDR *)&sRecvAddr, sizeof(sRecvAddr));
    if (nResult != 0)
    {
        // printf("bind failed:%d\n", WSAGetLastError());
        return -3;
    }
    // printf("Waiting recv data...\n");
    nResult = recvfrom(s, szBuf, nBufLen, 0, (SOCKADDR *)&sSendAddr, &nSenderAddrSize);
    if (nResult == SOCKET_ERROR)
    {
        printf("recvfrom failed:%d\n", WSAGetLastError());
    }
    else{
        printf("SenderIP  :%s\n", inet_ntoa(sSendAddr.sin_addr));
        printf("SenderData:%s\n", szBuf);
    }
    // 关闭Socket连接
    nResult = closesocket(s);
    if (nResult == SOCKET_ERROR)
    {
        printf("closesocket failed:%d\n", WSAGetLastError());
        return 1;
    }
    // 清理Socket
    WSACleanup();

    system("pause");
    return 0;
}


