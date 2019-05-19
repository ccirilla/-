#include "function.h"

int makeChild(Process_Data *pchild,int chilid_num)
{
    int fds[2];
    int pid,i;
    for(i=0;i<chilid_num;i++)
    {
        socketpair(AF_LOCAL,SOCK_STREAM,0,fds);
        pid = fork();
        if( 0 == pid )
        {
            close(fds[1]);
            childHandle(fds[0]);
        }
        else
        {
            close(fds[0]);
            pchild[i].pid = pid;
            pchild[i].fd = fds[1];
            pchild[i].busy = 0;
        }
    }
    return 0;
}

int childHandle(int fd)
{
    desk_stat desk;
    bzero(&desk,sizeof(desk));
    sigset_t sigset;
    sigemptyset(&sigset);
    sigaddset(&sigset,SIGQUIT);
    while(1)
    {
        recvFd(fd,&desk);
        sigprocmask(SIG_BLOCK,&sigset,NULL);
        tranFile(&desk);
        for(int i=0;i<3;i++)
            close(desk.socket_fd[i]);
        write(fd,&desk,1);
        sigprocmask(SIG_UNBLOCK,&sigset,NULL);
    }
    return 0;
}
