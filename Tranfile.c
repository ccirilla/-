#include "function.h"

const char m_iSort[15] = {12,13,1,2,3,4,5,6,7,8,9,10,11,14,15};
PLY_pk ply_pk;
Train_t train;

int recvCYL(int fd,void *pbuf,int len)
{
    char *buf = (char*)pbuf;
    int total = 0,ret;
    while(total<len)
    {
        ret = recv(fd,buf+total,len-total,0);
        if(ret == 0)
        {
            printf("plyer qiut\n");
            return -1;
        }
        total += ret;
    }
    return 0;
}

void Reshuffle(char*m_iPoker)
{
    srand(time(0));
    memset(m_iPoker, -1, 54);
    for (int iCard = 0; iCard < 54; iCard++)
    {
        int iPos = rand() % 54;
        while (m_iPoker[iPos] != -1)
        {
            iPos++;
            if (iPos >= 54) iPos = 0;
        }
        if(iCard==52)
            m_iPoker[iPos] = 13;
        else if(iCard==53)
            m_iPoker[iPos] = 14;
        else
            m_iPoker[iPos] = iCard%13;
    }
}

void Sort(char x[], int iMin, int iMax)
{
    for (int i = iMin; i <= iMax-1; i++)
    {
        for (int j = i+1; j <= iMax; j++)
        {
            if (m_iSort[(int)x[j]] < m_iSort[(int)x[i]]) SWAP(x[i], x[j]);
        }
    }
}

void send_poker(int fd,int i)
{
    train.Len = ply_pk.len[i];
    train.ctl_code = 1;
    memcpy(train.buf,ply_pk.poker[i],ply_pk.len[i]);
    int ret = send(fd,&train,8+train.Len,0);
    if(ret == -1)
    {
        exit(0);
    }
}

int judge_poker(int i,char *buf,int len)
{
    if(*buf == -1)
        return -2;
    char jug[21];
    int s,j,k;
    memcpy(jug,ply_pk.poker[i],ply_pk.len[i]);
    for(s=0;s<len;s++)
    {
        k=0;
        for(j=0;j<ply_pk.len[i]-s;j++)
        {
            if(buf[s]==jug[j])
                k=1;
            if(k==1)
                jug[j]=jug[j+1];
        }
        if(k==0)
            return -1;
    }
    if(ply_pk.len[i]==len)
    {
        ply_pk.len[i]=0;
        return 0;
    }
    memcpy(ply_pk.poker[i],jug,ply_pk.len[i]-len);
    ply_pk.len[i] -= len;
    return 1;
}
        
int tranFile(desk_stat *desk)
{
    char poker[54],buf[100];
    int i,host,ret,ctl_code,len;
    memset(&ply_pk,-1,sizeof(PLY_pk));
    Reshuffle(poker);

    for(i=0;i<3;i++)
    {
        train.Len = strlen(desk->name[(i+1)%3]);
        strcpy(train.buf,desk->name[(i+1)%3]);
        send(desk->socket_fd[i],&train,8+train.Len,0);
        train.Len = strlen(desk->name[(i+2)%3]);
        strcpy(train.buf,desk->name[(i+2)%3]);
        send(desk->socket_fd[i],&train,8+train.Len,0);
    }
    for (i = 0; i < 48; i++)
    {
        SWAP(poker[i+0], poker[i+3]);
        SWAP(poker[i+1], poker[i+4]);
        SWAP(poker[i+2], poker[i+5]);
    }
    for(i=0;i<3;i++)
    {
        memcpy(ply_pk.poker[i],poker+17*i,17);
        ply_pk.len[i]=17;
    }
    for(i=0;i<3;i++)
    {
        send_poker(desk->socket_fd[i],i);
    }
    Sort(poker,0,16);
    Sort(poker,17,33);
    Sort(poker,34,50);
    Sort(poker,51,53);
    for(i=0;i<3;i++)
    {
        memcpy(ply_pk.poker[i],poker+17*i,17);
        ply_pk.len[i]=17;
    }
    for(i=0;i<3;i++)
    {
        send_poker(desk->socket_fd[i],i);
    }
    //---------------------------------------------------
    //抢地主阶段，三个人都抢，最后一个人是地主，三个人都不抢，第一个人是地主
    srand(time(0));
    ret = rand()%3;
    host =ret;
    for(i=ret;i<3+ret;i++)
    {
        train.Len = strlen(desk->name[i%3]);
        train.ctl_code = 2;
        strcpy(train.buf,desk->name[i%3]);
        for(int j=0;j<3;j++)
        {
            send(desk->socket_fd[j],&train,8+train.Len,0);
        }
        recvCYL(desk->socket_fd[i%3],&train.Len,4);
        recvCYL(desk->socket_fd[i%3],&train.ctl_code,4);
        recvCYL(desk->socket_fd[i%3],train.buf,train.Len);
        if(*(train.buf) == '1')
        {
            host = i%3;
        }
        for(int j=0;j<3;j++)
        {
            if(j!=i%3)
                send(desk->socket_fd[j],&train,8+train.Len,0);
        }
    }
    train.Len = strlen(desk->name[host]);
    train.ctl_code = 3;
    strcpy(train.buf,desk->name[host]);
    for(int j=0;j<3;j++)
    {
        send(desk->socket_fd[j],&train,8+train.Len,0);
    }
    train.Len = 3;
    train.ctl_code = 3;
    memcpy(train.buf,poker+51,3);
    for(int j=0;j<3;j++)
    {
        send(desk->socket_fd[j],&train,8+train.Len,0);
    }
    memcpy(ply_pk.poker[host]+17,poker+51,3);
    ply_pk.len[host] = 20;
    Sort(ply_pk.poker[host],0,19);
    send_poker(desk->socket_fd[host],host);

    train.Len = strlen(desk->name[host]);
    train.ctl_code = 4;
    strcpy(train.buf,desk->name[host]);
    for(int j=0;j<3;j++)
    {
        send(desk->socket_fd[j],&train,8+train.Len,0);
    }
    //---------------------------------------------------------
    //出牌阶段 
    for(i=host;;i++)
    {
        memset(buf,-1,50);
        recvCYL(desk->socket_fd[i%3],&len,4);
        recvCYL(desk->socket_fd[i%3],&ctl_code,4);
        recvCYL(desk->socket_fd[i%3],buf,len);
        ret =judge_poker(i%3,buf,len);
        while(ret == -1)
        {
            train.Len = 0;
            train.ctl_code = 5;
            send(desk->socket_fd[i%3],&train,8,0);
            recvCYL(desk->socket_fd[i%3],&len,4);
            recvCYL(desk->socket_fd[i%3],&ctl_code,4);
            recvCYL(desk->socket_fd[i%3],buf,len);
            ret =judge_poker(i%3,buf,len);
        }
        if(ret ==1||ret==-2)
        {
            train.Len = strlen(desk->name[i%3]);
            train.ctl_code = 6;
            strcpy(train.buf,desk->name[i%3]);
            for(int j=0;j<3;j++)
                send(desk->socket_fd[j],&train,8+train.Len,0);
            if(ret == 1)
                train.Len = len;
            else if(ret == -2)
                train.Len = 0;
            train.ctl_code = 6;
            memcpy(train.buf,buf,len);
            for(int j=0;j<3;j++)
                send(desk->socket_fd[j],&train,8+train.Len,0);
            train.Len = strlen(desk->name[(i+1)%3]);
            train.ctl_code = 6;
            strcpy(train.buf,desk->name[(i+1)%3]);
            for(int j=0;j<3;j++)
                send(desk->socket_fd[j],&train,8+train.Len,0);
            send_poker(desk->socket_fd[i%3],i%3);
        }
        if(ret == 0)
        {
            train.Len = strlen(desk->name[i%3]);
            train.ctl_code = 6;
            strcpy(train.buf,desk->name[i%3]);
            for(int j=0;j<3;j++)
                send(desk->socket_fd[j],&train,8+train.Len,0);
            if(ret == 1)
                train.Len = len;
            else if(ret == -2)
                train.Len = 0;
            train.ctl_code = 6;
            memcpy(train.buf,buf,len);
            for(int j=0;j<3;j++)
                send(desk->socket_fd[j],&train,8+train.Len,0);
            train.Len = strlen(desk->name[(i+1)%3]);
            train.ctl_code = 6;
            strcpy(train.buf,desk->name[(i+1)%3]);
            for(int j=0;j<3;j++)
                send(desk->socket_fd[j],&train,8+train.Len,0);
            send_poker(desk->socket_fd[i%3],i%3);
            train.Len = strlen(desk->name[i%3]);
            train.ctl_code = 7;
            strcpy(train.buf,desk->name[i%3]);
            send(desk->socket_fd[i%3],&train,8+train.Len,0);
            sleep(3);
            return 0;
        }
    }
    return 0;
}

