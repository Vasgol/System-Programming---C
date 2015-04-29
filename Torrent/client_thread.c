/************************************************* Client thread **********************************************************************/
#include <stdio.h>
#include <sys/types.h>	     /* sockets */
#include <sys/socket.h>	     /* sockets */
#include <netinet/in.h>	     /* internet sockets */
#include <unistd.h>          /* read, write, close */
#include <netdb.h>	         /* gethostbyaddr */
#include <stdlib.h>	         /* exit */
#include <string.h>	         /* strlen */
#include <math.h>                /* ceil */
#include "thread_lib.h"
#include "MD5_Lib.h"
#include "BMP_Lib.h"

#define LEECHLOG "leechLog.txt"

/* Client thread Writing Chunk Request returning 0 upon success */
int Write_Chunk_Request(int sock, int chunkpos)
{
  char reqbuf[256];
  int err;
  
  /* Locking the mutex because thread is about to have access to the peer data structure in order to write a Chunk Request */
  if( err = pthread_mutex_lock(&peer_mutex) )
  {  
     fprintf(stderr,"p_thread_mutex_lock failed");
     return -40;
  }

  /* Storing ChunkRequest in a buffer */
  sprintf(reqbuf,"MessageType: ChunkRequest\r\nFileName: %s\r\nChunkPos: %d\r\n\r\n",peer.filemdata.filename,chunkpos);

  /* Unlocking the mutex after writing a Chunk Request */
  if(err=pthread_mutex_unlock(&peer_mutex))
  {
     fprintf(stderr,"p_thread_mutex_unlock failed");
     return -41;
  }

  /* Writing ChunkRequest to socket */
  if(write(sock,reqbuf,strlen(reqbuf))==-1)
  {
     perror("write");
     return -1;
  }
  
  /* Returning 0 upon success */
  return 0;
}

/* Client thread Reading Chunk Response. Returns 0 upon success (Chunk Found by server peer) or -1 upon failure (Chunk Not Found).
   In case of any other failures, it returns a negative value. */
int Read_Chunk_Response(int sock, char* description, char* value, int chunkpos, FILE *fplog, PeerList *plistrec)
{
   FILE* fp;
   char* filepath;
   int code=0, err=0;
   int conlength;
   char *chunkvalidmd5, *cmd5;
   char* chunkdata;

   /* Parsing ChunkResponse from Server Peer till end of Heading (emptyline with \r\n) */
   while((code=Parse_Response_Line(sock,description,value))==0)
   {
       //printf("%s %s\n",description,value);

       if(!strcmpin(description,"MessageType:") && !strcmpin(value,"ChunkResponse"))
       continue;
       else if(!strcmpin(description,"ResponseStatus:"))
       {
           if(!strcmpin(value,"ChunkNotFound"))
           {
              while((code=Parse_Response_Line(sock,description,value))==0);
              return 1;
           }
           else if(!strcmpin(value,"ChunkFound"))
               continue;
           else
           {   
               printf("Bad Response\n");
               return -1; 
           }
       }
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
       else if(!strcmpin(description,"File-md5sum:"))
       {
          /* Locking the mutex because thread is about to have access to internal peer data structure used by all threads */
	  if( err = pthread_mutex_lock(&peer_mutex) )
	  {
	      close(sock);
	      fprintf(stderr,"p_thread_mutex_lock failed");
	      return -40;
	  }

          if(strcmpin(value,peer.filemdata.filemd5))
          {
             printf("Wrong File MD5 Sum\n");
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
       else if(!strcmpin(description,"FileSizeInBytes:"))
       {
          /* Locking the mutex because thread is about to have access to internal peer data structure used by all threads */
	  if( err = pthread_mutex_lock(&peer_mutex) )
	  {
	      close(sock);
	      fprintf(stderr,"p_thread_mutex_lock failed");
	      return -40;
	  }

          if(atol(value)!=peer.filemdata.filesize)
          {
             printf("Wrong File Size\n");
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
       else if(!strcmpin(description,"ChunkSizeInBytes:"))
       {
          /* Locking the mutex because thread is about to have access to internal peer data structure used by all threads */
	  if( err = pthread_mutex_lock(&peer_mutex) )
	  {
	      close(sock);
	      fprintf(stderr,"p_thread_mutex_lock failed");/* Locking the mutex because thread is about to have access to the peerlist data structure accessed by heartbit-peerlist thread and client thread */

	      return -40;
	  }

          if(atoi(value)!=peer.ChunkSize)
          {
             printf("Wrong Chunk Size\n");
             return -1;
          }

          /* Unlocking the mutex */
	  if(err=pthread_mutex_unlock(&peer_mutex))
	  {
	     close(sock);
	     fprintf(stderr,"p_thread_mutex_unlock failed");
	     return -41;
	  }
       }
       else if(!strcmpin(description,"ChunkPos:"))
       {
          if(atoi(value)!=chunkpos)
          {
             printf("Wrong Filename\n");
             return -1;
          }
       }
       else if(!strcmpin(description,"Chunk-md5sum:"))
       {
          if((chunkvalidmd5=malloc(strlen(value)))==NULL)
          {
             fprintf(stderr,"No memory available\n");
             return -10;
          }

          memcpy((void*)chunkvalidmd5,(void*)value,strlen(value)+1);
       }
       else if(!strcmpin(description,"ContentLength:"))
           conlength=atoi(value);
       else
       {   
           printf("Bad Response (%s %s)\n",description,value);
           return -1; 
       }
           
   }


   /* Returning error upon negative code from parsing */
   if(code<0)
   return -20;

   /* Locking the mutex because thread is about to have access to internal peer data structure used by all threads */
   if( err = pthread_mutex_lock(&peer_mutex) )
   {
       close(sock);
       fprintf(stderr,"p_thread_mutex_lock failed");
       return -40;
   }

   if((filepath=malloc(strlen(peer.wd)+strlen(peer.filemdata.filename)+2))==NULL)
   {
       fprintf(stderr,"No memory available\n");
       return -10;
   }
      
   sprintf(filepath,"%s/%s",peer.wd,peer.filemdata.filename);

   /* Unlocking the mutex */
   if(err=pthread_mutex_unlock(&peer_mutex))
   {
      close(sock);
      fprintf(stderr,"p_thread_mutex_unlock failed");
      return -41;
   }
   
   /* Opening file to write chunk (appending) */
   if((fp=fopen(filepath,"rb+"))==NULL)
   {
       free(filepath);
       perror("open");
       return -11;
   }

   free(filepath);
   
   /* Allocating memory for the exact size of chunk */
   if((chunkdata=malloc(conlength))==NULL)
   {
       fprintf(stderr,"No memory available\n");
       return -10;
   }

   if((cmd5=malloc(strlen(chunkvalidmd5)))==NULL)
   {
       fprintf(stderr,"No memory available\n");
       return -10;
   }

   memset(chunkdata,0,conlength);
   memset(cmd5,0,strlen(chunkvalidmd5));

   /* Getting file point in start of Chunk to be written */
   fseek(fp,chunkpos*peer.ChunkSize,SEEK_SET);

   /* Reading Chunk from Server Peer Chunk Response */
   if(read(sock,chunkdata,conlength)==-1)
   { 
       perror("read");
       return -11;
   }

   /* Computing MD5 Sum of chunk */
   if(MD5Sum_Chunk(chunkdata,conlength,cmd5))
   {
     
      free(chunkdata);
      
      return -6;

   } 

   /* Checking Md5 Sum of Chunk */
   if(strcmp(cmd5,chunkvalidmd5))
   {
     
      /* Updating leechlog file */
      fprintf(fplog,"Received Chunk %d with md5-sum %s from %s:%u : CORRUPTED\n",chunkpos,cmd5,plistrec->ip,plistrec->port);
      return -7;
   }

   /* Writing Chunk in file */
   if((fwrite(chunkdata,1,conlength,fp))==-1)
   {
      free(filepath);
      free(chunkdata);
      perror("fread");
      return -4;
   }

   if(fclose(fp)==EOF)
   {
       perror("close");
       return -11;
   }

   /* Updating bitmap */
   if(BMP_Set(Bitmap,chunkpos))
   {
     printf("Error updating bitmap bit\n");
     return -12;
   }

   /* Updating leechlog file */
   fprintf(fplog,"Received Chunk %d with md5-sum %s from %s:%u : OK\n",chunkpos,cmd5,plistrec->ip,plistrec->port);
 
   free(chunkvalidmd5);
   free(chunkdata);
   free(cmd5);
   return 0;
  
}
/* Clienth Thread main function */
void *Client_thread()
{
   PeerList* plistrec=NULL;
   char description[20], value[50];
   int code=0,chunkpos;
   int err;
   int csock;
   char logfile[25];
   int length=ceil((float)peer.filemdata.filesize/(float)peer.ChunkSize); // Defining number of chunks file contains 
   int chunks=0;  // Counter of chunks successfully obtained by server;
   int percentage; // Percentage of chunks downloaded
   FILE *fplog;    // Leechlog txt file pointer

   struct sockaddr_in server;
   struct sockaddr *serverptr = (struct sockaddr*)&server;
   struct hostent *rem;

   sprintf(logfile,"%s/%s",peer.wd,LEECHLOG);
   if((fplog=fopen(logfile,"a"))==NULL)
   {
      perror("open ");
      pthread_exit((void*)100);
   }
   
   /* Client thread loop forever till the whole file requested by leecher has been downloaded */
   while(1)
   {  
       int connected=0, cnf=0; 
       
       
       /* Locking the mutex because thread is about to have access to the Bitmap data structure which is shared by client-server threads */
       if( err = pthread_mutex_lock(&peer_mutex) )
       {
	 close(csock);
	 fprintf(stderr,"p_thread_mutex_lock failed");
	 pthread_exit((void*)40);
       } 
       
       /* Finding a zero bit to request the chunkpos chunk of the file. If -1 is returned, leecher downloaded the whole file (returning 1 in this case) */
       /* Also checking termination variable. */  
       if((chunkpos=BMP_FindEmpty(Bitmap,length))==-1 || term_var!=0)
       {
         char fmd5[30], *filepath;
              
         /* File's path */
         if((filepath=malloc(strlen(peer.wd)+strlen(peer.filemdata.filename)+2))==NULL)
	 {
            close(csock);
            fprintf(stderr,"No Memory Available\n");
            pthread_exit((void*)43);
         }
	 
	 sprintf(filepath,"%s/%s",peer.wd,peer.filemdata.filename);

         /* Getting file's md5 sum */
         if(MD5Sum_File(filepath,fmd5))
         {
            close(csock);
            fprintf(stderr,"Error Computing file MD5 Sum\n");
            pthread_exit((void*)43);
         }
             
         //printf("FILE MD5: %s (%s)\n",peer.filemdata.filemd5,fmd5); 

         /* Checking validity of file's MD5 Sum */
         if(strcmp(fmd5,peer.filemdata.filemd5))
         { 
           printf("%s MD5 Sum does not aggree with File's MD5 Sum of the Swarm\n",peer.filemdata.filename);
           /* Removing file */
           if(remove(filepath))
           perror("remove");

         }

         free(filepath);
        // printf("Whole file has been downloaded\n");

         /* Client thread terminates after downloading the whole file of the swarm (leecher becomes seeder) */
         break;
       }

       /* Getting first peer of peer's list in order to connect */
       plistrec=peer.peerlist;
        

       /* Unlocking the mutex after finding empty bit in Bitmap data structure */
       if(err=pthread_mutex_unlock(&peer_mutex))
       {
         close(csock);
         fprintf(stderr,"p_thread_mutex_unlock failed");
         pthread_exit((void*)42);
       }

       

       /* Connecting to a server peer (ip-port couple obtained by peer's list from tracker. If server peer connected responses ChunkNotFound, getting next peer of the list till finding the chunk.
          If all list is scanned and chunk was not found getting another chunk and trying again. */
       while(connected==0)
       {
               /* Creating socket for connection */
	       if ((csock = socket(PF_INET, SOCK_STREAM, 0)) < 0)
	       {
	    	  perror("socket");
		  pthread_exit((void*)39);
	       }

	       /* Locking the mutex because thread is about to have access to the peerlist data structure accessed by heartbit-peerlist thread and client thread */
	       if( err = pthread_mutex_lock(&peer_mutex) )
	       {
		 close(csock);
		 fprintf(stderr,"p_thread_mutex_lock failed");
		 pthread_exit((void*)40);
	       }

	       /* Connecting to a server peer (Ip-port couple found on peer's list send by tracker) */
	       if(plistrec!=NULL)
	       {
		 server.sin_family = PF_INET;       			      /* Internet domain */
		 server.sin_addr.s_addr = inet_addr(plistrec->ip);            /* IP address */  
		 server.sin_port = htons(plistrec->port);                     /* Peer's Server port */

		 /* Initiate connection with the peer's server. In case of failure, getting next peer of the list to connect */
		 if (connect(csock, serverptr, sizeof(server)) < 0)
                 {
            
                    /* Getting next couple of ip-port from peer's list */
                    plistrec=PL_Get_Next(plistrec);
              
                    /* Unlocking the mutex */
		    if(err=pthread_mutex_unlock(&peer_mutex))
		    {
		       
		       fprintf(stderr,"p_thread_mutex_unlock failed");
		       pthread_exit((void*)42);
		     }
                
                    close(csock);
		    continue;
                 }
		
                // PL_Print(peer.peerlist);
		// printf("Connected to %s:%u\n",plistrec->ip, plistrec->port);
		   
                 /* Managed to connect to a server peer */
		 connected=1;
	       } 
	       /* Starting from head of list when finished scanning the whole peer list */
	       else
               {
		 plistrec=peer.peerlist;

                 /* Unlocking the mutex */
	         if(err=pthread_mutex_unlock(&peer_mutex))
	         {
		   close(csock);
		   fprintf(stderr,"p_thread_mutex_unlock failed");
		   pthread_exit((void*)42);
	         }

                 close(csock);
                 continue;
               }

               /* Unlocking the mutex after reading a couple of ip-port from Peerlist */
	       if(err=pthread_mutex_unlock(&peer_mutex))
	       {
		 close(csock);
		 fprintf(stderr,"p_thread_mutex_unlock failed");
		 pthread_exit((void*)42);
	       }
               

	       /* Writing a ChunkRequest requesting a random Chunk */
	       if(Write_Chunk_Request(csock,chunkpos))
	       {
		  printf("Writing Chunk Request failed\n");
		  pthread_exit((void*)43);
	       }    
      
	       /* Reading Chunk response from Server in case of chunknotfound or failure reading chunk response, getting next in peer list */
	       if((err=Read_Chunk_Response(csock,description,value,chunkpos,fplog,plistrec)))
	       {
		 close(csock); // Closing client socket in order to get the next server peer of the list
		 connected=0; // Disconnecting
                 cnf=1;        // Marking Chunk not found flag variable true upon unsuccessful Chunk Response
		 //printf("Reading Chunk Response failed (%d)\n",err);
	       }
               /* If chunk found successfully computing percentage of chunks downloaded to print it to stdout */
               else
               {
                 int pper=percentage;

                 chunks++;
                 percentage=(100*chunks)/length;         
                
                 if(percentage%10==0 && percentage!=0 && pper!=percentage)
                    printf("Leeching %d%% complete\n",percentage);
               }
          
 
               /* If chunk requested has not been found, getting next peer of the list */
               if(cnf==1)
               {
                 /* Locking the mutex because thread is about to have access to the peerlist data structure accessed by heartbit-peerlist thread and client thread */
	         if( err = pthread_mutex_lock(&peer_mutex) )
	         {
		   close(csock);
		   fprintf(stderr,"p_thread_mutex_lock failed");
		   pthread_exit((void*)40);
	         }

                 /* Getting next peer */         
                 plistrec=PL_Get_Next(plistrec);         

                 /* Unlocking the mutex */
	         if(err=pthread_mutex_unlock(&peer_mutex))
	         {
		   close(csock);
		   fprintf(stderr,"p_thread_mutex_unlock failed");
		   pthread_exit((void*)42);
	         }

                 /* Finished scanning the whole peer list breaking from connection loop in order to request for another chunk */ 
                 if(plistrec==NULL)
                 {
                   //printf("Finished Scanning Peer List, requesting new chunk...\n");
                   break;
                 }
                  
                 cnf=0;  
               }
             
       }

       /* Closing client sock after successfull ChunkResponse (Chunk Found) */
       close(csock);


    } // end of while(1)

    /* Unlocking the mutex before terminating client thread */
    if(err=pthread_mutex_unlock(&peer_mutex))
    {
         close(csock);
         fprintf(stderr,"p_thread_mutex_unlock failed");
         pthread_exit((void*)42);
    }
  
    /* Closing connection with server */
    close(csock);
  
    if(fclose(fplog)==EOF)
    {
       perror("close ");
       pthread_exit((void*)101);
    }

    /* Terminating client thread */
    pthread_exit(NULL);
   
}
