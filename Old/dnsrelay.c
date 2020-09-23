#include <stdio.h>
#include <signal.h>
#include <stdarg.h>
#include "hash.h"
#include "cache.h"
#include "process.h" 
#include "idtrans.h"
#define MAX_IP_LENGTH 20

char PATH[MAX_FILE_LENGTH];
char DNS_IP[MAX_IP_LENGTH];
const int p = 23333, mod = 4993; //哈希用的模数
int debuglevel = 0;
void Init(int argc, char* argv[]); //初始化函数
void DebugOutput(int d, const char *str, ...); //调试输出
void printErr(int sig); //错误输出
struct Cache c; //Cache 
SOCKET so; //接受本机传来的query用的Socket
int main(int argc, char *argv[])
{
	Init(argc, argv); 
	
	load_dns_table(); //载入DNS表
	Init_ID_table(); //初始化ID转换表
	hash_test(); //哈希
	so=init_socket(); //初始化Socket
	
	while(1)
	{
		Recv_from_local(); //从本地接收
		Recv_from_server(); //从DNS服务器接收
	}
 
//	clear();
	return 0;
}

void printErr(int sig){
    printf("Fuck, %d", sig);
}

void Init(int argc, char* argv[]){
    signal(SIGINT,printErr);
    if(argc == 1)return;
    int i;
    for(i = 1; i < argc; i++){
        if(strlen(argv[i]) < 2){
            raise(SIGINT);
        }
        if(argv[i][1] == 'd'){ //调试等级
            if(strlen(argv[i]) != 4 || !(argv[i][3] == '0' || argv[i][3] == '1' || argv[i][3] == '2')){
                raise(SIGINT);
            }
            else{
                debuglevel = argv[i][3] - '0';
            }
        }
        else if(argv[i][1] == 'i'){ //DNS服务器地址
            if(strlen(argv[i]) < 4 || strlen(argv[i]) > 18){
                raise(SIGINT);
            }
            else{
                int l = strlen(argv[i]);
                int j;
                char tem[20] = {0};
                for(j = 3; j < l; j++){
                    tem[j - 3] = argv[i][j];
                }
                l -= 3;
                int cnt = 0;
                int x = -1;
                for(j = 0; j < l; j++){
                    if(isdigit(tem[j])){
                        if(x == -1){
                            x = 0;
                            cnt++;
                        }
                        x *= 10;
                        x += tem[j] - '0';
                        if(x > 255){
                            raise(SIGINT);
                        }
                    }
                    else{
                        x = -1;
                    }
                }
                if(cnt != 4){
                    raise(SIGINT);
                }
                strcpy(DNS_IP, tem);
            }
        }
        else if(argv[i][1] == 'f'){ //DNS表
            if(strlen(argv[i]) < 4){
                raise(SIGINT);
            }
            int l = strlen(argv[i]);
            char tem[128] = {0};
            int j;
            for(j = 3; j < l; j++){
                tem[j - 3] = argv[i][j];
            }
            strcpy(PATH, tem);
        }
        else{
            raise(SIGINT);
        }
    }
}
void DebugOutput(int d, const char *str, ...){
​    va_list var;
	char c = 0;
	unsigned int ui = 0;
	int i = 0;
	float f = 0;
	double d = 0;
	char *s = NULL;
	
	va_start(var,str);
	
	while('\0' != *str)
	{
		if('%' != *str)
		{
			printf("%c",*str);
			str++;
			continue;
		}
		
		else
		{
			switch (*(++str))
			{
				case 'c':
					c = (char)va_arg(var,int);
					printf("%c",c);
					break;
					
				case 'u':
					ui = (unsigned int)va_arg(var,int);
					printf("%u",ui);
					break;
					
				case 'd':
					i = va_arg(var,int);
					printf("%i",i);
					break;
					
				case 'f':
					f = (float)va_arg(var,double);
					printf("%f",f);
					break;
					
				case 'l':
					if('f' == *(str + 1))
					{
						d = va_arg(var,double);
						printf("%lf",d);
						str++;
					}
					break;
					
				case 's':
					s = (char*)va_arg(var,char*);
					printf("%s",s);
					break;
					
				default:
					printf("%c",*str);
					break;
			}
			str++;
		}
		
	}
	
	va_end(var);
}
