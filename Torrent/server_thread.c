/***************************************** Server Thread **********************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>                                  
#include <sys/socket.h>                              
#include <netinet/in.h>                        
#include <netdb.h>                                
#include <sys/socket.h>
#include "thread_lib.h"

#define SEEDLOG "seedLog.txt"

/* Parsing Chunk Request from client, returning ChunkPos */
int Read_Chunk_Request(int sock)
{
   int code=0, err=0;
   char description[20], value[50];
   int chunkpos;
   
  //  printf("About to Read a Chunk Request \n");
   /* Parsing ChunkRequest from Client */
   while((code=Parse_Response_Line(sock,description,value))==0)
   {
       //printf("%s %s\n",description,value);
       if(!strcmpin(description,"MessageType:") && !strcmpin(value,"ChunkRequest"))
       continue;
       else if(!strcmpin(description,"FileName:"))
       {
          /* Locking the mutex because thread is about to have access to internal peer data structure used by all threads */
	  if( err = pthread_mutex_lock(&peer_mutex) )
	  {
	      close(sock);
	      fprintf(stderr,"p_thread_mutex_lock failed");
	      return -40;
	  }

          if(strcmpin(value,peer.filemdata.filename))
          {
             printf("Wrong Filename\n");
             return -1;
          }

          /* Unlocking the mutex after reading ChunkRequest and sending ChunkResponse */
	  if(err=pthread_mutex_unlock(&peer_mutex))
	  {
	     close(sock);
	     fprintf(stderr,"p_thread_mutex_unlock failed");
	     return -41;
	  }
       }
       else if(!strcmpin(description,"ChunkPos:"))
           chunkpos=atoi(value); 
       else
       {
           printf("Bad Chunk Request\n");
           return -2;
       }
   }

  // printf("Just Read a Chunk Request (%d)\n",chunkpos);
   
   if(code==1)
      return chunkpos;
   else 
      return code;
   
}

/* Sending Chunk Response to client. In case of success sending the whole chunk or chunknotfound in case of failure */
int Write_Chunk_Response(int sock, int chunkpos, FILE *fplog)
{
   FILE *fp;
   char* filepath, *chunkdata, chunkmd5[50];
   char reqbuf[2048]; 		/* Memory Buffer to hold ChunkResponse */
   char messagetype[20];
   int conlength, err=0;
   int cnf=0;  /* Chunk Not Found flag variable - Setting to 1 if chunk is not found */

   struct sockaddr_in client;
   socklen_t clientlen;
   struct sockaddr *clientptr=(struct sockaddr *)&client;

   /* Locking the mutex because thread is about to have access to internal peer data structure used by all threads */
   if( err = pthread_mutex_lock(&peer_mutex) )
   {
       close(sock);
       fprintf(stderr,"p_thread_mutex_lock failed");
       return -40;
   }

   /* First Checking Bitmap of the peer. The bit in chunkpos of the bitmap must be 1. That means that chunk is available */
   if(BMP_Is_Set(Bitmap,chunkpos)<=0)
   {
    /* Forming Chunk Response when Chunk is Found */
    sprintf(reqbuf,"MessageType: ChunkResponse\r\nResponseStatus: ChunkNotFound\r\nFileName: %s\r\nFile-md5sum: %s\r\nChunkPos: %d\r\n\r\n",peer.filemdata.filename,peer.filemdata.filemd5,chunkpos);
    /* Setting Chunk Not Found variable */
    cnf=1;
   }
   else{
           int conl;

	   if((filepath=malloc(strlen(peer.wd)+strlen(peer.filemdata.filename)+2))==NULL)
	   {
	      fprintf(stderr,"No Memory Available\n");
	      return -1;
	   }
	 
	   sprintf(filepath,"%s/%s",peer.wd,peer.filemdata.filename);
	 
	   if((fp=fopen(filepath,"rb"))==NULL)
	   {
	      free(filepath);
	      perror("fopen");
	      return -2;
	   }

           /* Getting file point in start of Chunk to be read */
	   fseek(fp,chunkpos*peer.ChunkSize,SEEK_SET);
           /* Computing content length */
           if((conl=peer.filemdata.filesize-ftell(fp))>=peer.ChunkSize)
              conl=peer.ChunkSize;
  
           /* Allocating memory for chunk */
	   if((chunkdata=malloc(conl))==NULL)
	   {
	      fprintf(stderr,"No Memory Available\n");
	      return -3;
	   }

           memset(chunkdata,0,conl);

	   /* Getting Chunk into a buffer */
	   if((conlength=fread(chunkdata,1,conl,fp))==-1)
	   {
	      free(filepath);
	      free(chunkdata);
	      perror("fread");
	      return -4;
	   }

	   /* Closing File after getting chunk */
	   if(fclose(fp)==EOF)
	   {
	      free(filepath);
	      free(chunkdata);
	      perror("fclose");
	      return -5;
	   }

	   free(filepath);

	   /* Computing MD5 Sum of chunk */
	   if(MD5Sum_Chunk(chunkdata,conlength,chunkmd5))
	   {
	      free(chunkdata);
	      printf("Error Computing Md5sum of chunk %d\n",chunkpos);
	      return -6;
	   }

   /* Forming Chunk Response when Chunk is Found */
      sprintf(reqbuf,"MessageType: ChunkResponse\r\nResponseStatus: ChunkFound\r\nFileName: %s\r\nFile-md5sum: %s\r\nFileSizeInBytes: %ld\r\nChunkSizeInBytes: %d\r\nChunkPos: %d\r\nChunk-md5sum: %s\r\nContentLength: %d\r\n\r\n",peer.filemdata.filename,peer.filemdata.filemd5,peer.filemdata.filesize,peer.ChunkSize,chunkpos,chunkmd5,conlength);

   }

   /* Unlocking the mutex after reading ChunkRequest and sending ChunkResponse */
   if(err=pthread_mutex_unlock(&peer_mutex))
   {
      close(sock);
      fprintf(stderr,"p_thread_mutex_unlock failed");
      return -41;
   }

   /* Sending ChunkResponse to Client Peer */
   if(write(sock,reqbuf,strlen(reqbuf))==-1)
   {
     free(chunkdata);
     perror("write");
     return -7;
   }

   /* Sending Chunk to Client Peer (only if chunk is found) */
   if(cnf==0)
   {
     if(write(sock,chunkdata,conlength)==-1)
     {
       free(chunkdata);
       perror("write");
       return -7;
     }
   }

   /* Getting client's address */
   if(getpeername(sock,clientptr,&clientlen))
   {
     if(cnf==0)
     free(chunkdata);

     perror("getpeername ");
     return -8;
   }
   
   /* Saving log record in seedLog file */
   fprintf(fplog,"Sent Chunk %d to %s\n",chunkpos,(char*)inet_ntoa(client.sin_addr));

   if(cnf==0)
   free(chunkdata);
 
   return 0;
   
  
}
/*  Detached Server Thread, accepting a ChunkRequest and sending a ChunkResponse */  
void *Server_thread(void *srvsock)
{
   int code=0,chunkpos;
   int err;
   FILE *fplog;    // Seedlog txt file pointer
   char logfile[25];

   /* Detaching thread */
   if(err=pthread_detach(pthread_self())) 
   { 
     printf("Error Detaching Server Thread\n");
     close(*(int*)srvsock);
     free((int*)srvsock);
     pthread_exit((void*)39);
   }

   /* Opening seedLog file to save info about sent chunks */
   sprintf(logfile,"%s/%s",peer.wd,SEEDLOG);
   if((fplog=fopen(logfile,"a+"))==NULL)
   {
     perror("open "); 
     close(*(int*)srvsock);
     free((int*)srvsock);
     pthread_exit((void*)100);
   }

   /* Reading ChunkRequest and getting Chunk Position requested */
   if((chunkpos=Read_Chunk_Request(*(int*)srvsock))<0)
   {
     printf("%d - Error Reading\n",*(int*)srvsock);
     close(*(int*)srvsock);
     free((int*)srvsock);
     pthread_exit((void*)41);
   }


   /* Writing ChunkResponse to Client */
   if(Write_Chunk_Response(*(int*)srvsock,chunkpos,fplog)<0)
   {
     printf("%d - Error Writing\n",*(int*)srvsock);
     close(*(int*)srvsock);
     free((int*)srvsock);
     pthread_exit((void*)42);
   }
  

   /* Closing seedLog file */
   if(fclose(fplog)==EOF)
   {
     perror("close ");
     pthread_exit((void*)101);
   }

  
   close(*(int*)srvsock);
   free((int*)srvsock);

   pthread_exit(NULL);

}
