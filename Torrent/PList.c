/************************************* Peer List (Linked List) Data structure Functions **********************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "thread_lib.h"

/* Initializing peerlist */
void PL_Init(PeerList **peerlist)
{
  *peerlist=NULL;
}

/* Inserting a Couple of Ip-port record in Peerlist data structure of the peer in the end of the linked list */
int PL_Insert(PeerList **peerlist, char *ip, unsigned int port)
{
  PeerList *prevnode=*peerlist, *node;
 
  if((node=malloc(sizeof(PeerList)))==NULL)
  { 
      fprintf(stderr,"No memory available\n");
      return -1;
  }
  if((node->ip=malloc(strlen(ip)+1))==NULL)
  { 
      fprintf(stderr,"No memory available\n");
      return -1;
  }

  strcpy(node->ip,ip);
  node->port=port;

  if(*peerlist==NULL)
  {
     *peerlist=node;
     node->next=NULL;
     return 0;
  }
   
  while(prevnode->next!=NULL)
    prevnode=prevnode->next;

  prevnode->next=node;
  node->next=NULL;

  return 0;
} 

/* Getting Next Node after peernode in peerlist */
PeerList* PL_Get_Next(PeerList *peernode)
{
   /* Checking for empty list */
   if(peernode==NULL)
     return NULL;
   /* Returning next node */
   else
   return peernode->next;
  
}

/* Destroying the Peerlist */
void PL_Destroy(PeerList **peerlist)
{
  PeerList *prevnode, *node;

  node=*peerlist;

  while(node!=NULL)
  {
     prevnode=node;
     node=node->next;

     
     free(prevnode->ip);
     free(prevnode);
  }
  
  *peerlist=NULL;

}

/* Printing Peerlist data */
void PL_Print(PeerList *peerlist)
{
  PeerList *node;

  node=peerlist;

  while(node!=NULL)
  {
     printf("%s %u\n",node->ip,node->port);
     node=node->next;
  } 

}



