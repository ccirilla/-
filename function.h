#include <errno.h>
#include <sys/epoll.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <setjmp.h>
#include <signal.h>
#include <sys/msg.h>
#include <strings.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <syslog.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <fcntl.h>
#define ARGS_CHECK(argc,val) {if(argc!=val) \
	{printf("error args\n");return -1;}}
#define ERROR_CHECK(ret,retval,funcName) {if(ret==retval) \
	{printf("%d:",__LINE__);fflush(stdout);perror(funcName);return -1;}}
#define THREAD_ERROR_CHECK(ret,funcName) {if(ret!=0) \
{printf("%s:%s\n",funcName,strerror(ret));return -1;}}

#define SWAP(x, y) x = (y + x) - (y = x)
//管理每一个子进程的数据结构
typedef struct{
	pid_t pid;//子进程的pid
	int fd;//子进程的管道对端
	short busy;//子进程是否忙碌，0代表非忙碌，1代表忙碌
}Process_Data;
typedef struct{
	int Len;
    int ctl_code;
	char buf[100];
}Train_t;
typedef struct node{
    Process_Data *p;
    struct node * pNext;
}node,*pNode;
typedef struct{
	int sum;
    int socket_fd[3];
	char name[3][50];
}desk_stat;
typedef struct{
    int len[3];
	char poker[3][20];
}PLY_pk;

#define FILENAME "file"
#define DEBUG
int makeChild(Process_Data*,int);
int childHandle(int);
int tcpInit(int*,char*,char*);
int sendFd(int,desk_stat*);
int recvFd(int,desk_stat*);
int tranFile(desk_stat*);
void enterQue(pNode*,pNode*,Process_Data*);
void quitQue(pNode*,pNode*,Process_Data*);
void printQue(pNode);
