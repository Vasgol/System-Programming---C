/************************************************** Data structures definitions *******************************************************************/
#include <pthread.h>

#define SEEDER  0
#define LEECHER 1

/* File Metadata */
typedef struct{
                 char* 	filename;     /* File's Name */
                 char*  filemd5;      /* File's Valid MD5 */
                 long   filesize;     /* File's size in bytes */

              }FileMData;

/* Peer List taken by tracker server */
typedef struct Plist{
                 char*         ip;
                 unsigned int  port;
                 struct Plist *next;
              }PeerList;

/* Peer Main Data structure */
typedef struct{  
                 int   		type;         /* Peer Type (SEEDER or LEECHER) */
                 char* 		wd;           /* Working Directory of peer where the file to be downloaded or uploaded from */ 
                 char*          tracker_ip;   /* Tracker Server Ip */
                 unsigned int   tracker_port; /* Tracker's Listening port */
                 unsigned int   peer_port;    /* Peer's Server Listening port */
                 FileMData 	filemdata;    /* File's MetaData */
                 int 		ChunkSize;    /* Chunk Size in Bytes */
                 PeerList*	peerlist;     /* List of swarm's peers */

               }Peer;


Peer 	       peer;   			       /* Peer Data structure Shared By All threads and Controlled by the following mutex */
unsigned char* Bitmap;                         /* Bitmap data structure */
unsigned int   term_var;                     /* Is set to 1 by signal handler in order to terminate peer */

pthread_mutex_t peer_mutex;                    /* Mutex Definition used for sychronization of threads accessing peer main data structure */

pthread_t hb_pr;       			       /* Defining hb_pr  thread  */
pthread_t cl;          			       /* Defining client thread */
pthread_t srv;                                 /* Defining server thread */

/* PeerList Functions */
void PL_Init(PeerList **peerlist);
int PL_Insert(PeerList **peerlist, char *ip, unsigned int port);
PeerList* PL_Get_Next(PeerList *peernode);
void PL_Destroy(PeerList **peerlist);
void PL_Print(PeerList *peerlist);

/* Signal Handler - Clean resources */
void signal_handler(int signo);
void Clean_Resources();
