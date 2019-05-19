#include "function.h"

//三人就直接开，换桌，准备等还要写个线程没空写
//没有牌型比大小规则，只要是有的牌，可以一次性出空
//没有超时重连，托管等，不会写
//还有N多BUG，不会写
//
//客户端输入：网址，端口，用户名
//服务器端输入：网址，端口，最大游戏桌数（一个线程一桌游戏）

int exit_fds[2];

void sighHandle(int sigh)
{
    write(exit_fds[1],&sigh,1);
}

int main(int argc, char *argv[])
{
    signal(SIGUSR1,sighHandle);
    ARGS_CHECK(argc,4);
    pipe(exit_fds);
    int child_num = atoi(argv[3]);
    Process_Data *pChild = (Process_Data*)calloc(child_num,sizeof(Process_Data));
    makeChild(pChild,child_num);
    int i ,j;
    int socket_fd,new_fd,epfd,ret,ready_num;
    pNode phead = 0,ptail =0;
    Process_Data work;
    struct epoll_event event,evs[child_num + 2];
    ret = tcpInit(&socket_fd,argv[1],argv[2]);
    ERROR_CHECK(ret,-1,"tcpInit");
    epfd = epoll_create(1);
    event.events = EPOLLIN;
    event.data.fd = socket_fd;
    epoll_ctl(epfd,EPOLL_CTL_ADD,socket_fd,&event);
    event.data.fd = exit_fds[0];
    epoll_ctl(epfd,EPOLL_CTL_ADD,exit_fds[0],&event);
    for(j=0;j<child_num;j++)
    {
        event.data.fd = pChild[j].fd;
        epoll_ctl(epfd,EPOLL_CTL_ADD, pChild[j].fd,&event);
        enterQue(&phead,&ptail,pChild+j);
    }
    desk_stat desk;
    bzero(&desk,sizeof(desk));
#ifdef DEBUG
    printf("pid   fd   busy\n");
    for(i =0;i<child_num;i++)
    {
        printf("%d   %d   %d\n",pChild[i].pid,pChild[i].fd,pChild[i].busy);
    }
    printf("create sucessful\n");
    printQue(phead);
#endif
    while(1)
    {
        ready_num = epoll_wait(epfd,evs,child_num+2,-1);
        for(i=0;i<ready_num;i++)
        {
            if(evs[i].data.fd == socket_fd)
            {
                new_fd = accept(socket_fd,NULL,NULL);
                for(j=0;desk.socket_fd[j];j++);
                recv(new_fd,desk.name[j],sizeof(desk.name[j]),0);
                printf("name: %s had connect and waiting\n",desk.name[j]);
                desk.socket_fd[j] = new_fd;
                desk.sum++;
                if(desk.sum == 3)
                {
                    if(NULL != phead)//尚未考虑线程全部忙碌
                    {
                        quitQue(&phead,&ptail,&work);
                        sendFd(work.fd,&desk);
                        work.busy = 1 ;
                        printf("desk: %d game is begin!\n",work.pid);
                        printf("player :    %s  and  %s  and  %s \n",
                               desk.name[0],desk.name[1],desk.name[2]);
                    }
                    for(j=0;j<3;j++)
                    {
                        close(desk.socket_fd[j]);
                    }
                    bzero(&desk,sizeof(desk));
                }

            }
            else if(evs[i].data.fd == exit_fds[0])
            {
                close(socket_fd);
                printf("strat quit\n");
                for(j=0;j<child_num;j++)
                {
                    kill(pChild[j].pid,SIGQUIT);
                }
                for(j=0;j<child_num;j++)
                {
                    wait(NULL);
                }
                free(pChild);
                printf("quit finish\n");
                return 0;
            }
            else
            {
                for(j=0;j<child_num;j++)
                {
                    if(evs[i].data.fd == pChild[j].fd)
                    {
                        read(pChild[j].fd,&ret,1);
                        enterQue(&phead,&ptail,pChild+j);
                        printf("child %d is not busy!\n",pChild[j].pid);
                    }
                }
            }
        }
    }
    return 0;
}

