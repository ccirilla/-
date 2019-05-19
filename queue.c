#include "function.h"

void enterQue(pNode* pphead,pNode *pptail,Process_Data *p)
{
    pNode pnew= (pNode)calloc(1,sizeof(node));
    pnew->p = p;
    if(*pptail == NULL)
    {
        *pphead = *pptail =pnew;
    }
    else
    {
        (*pptail)->pNext= pnew;
        *pptail = pnew;
    }
}
void quitQue(pNode* pphead,pNode *pptail,Process_Data *p)
{
    pNode q;
    q = *pphead;
    *p = *(q->p);
    if(*pphead == *pptail)
    {
        *pphead = *pptail = NULL;
    }
    else
    {
        *pphead = q->pNext;
    }
    free(q);
}
void printQue(pNode phead)
{
    while(NULL != phead)
    {
        printf("%d   %d   %d\n",phead->p->pid,phead->p->fd,phead->p->busy);
        phead = phead->pNext;
    }
}
