#ifndef IDTRANS_H
#define IDTRANS_H
#define TIMEOUT 10
#define MAX_ID_TABLE_SIZE 65540
#include <Winsock2.h>  
  
#pragma comment(lib,"Ws2_32.lib")

struct ID_TABLE//ID转换表
{
	int nx;//模拟链表,指向的下一个表项
	int old_ID;//转换前的ID
	struct sockaddr_in client;//客户端socket
	int timeout;//超时时间
	int released;//是否被释放
};
//待分配ID表与已占用ID表
struct ID_TABLE ID_table[MAX_ID_TABLE_SIZE + 5], used[MAX_ID_TABLE_SIZE + 5];
int ID_table_tail, used_table_tail;//待分配ID表与已占用ID表的队尾
//初始化待分配ID表
void Init_ID_table()
{
	int i = 1;
	for (; i <= MAX_ID_TABLE_SIZE; i++)
	{
		ID_table[i-1].nx = i;
		ID_table[i].released = 1;
	}
	ID_table[MAX_ID_TABLE_SIZE].nx = -1;
	ID_table_tail = MAX_ID_TABLE_SIZE;
}
//为客户端的原ID分配一个新ID
int get_new_ID(int old_ID, struct sockaddr_in client)
{
	//从队列首取出一个新ID
	int new_ID = ID_table[0].nx;
	ID_table[0].nx = ID_table[new_ID].nx;
	ID_table[new_ID].old_ID = old_ID;
	ID_table[new_ID].client = client;
	ID_table[new_ID].timeout = time(NULL) + TIMEOUT;
	ID_table[new_ID].released = 0;
	
	//将这个ID加入已占用ID表队尾
	used[used_table_tail].nx = new_ID;
	used_table_tail = new_ID;
	used[new_ID].nx = -1;
	return new_ID;
}
//释放ID
void release_ID(int new_ID)
{
	//将新ID释放
	ID_table[new_ID].released = 1;
	ID_table[ID_table_tail].nx = new_ID;
	ID_table_tail = new_ID;
	ID_table[new_ID].nx = -1;
	//在已占用ID表中删除
	while (used[0].nx != -1 
	&& ((ID_table[used[0].nx].timeout > 0 && ID_table[used[0].nx].released) 
	|| time(NULL) > ID_table[used[0].nx].timeout))
	{
		int tmp = used[0].nx;
		used[0].nx = used[tmp].nx;
		if(ID_table[tmp].released) continue;
		ID_table[tmp].released = 1;
		ID_table[ID_table_tail].nx = tmp;
		ID_table_tail = tmp;
		ID_table[tmp].nx = -1;
	}
}
#endif
