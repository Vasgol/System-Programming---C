/********************************************* Main function of the peer (Main Thread) ***********************************************************************************************/
/* 1. Reading Configuration File of the peer to define peer's type (Seeder or Leecher) and saving peer's data in internal data structures of the peer				     */
/* 2. Connecting to Tracker in order to create new swarm as a seeder or to join swarm as a leecher										     */
/* 3. Managing Multi-Threaded Server - Creating detached threads to read ChunkRequests and write ChunkResponses 			                                             */
/*																						     */
/*************************************************************************************************************************************************************************************/
#include <sys/types.h>                                   /* For sockets */
#include <sys/socket.h>                                  /* For sockets */
#include <netinet/in.h>                         /* For Internet sockets */
#include <netdb.h>                                 /* For gethostbyname */
#include <stdio.h>                                           /* For I/O */
#include <stdlib.h>                                         /* For exit */
#include <string.h>                         /* For strlen, bzero, bcopy */
#include <limits.h>                         /* For LINE_MAX */
#include <pthread.h>
#include <math.h>                           /* For ceil */
#include <signal.h>                         /* For signal handler */
#include <unistd.h>                         /* For fcntl */
#include <fcntl.h>


#include "main_thread_lib.h"                /* Main Thread functions library */
#include "hb_pl_thread_lib.h"		    /* Heart Beat/ Peer List Thread Library */
#include "server_thread_lib.h"              /* Server Thread functions library */
#include "client_thread_lib.h"              /* Client Thread functions library */

int lsock;   /* Socket to listen connections for the multithreaded server */

/* Main Thread Function */
main(int argc, char *argv[])
{
   int i=0;
   FILE *fp;    /* Configuration File Pointer */
   int tsock;   /* Socket to connect to tracker server */
   struct sockaddr_in server, client;
   socklen_t clientlen;
   struct sockaddr *serverptr = (struct sockaddr*)&server;
   struct sockaddr *clientptr=(struct sockaddr *)&client;
   struct hostent *rem;
   struct in_addr ip;
   int enableSocketOption = 1;

   int err,status,chunksnum;
  

   /* Signal handler definition - Dispositioning of signals SIGINT and SIGTERM to clean resources before termination of peer */
   signal(SIGINT,signal_handler);
   signal(SIGTERM,signal_handler);

   /* Executable client has only one parameter -> configuration file */
   if(argc!=2)
   {
      fprintf(stderr,"Usage ./tclient <configuration file>\n");
      exit(EXIT_FAILURE);
   }
   
   /* Opening Configuration file for reading. Exiting upon failure */
   if((fp=fopen(argv[1],"rb"))==NULL)
   {
      perror("Open ");
      exit(EXIT_FAILURE);
   }

   /* Reading File and saving data in peer's data structure */
   if(Parse_Conf_File(fp,&peer)<0)
      exit(EXIT_FAILURE);

   /* Closing Peer Configuration file after storing all data in memory */
   if(fclose(fp)==EOF)
   {
      perror("Close ");
      exit(EXIT_FAILURE);
   }
 
   /* Creating socket to connect to tracker */
   if((tsock = socket(PF_INET, SOCK_STREAM, 0)) < 0)
   {
      perror("socket");
      exit(EXIT_FAILURE);
   }
    
   server.sin_family = AF_INET;       			       /* Internet domain */
   server.sin_addr.s_addr = inet_addr(peer.tracker_ip);        /* IP address */  
   server.sin_port = htons(peer.tracker_port);                 /* Tracker Server port */

   /* Initiate connection with the tracker Server */
   if (connect(tsock, serverptr, sizeof(server)) < 0)
   {
       perror("connect");
        exit(EXIT_FAILURE);
   }

   //printf("Connected to %s port %d\n", peer.tracker_ip, peer.tracker_port);

   /* Request Tracker to create/join Swarm (Using SwarmRequest/JoinSwarmRequest Communication Protocol) */
   if (Tracker_Request(&peer,tsock)<0)
      exit(EXIT_FAILURE);

   /* Getting Response from Tracker. Terminating Connection in case of Bad Response or Invalid JoinResponse Protocol is read */
   if (Tracker_Response(&peer,tsock)<0)
      exit(EXIT_FAILURE);

   /* Initializing Peer list data structure of the peer */
   PL_Init(&peer.peerlist);
   /* Initializing Termination variable */
   term_var=0;

   /* Initializing Bitmap data structure */
   chunksnum=ceil((float)peer.filemdata.filesize/(float)peer.ChunkSize);
   if(BMP_Init(&Bitmap,chunksnum))
   {
      fprintf(stderr,"Error Initializing Peer's Bitmap\n");
      exit(EXIT_FAILURE);
   }

   /* If peer is seeder, setting all bits to 1 (Seeder possesses the whole file ) */
   if(peer.type==SEEDER)
   {
      int i;
      /* Setting all bitmap bits to 1 (Seeder has all chunks of the file) */
      for(i=0;i<chunksnum;i++)
      {
         if(BMP_Set(Bitmap,i))
         { 
           printf("Error Setting bit %d of Bitmap",i);
           exit(EXIT_FAILURE);
         }
      } 
  
   }
   /* If peer is leecher, creating empty file */
   else
   {
      FILE *filep;
      char* filepath;

      /* Full file path */
      if((filepath=malloc(strlen(peer.wd)+strlen(peer.filemdata.filename)+2))==NULL)
      {
          fprintf(stderr,"No memory available\n");
      	  exit(EXIT_FAILURE);
      }
      
      sprintf(filepath,"%s/%s",peer.wd,peer.filemdata.filename);
    
      /* Creating empty file */
      if((filep=fopen(filepath,"wb"))==NULL)
      {
          perror("open");
          exit(EXIT_FAILURE);
      }
    
      /* Closing the file */
      if(fclose(filep)==EOF)
      {
          perror("close");
          exit(EXIT_FAILURE);
      }

      free(filepath);
   }

   /* Initializing a mutex controlling peer's main data structure access of the threads */ 
   pthread_mutex_init(&peer_mutex,NULL);

   /************************************************ MULTITHREADING *************************************************************************************/

   /* Leecher peer - Client thread */
   if(peer.type==LEECHER)
   {
      /* Creating Client thread (Leecher), responsible to make ChunkRequests and receiving ChunkResponses till getting the entire shared in the swarm */
      if (err=pthread_create(&cl,NULL,Client_thread,NULL))
      {
         printf("Error Creating Heart Beat - Peer List thread\n");
         exit(EXIT_FAILURE);
      } 
   }
  
   /* Creating Heart Bit - Peer List thread, responsible to receive these messages from Tracker */
   if (err=pthread_create(&hb_pr,NULL,HB_PL_thread,(void*)&tsock))
   {
       printf("Error Creating Heart Beat - Peer List thread\n");
       exit(EXIT_FAILURE);
   }   

   /* Creating multithreaded server listening socket */
   if ((lsock = socket(PF_INET, SOCK_STREAM, 0)) < 0)
   {
        perror("socket");
        exit(EXIT_FAILURE);
   }
   
   server.sin_family = PF_INET;       		 /* Internet domain */
   server.sin_addr.s_addr = htonl(INADDR_ANY);
   server.sin_port = htons(peer.peer_port);      /* Peer's server port */

   setsockopt(lsock, SOL_SOCKET, SO_REUSEADDR, &enableSocketOption, sizeof(int)); /* For socket reuse */

   /* Bind socket to address */
   if (bind(lsock, serverptr, sizeof(server)) < 0)
   {
        perror("bind");
        exit(EXIT_FAILURE);
   }
 
   /* Listen for connections */
   if (listen(lsock, 5) < 0)
   {
        perror("listen");
        exit(EXIT_FAILURE);      
   }
 
   printf("\nListening on port: %d\n", peer.peer_port);
   if(peer.type==LEECHER)
      printf("Leeching file: %s/%s\nFile md5-sum: %s\n",peer.wd,peer.filemdata.filename,peer.filemdata.filemd5);
   else
      printf("Seeding file: %s/%s\nFile md5-sum: %s\nFile size in bytes: %ld\nChunk Size: %d\nNumber of Chunks: %d\n",peer.wd,peer.filemdata.filename,peer.filemdata.filemd5,peer.filemdata.filesize,peer.ChunkSize,chunksnum);

   
   clientlen=sizeof(client);

   /************************** Managing Multi-threaded Server ***********************/
   while(1) 
   {
        int *srvsock;  /* Server socket */
    
        srvsock=malloc(sizeof(int));  /* Allocating socket */

        /* Accept connection */
    	*srvsock = accept(lsock, clientptr, &clientlen);
       
        /* Checking accept return code */
        if(*srvsock<0 && term_var==0)
        {
           perror("accept");
           exit(EXIT_FAILURE);      
        }
        /* Checking Termination variable in case of signal */
        else if(term_var!=0)
           break;

    	/* Find client's address */
    	if ((rem = gethostbyaddr((char *) &client.sin_addr.s_addr, sizeof(client.sin_addr.s_addr), client.sin_family)) == NULL) 
        {
    	    herror("gethostbyaddr"); 
            exit(EXIT_FAILURE);
        }

      
    	printf("Accepted connection from: %s (%s)\n", rem->h_name, (char*)inet_ntoa(client.sin_addr));
    	
   
        /* Creating Thread to read ChunkRequest from client and send ChunkResponse */
        if (err=pthread_create(&srv,NULL,Server_thread,(void*)srvsock))
        {
           printf("Error Creating Server thread\n");
           exit(EXIT_FAILURE);
        }
   
   }

   /****** Joining Threads Upon Exiting  ******/
   if (err=pthread_join(hb_pr,(void**)&status))
   {
       printf("Error join (heart bit - peer list thread)");
       exit(EXIT_FAILURE);
   }
   
   if(peer.type==LEECHER)
   {
     if (err=pthread_join(cl,(void**)&status))
     {
        printf("Error join (client thread)");
        exit(EXIT_FAILURE);
     } 
   }
  
   /* Exiting Peer Program */
   exit(EXIT_SUCCESS);
}

/* Signal Handler - Assigning Termination Variable to 1 in order to be handled suitably by every thread. Also unblocks accept. */
void signal_handler(int signo)
{
   int status,err,flags; 

   /* Setting Termination Variable */   
   term_var=1;   

   /* Unblocking accept, so that main thread can terminate */
   if ((flags = fcntl(lsock, F_GETFL, 0)) < 0)
   {
     perror("fcntl ");
     exit(EXIT_FAILURE);
   }

   if (fcntl(lsock, F_SETFL, flags | O_NONBLOCK) < 0)
   {
     perror("fcntl ");
     exit(EXIT_FAILURE);
   }

   /* Destructor - Cleaning Resources at exit */
   if(atexit(&Clean_Resources))
   {
     fprintf(stderr,"Error Cleaning Resources\n");
     exit(EXIT_FAILURE);
   }

   printf("\nCleaning resources...\n"); 
   printf("Terminating peer client...\n");

}
    
/* Clean all resources */
void Clean_Resources()
{
   /* Free allocated memory */
   free(peer.filemdata.filename);
   free(peer.filemdata.filemd5);
   free(peer.wd);
   free(peer.tracker_ip);

   /* Destroy peerlist */
   PL_Destroy(&peer.peerlist);

   /* Destroy Bitmap */
   BMP_Release(Bitmap);
}
   
