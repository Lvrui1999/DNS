#ifndef IDTRANS_H
#define IDTRANS_H
#define TIMEOUT 10
#define MAX_ID_TABLE_SIZE 65540
#include <Winsock2.h>  
  
#pragma comment(lib,"Ws2_32.lib")

struct ID_TABLE
{
	int nx;
	int old_ID;
	struct sockaddr_in client;
	int timeout;
	int released;
};
struct ID_TABLE ID_table[MAX_ID_TABLE_SIZE + 5], used[MAX_ID_TABLE_SIZE + 5];
int ID_table_tail, used_table_tail;
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
int get_new_ID(int old_ID, struct sockaddr_in client)
{
	int new_ID = ID_table[0].nx;
	ID_table[0].nx = ID_table[new_ID].nx;
	ID_table[new_ID].old_ID = old_ID;
	ID_table[new_ID].client = client;
	ID_table[new_ID].timeout = time(NULL) + TIMEOUT;
	ID_table[new_ID].released = 0;
	
	used[used_table_tail].nx = new_ID;
	used_table_tail = new_ID;
	used[new_ID].nx = -1;
	return new_ID;
}
void release_ID(int new_ID)
{
	ID_table[new_ID].released = 1;
	ID_table[ID_table_tail].nx = new_ID;
	ID_table_tail = new_ID;
	ID_table[new_ID].nx = -1;
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
