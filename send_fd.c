#include "function.h"

int sendFd(int fd,desk_stat *desk)
{
    struct msghdr msg;
    struct iovec iov;
    struct cmsghdr *cmsg;
    int len,ret;
    iov.iov_base = desk;
    iov.iov_len = sizeof(desk_stat);
    len = CMSG_LEN(sizeof(int)*3);
    cmsg = ( struct cmsghdr * )calloc(1,len);
    cmsg->cmsg_len =len;
    cmsg->cmsg_level = SOL_SOCKET;
    cmsg->cmsg_type = SCM_RIGHTS;
    memcpy(CMSG_DATA(cmsg),desk->socket_fd,12);
    bzero(&msg,sizeof(struct msghdr));
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    msg.msg_control = cmsg;
    msg.msg_controllen = len;
    ret = sendmsg(fd,&msg,0);
    ERROR_CHECK(ret,-1,"sendmsg");
    free(cmsg);
    return 0;
}

int recvFd(int fd,desk_stat *desk)
{
    struct msghdr msg;
    struct cmsghdr *cmsg;
    struct iovec iov;
    int len,ret;
    iov.iov_base = desk;
    iov.iov_len = sizeof(desk_stat);
    len = CMSG_LEN(sizeof(int)*3);
    cmsg = ( struct cmsghdr * )calloc(1,len);
    cmsg->cmsg_len =len;
    cmsg->cmsg_level = SOL_SOCKET;
    cmsg->cmsg_type = SCM_RIGHTS;
    bzero(&msg,sizeof(struct msghdr));
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    msg.msg_control = cmsg;
    msg.msg_controllen = len;
    ret = recvmsg(fd,&msg,0);
    ERROR_CHECK(ret,-1,"sendmsg");
    memcpy(desk->socket_fd,CMSG_DATA(cmsg),12);
    free(cmsg);
    return 0;
}
