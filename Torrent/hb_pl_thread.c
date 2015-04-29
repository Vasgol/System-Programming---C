/***************************************** Heart Beat - Peer List Thread **********************************************************/
#include <stdio.h>
#include "thread_lib.h"

/************* HEART BEAT / PEER LIST THREAD responsible for receiving these messages from tracker. *************************/
/* 1. Heart Beat Message: Just Reading it so that tracker knows that peer is alive in the swarm                             */
/* 2. Peer List Message:  Reading Peer List message and updating peerlist internal data structure of the peer               */
/*                        to be used by client thread to search for chunks from peer's from this list. Using mutex          */
/*                        in order to ensure sychronization in the access of the list by two these threads                  */
/****************************************************************************************************************************/                           
void *HB_PL_thread(void *tsock)
{
   char buf1[20], buf2[20];
   int code=0;
   int err;
  

   /* Reading Heart Beat - Peer List messages from tracker. In case of Peer List message, updating the list of peers data structure */
   while((code=Parse_Response_Line(*(int*)tsock,buf1,buf2))>=0)
   {
 
      /* Checking Termination variable in case of signal (SIGINT / SIGTERM) */
      if(term_var)
        break;

      /* Reading Couple of Description - Value from tracker */
      if(code==0)
      {
        /* Checking MessageType */
        if(!strcmpin(buf1,"MessageType:"))
        {
           /* In case of reading a HeartBeat message just continue reading from tracker */
           if(!strcmpin(buf2,"HeartBeat"))
           continue;
           /* In case of reading a PeerList message, Peerlist is updated */
           else if(!strcmpin(buf2,"PeerList"))
           {
             /* Just Reading \r\n after Peerlist heading */ 
             if(Parse_Response_Line(*(int*)tsock,buf1,buf2)==1)
             {
               /* Locking the mutex because thread is about to have access to internal peer data structure used by all threads */
               if( err = pthread_mutex_lock(&peer_mutex) )
               {
                  close(*(int*)tsock);
                  fprintf(stderr,"p_thread_mutex_lock failed");
                  pthread_exit((void*)40);
               }
               
               /* Destroying Previous Peerlist */
               PL_Destroy(&peer.peerlist);
              
               //printf("------- Peer List ----------\n");
               /* Reading Couple of Ip - Port till line with only \r\n */
               while((code=Parse_Response_Line(*(int*)tsock,buf1,buf2))==0)
               {
                  unsigned int port=(unsigned int)atoi(buf2);

                  /* Updating the list with new data */
                  if(PL_Insert(&peer.peerlist,buf1,port)<0)
                  {
                     close(*(int*)tsock);
                     pthread_exit((void*)50);
                  }
               }
               /* In case of error Code from parsing thread will terminate (Tracker will no longer acknowledge the peer as member of the swarm) */
               if(code<0)
               break;

               //PL_Print(peer.peerlist); 
               //printf("-----------------------------\n"); 

               /* Unlocking the mutex after updating PeerList */
               if( err = pthread_mutex_unlock(&peer_mutex) )
               {
                 close(*(int*)tsock);
                 fprintf(stderr,"p_thread_mutex_unlock failed");
                 pthread_exit((void*)41);
               }
             }
 
             /* In case of error Code from parsing thread will terminate (Tracker will no longer acknowledge the peer as member of the swarm) */
             if(code<0)
             break;
                 
           }             
           
        }
      
      }
       
   }

   close(*(int*)tsock);
   pthread_exit(NULL);
} 
