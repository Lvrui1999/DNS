#include <stdio.h>
#include <signal.h>
#include "hash.h"
#include "cache.h"
#include "process.h"
#define MAX_IP_LENGTH 20

char PATH[MAX_FILE_LENGTH];
char DNS_IP[MAX_IP_LENGTH];
const int p = 23333, mod = 4993;
int debuglevel = 0;
void Init(int argc, char* argv[]);
void DebugOutput(int d, const char *str);
void printErr(int sig);
struct Cache c;
int main(int argc, char *argv[])
{
	Init(argc, argv);
	

	load_dns_table();
	init_socket();
	
	while(1)
	{
		Recv();
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
        if(argv[i][1] == 'd'){
            if(strlen(argv[i]) != 4 || !(argv[i][3] == '0' || argv[i][3] == '1' || argv[i][3] == '2')){
                raise(SIGINT);
            }
            else{
                debuglevel = argv[i][3] - '0';
            }
        }
        else if(argv[i][1] == 'i'){ 
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
        else if(argv[i][1] == 'f'){
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
void DebugOutput(int d, const char *str){
    if(debuglevel == d){
        printf(str);
    }
}
