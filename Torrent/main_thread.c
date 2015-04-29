/***************************************** Main Thread Functions and Help Functions used by every thread of the peer program *****************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "main_thread_lib.h"
#include "MD5_Lib.h"

/* Getting Configuration File Data and Storing them in Peer's Data Structure */
int Get_Conf_Data(Peer *p, char* buf1, char* buf2)
{
   /* Working Directory of file seeding/leeching */ 
   if(!strcmpin(buf1,"WorkingDirectory"))
   {
     if((p->wd=malloc(strlen(buf2)+1))==NULL)
     {
        fprintf(stderr,"No Memory Available");
     	return -2;
     }

     strcpy(p->wd,buf2);
   }
   /* Tracker's location IP */
   else if(!strcmpin(buf1,"TrackerIP"))
   {
     if((p->tracker_ip=malloc(strlen(buf2)+1))==NULL)
     {
        fprintf(stderr,"No Memory Available");
     	return -2;
     }

     strcpy(p->tracker_ip,buf2); 
   }
   /* Tracker's Listening Port */
   else if(!strcmpin(buf1,"TrackerPort"))
     p->tracker_port=atoi(buf2);
   /* Peer's Multi Threaded Server Listening Port */ 
   else if(!strcmpin(buf1,"PeerPort"))
     p->peer_port=atoi(buf2);
   /* Name of File shared in Swarm */
   else if(!strcmpin(buf1,"Filename"))
   {
     if((p->filemdata.filename=malloc(strlen(buf2)+1))==NULL)
     {
        fprintf(stderr,"No Memory Available");
     	return -2;
     }

     strcpy(p->filemdata.filename,buf2);
   }
   /* File's MD5 Sum (Seeder Conf File has FileMD5 NONE because it can compute its MD5 Sum. Leecher's knows the MD5 Sum of File requesting */ 
   else if(!strcmpin(buf1,"MD5"))
   {
     if((p->filemdata.filemd5=malloc(strlen(buf2)+1))==NULL)
     {
        fprintf(stderr,"No Memory Available");
     	return -2;
     }

     /* Defining Type of Peer looking at MD5 */    
     if(!strcmpin(buf2,"NONE"))
        p->type=SEEDER;
     else
     {
        p->type=LEECHER;
        strcpy(p->filemdata.filemd5,buf2);
     }
   }
   /* Chunk's Size in Bytes. Last Chunk of the file may have smaller size than ChunkSize */
   else if(!strcmpin(buf1,"ChunkSizeInBytes"))
     p->ChunkSize=atoi(buf2);
   /* In case of any other Description different to the above, configuration file is invalid */
   else
     return -1;

   return 0;
}
   
/* Reading Configuration File and obtaining all available data for the peer */
int Parse_Conf_File(FILE *fp, Peer *p)
{
   char token1[50], token2[50];
   int i=0,error=0;

   /* Reading line by line and Holding two tokens (Description - Value) */
   while(fscanf(fp,"%s %s",token1,token2)!=EOF)
   {
       /* Storing Data in Peer's Data structure */
       if((error=Get_Conf_Data(p,token1,token2))==-1)
       {
         fprintf(stderr,"Invalid Configuration File Data (%s %s).\n",token1,token2);
         return -1;
       }
       else if(error==-2)
         return -2;
 
       /* Counting number of values from Configuration File */
       i++;
   }
  
   /* Valid Configuration File must have exactly 7 Descriptions-Values */
   if(i!=7)
   {
      fprintf(stderr,"Invalid Configuration File (Insufficient Data provided for the peer)\n");
       return -1;
    }

   return 0; 
}

/* Returns File Size in Bytes of file pointed by stream fp */
long GetFileSize(FILE *fp)
{
   long size;
 
   /* Getting file pointer in last of file */
   fseek(fp,0,SEEK_END);
   /* Size of File in Bytes is the current position of file pointer in end of file */
   size=ftell(fp);
   /* Returning file point in the starting byte of the file */
   rewind(fp);

   return size;
}

/* Peer Requesting the Tracker to create/join Swarm (Seeder -> SwarmRequest , Leecher -> JoinSwarmRequest)  */
int Tracker_Request(Peer *p, int tsock)
{
   FILE *fp;         /* Swarm File pointer */
   char reqbuf[256]; /* Memory Buffer to hold SwarmRequest (Seeder) or JoinSwarmRequest (Leecher) */
   char messagetype[20];
   char *filepath;

   /* Seeker forming SwarmRequest Communication Protocol*/ 
   if(p->type==SEEDER)
   {
       /* Seeder - SwarmRequest Communication Protocol with Tracker */
       strcpy(messagetype,"SwarmRequest");

       /* Allocating memory to hold whole path of file to seed */
       if((filepath=malloc(strlen(p->wd)+strlen(p->filemdata.filename)+2))==NULL)
       {
        fprintf(stderr,"No Memory Available");
     	return -1;
       }
       /* Holding full path of file */  
       sprintf(filepath,"%s/%s",p->wd,p->filemdata.filename);

       /* Opening file for reading. Exiting upon failure */
       if((fp=fopen(filepath,"rb"))==NULL)
       {
          free(filepath);
	  perror("Open ");
	  return -2;
       }

       /* Allocating memory for file's md5 sum */
       if((p->filemdata.filemd5=malloc(40*sizeof(char)))==NULL)
       {
        fprintf(stderr,"No Memory Available");
     	return -1;
       }
       /* Computing file's md5 sum */
       if(MD5Sum_File(filepath,p->filemdata.filemd5)<0)
       {
	 fprintf(stderr,"Error computing MD5 Sum of file %s\n",p->filemdata.filename);
	 return -3;
       }
       /* Computing file's size in Bytes */
       p->filemdata.filesize=GetFileSize(fp);

       /* Closing File after computing its size */
       if(fclose(fp)==EOF)
       {
	  perror("Close ");
	  return -4;
       }
       
       free(filepath);
       /* Forming SwarmRequest Protocol */
       sprintf(reqbuf,"MessageType: %s\r\nFileName: %s\r\nFile-md5sum: %s\r\nFileSizeInBytes: %ld\r\nChunkSizeInBytes: %d\r\nPort: %d\r\n\r\n",messagetype,p->filemdata.filename,
                                                                                                            p->filemdata.filemd5,p->filemdata.filesize,p->ChunkSize,p->peer_port);

   }
   /* Leecher forming JoinSwarmRequest Communication Protocol */
   else
   {
       /* Leecher - JoinSwarmRequest Communication Protocol with Tracker */
       strcpy(messagetype,"JoinSwarmRequest");
       /* Forming JoinSwarmRequest Protocol */ 
       sprintf(reqbuf,"MessageType: %s\r\nFileName: %s\r\nFile-md5sum: %s\r\nPort: %d\r\n\r\n",messagetype,p->filemdata.filename,
                                                                                                            p->filemdata.filemd5,p->peer_port);
   }
    
   /* Sending SwarmRequest(Seeder) or JoinSwarmRequest(Leecher) to Tracker */
   if((write(tsock,reqbuf,strlen(reqbuf)))==-1)
   {
      perror("write ");
      return -4;
   }

   return 0;
}

/* Case insensitive string compare */
int strcmpin(char *string1, char *string2)
{
  int i,j;
  int e;

  if((e=strcmp(string1,string2))!=0)
  {
    for(i=0, j=0 ; i<strlen(string1) , j<strlen(string2) ; i++ , j++)
        if(toupper(string1[i]) != toupper(string2[i]))
            return e;
  }

  return 0;
}     

  
/* Reads a Single Line from Communication Protocol and getting description and value of particular line 
 * Returns 0 if line has description value - Returns 1 if line consists of only - Returns -1 if read from socket fails \r\n
																*/ 
int Parse_Response_Line(int sock, char* description, char* value)
{
   int i=0;
   char prevch,ch;
   char linebuf[100];
   
   do{
        /* Holding previous character read */
        prevch=ch;

        /* Reading from socket end character by character */
        if((read(sock,&ch,1))==-1)
        {
          perror("read ");
	  return -1;
        }
  
        /* Saving each character in a buffer */
        memcpy(linebuf+i,&ch,1);
        /* Reached end of line (last two characters are \r and \n) */
        if(prevch=='\r' && ch=='\n')
        {
           /* Terminating buffer string */
           linebuf[i-1]='\0';
           /* If line with only '\r' and '\n' terminating with code 1 */
           if(i==1)
              return 1;
           /* Parsing description - value couple */
           sscanf(linebuf,"%s %s",description,value);
          
           /* Breaking from loop */
           break; 
        }
        
        i++;

      }while(1);

   /* Returning code 0 in case of description - value couple read from socket */  
   return 0; 
}

/* Reads whole body of a Communication Protocol message which is included before an emptyline with \r\n and ends with an emptyline with \r\n */
int Parse_Response_Body(int sock, char* body)
{
    int i=0;
    char prevch,ch;
    
    do{
        /* Holding previous character read */
        prevch=ch;

        /* Reading from socket end character by character */
        if((read(sock,&ch,1))==-1)
        {
          perror("read ");
	  return -1;
        }
        /* Saving each character in body buffer */
        memcpy(body+i,&ch,1);
        /* Reached end of line (last two characters are \r and \n) */
        if(prevch=='\r' && ch=='\n')
        {
           /* Terminating buffer string */
           body[i-1]='\0';
           /* If line with only '\r' and '\n' terminating with code 1 */
           if(i==1)
              return 1;
          
           /* Breaking from loop */
           break; 
        }
        
        i++;

      }while(1);

     return 0;
}

/* Reading Join Response Protocol from Tracker. Leecher stores file's metadata. In case of invalid data an error code is returned
   in order to terminate connection with tracker.
*/
int Join_Response(Peer *p, int sock)
{
    char description[50], value[50];
    int code; 

    /* Reading Join Response Protocol line by line */
    while((code=Parse_Response_Line(sock,description,value))>=0)
    {
      if(code==0)
      {
         if(!strcmpin(description,"Filename:"))
         {
             if(strcmp(p->filemdata.filename,value))
             {
              printf("Filename invalid\n");
              return -1;
             }
         }
         else if(!strcmpin(description,"file-md5sum:"))
         {
              
             if(strcmp(p->filemdata.filemd5,value))
             {
                printf("Invalid MD5 Sum of File");
                return -2;
             }
         }
         else if(!strcmpin(description,"FileSizeInBytes:"))
         {
             long filesize;
             
             filesize=atol(value);
            
             if(p->type==SEEDER && p->filemdata.filesize!=filesize)
             {
                printf("Invalid File Size");
                return -3;
             }
             else if(p->type==LEECHER)
                p->filemdata.filesize=filesize;
                
         }
         else if(!strcmpin(description,"ChunkSizeInBytes:"))
         {
             int chunksize;

             chunksize=atoi(value);

             if(p->ChunkSize!=chunksize)
             {
                printf("Invalid Chunk Size");
                return -4;
             }  
         }
         else
         {
           printf("Unrecognized description of data (%s)",description);
           return -5;
         }
      }
      else if(code<0)
      return -6;
      else
      return 0;

    }

}

/* Reading Tracker Response from Tracker when peer sends a SwarmRequest (Seeder) or JoinSwarmRequest (Leecher) */  
int Tracker_Response(Peer *p, int tsock)
{
    char description[50], value[50];
    char body[50];
    int i=0,l=0,end=0;

    /* Tracker Response */
    if(Parse_Response_Line(tsock,description,value)==0)
    {
      if(!strcmp(description,"MessageType:"))
      {
         
        if(!strcmp(value,"JoinResponse"))
        {
          if(Join_Response(p,tsock)<0)
          return -1;
        }
        else if(!strcmp(value,"ErrorMessage"))
        {
           int code;

           /* Reading and ignoring response status message */
           while((code=Parse_Response_Line(tsock,description,value))==0);

           /* Printing Description of error */
           if(code==1)
           {
             while(Parse_Response_Body(tsock,body)==0)
             printf("%s\n",body);
           }

           return -2;
        }
        else
        {
          printf("Unrecognized description of data (%s)\n",value);
          return -3;
        }
      }
      else
      {
        printf("Unrecognized description of data (%s)\n",description);
        return -3;
      }
     
    }
  
   return 0;

}
    
