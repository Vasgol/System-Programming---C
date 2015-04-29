/********************************************* Computer Player (Parent Process) functions ***********************************************************************/
#include <curses.h>
#include <string.h>
#include "Computer.h"

/************************************ PARENT PROCESS - COMPUTER PLAYER **********************************************/
int Computer_Player(int *p1, int *p2)
{
      /* Variables Definitions */ 
      int l,c,i=0,tries=0,status;
      int hitline, hitcol;
      int first=0 , second=0; 
      int fhitline, fhitcol;
      int shitline, shitcol;
      int nhitline[10], nhitcol[10];
      int phitline, phitcol;
      int pphitline, pphitcol;
      int hpoints=0,seek=0;
      int pres=0;
      int found=0,failures=0;
      int total=0;
    

      /* First Computer Player (Parent Process) places randomly ships into grid and also fills its own algorithm table */
      computer_ships();
      

      /*********************************** GAME STARTING **************************************************/
      while(total<17)
      {
        i++;
	char hitpoint[4];  // Getting human player's hit
        char result[2];   //  Holding result of a hit
        char res;         
        char endofgame[2]="N";  // Holding state of game 'N'->game continues or 'Y'-> game ends

        /* 1. Computer Player (Parent Process) gets human hit from pipe */
        read(p1[READ],hitpoint,3);
        /* 2. Computer Player (Parent Process) gets result of previous hit from pipe. This is used later for its artificial intelligence */
        read(p1[READ],result,2);

        /* Computer is informed if succeeded 'P', failed 'N' or destroyed human's ship (other result varying depending from ship type) */
        if(result[0]=='P')
           pres=1;
        else if(result[0]=='N')
           pres=0;
        else
        {   
            pres=2;
            res=result[0];
        }
    
        /* Holding hitline, hitcolumn real values for computer's algorithm grid table. */
	hitline=hitpoint[0]-65;
        if(strlen(hitpoint)==2)
          hitcol=hitpoint[1]-'0';
        else
          hitcol=10; 

        /* Computer checking its ship table. Answering if human player's hit was successful or not */
        if(table[hitline][hitcol-1]==EMPTY) // Human failed 
           strcpy(result,"N");
        else  // Human succeeded
        {
         
           strcpy(result,"P"); 
 
           /* Checking if human destroyed a computer's ship */
           if(--ship[table[hitline][hitcol-1]].hitpoints==0)  
             result[0]=stype(table[hitline][hitcol-1]);
 
           /* Increment of total successful points that human hit */
           total++;
 
           /* Assigning endofgame if human hit all computer's battleships */
           if(total==17)
           strcpy(endofgame,"Y");
        }
     
        /* 3. Computer (Parent Process) pipes result of human hit */
        write(p2[WRITE],result,2);

        /* 4. Computer pipes endofgame */
        write(p2[WRITE],endofgame,2);    
 
        /* Parent Process (Computer) waiting for child (human player) to terminate first before exiting */
        if(!strcmp(endofgame,"Y"))
            return 0;

        /***************************** COMPUTER PLAYER TURN TO HIT ***********************************/
        /* Useful variables:  # first:    1  (Computer found a random point where a human's ship is placed)         0 (Unsuccessful computer random hit)  
                              # second:   1  (Computer found the second point of the ship after the first random)   0 (Still not found the second successful point)
                              # pres:     Holds previous result of computer 
        */
        
        /*** FIRST CASE: Computer seeks a random point in human's grid  ***/ 
        if(first==0 && second==0 && pres==0)
        {
             generate_random_hit(&hitline,&hitcol);
             failures=0;
        }
        /*** SECOND CASE: Computer hits a neighbour point of after found the first random point where a human's ship is placed. In this case seeks all the neighbour points next to the first */      
        else if(pres==1 && first==0 && second==0)
        {
              int direction,flag=0,i=0;

              // Holding First point computer hit successfully
              fhitline=phitline;   
              fhitcol=phitcol;     

              // Marking first (flag variable)
              first=1;

              // Used to hold in memory of computer player in order to seek other neighbour ships 
              nhitline[0]=fhitline;
              nhitcol[0]=fhitcol;

              hit_next_point(&hitline,&hitcol,phitline,phitcol);
         } 
         /*** THIRD CASE: Computer continues seeking neighbour points of first successful point hit till finding the second one. (Smallest ship is MineSweeper 2 points) */
         else if(first==1 && second==0 && pres==0)
              hit_next_point(&hitline,&hitcol,fhitline,fhitcol);
   
         /*** FOURTH CASE: After computer found the second point of enemy's ship continue's till destroying it. Also seeks for neighbour enemy's ships if they exist and destroys them too. */ 
         else
         {
              int flag=0;

              /* For each second and other successful points computer hits */
              if(pres==1)
              {
                   /* Marking second flag variable */
                   second=1;
 
                   /* Assigning Second/next hitline and hitcolumn */ 
                   shitline=phitline;
                   shitcol=phitcol;

                   /* If Computer is not in seeking mode (seek=0) of neighbour ships */
                   if(seek==0)
                   {
                 
		           hpoints++;        /* Counting successful consecutive points that computer hit */
		            
                           /* Holding consecutive successful points */ 
		           nhitline[hpoints]=shitline;    
		           nhitcol[hpoints]=shitcol;

                        
                           /* Sorting successful computer's points hit */

                           /* If computer hits points in same line, sorting by increasing values of columns */
		           if(nhitline[hpoints-1]==nhitline[hpoints])
		           {
		              
		                 int i=hpoints+1;
		                 int swap;

		                 while(--i>=0)
		                     if(nhitcol[i]<nhitcol[i-1]) 
                                     {
                                       swap=nhitcol[i];
                                       nhitcol[i]=nhitcol[i-1];
                                       nhitcol[i-1]=swap;
		                     }
		                   
		           }
                           /* If computer hits points in same column, sorting by increasing values of lines */
		           else if(nhitcol[hpoints-1]==nhitcol[hpoints])
		           {
		               int i=hpoints+1;
		               int swap;
  
		               while(--i>=0)
		                   if(nhitline[i]<nhitline[i-1]) 
                                   {
                                     swap=nhitline[i];
                                     nhitline[i]=nhitline[i-1];
                                     nhitline[i-1]=swap;
		                   }
                                  
		           }
                   
                   }
                   
               }
               /* If computer fails to finds third point of a ship, increasing failures and changing direction to seek next point */
               else if(pres==0)
               {                            
                    failures++;

                    /* If reached 2 failures and haven't destroyed a ship then computer must seek ships in other directions */
                    if(failures==2 && seek==0)
                       seek=1;  /* Assigning seek mode */

                   

                    swap(&shitline,&fhitline);
                    swap(&shitcol,&fhitcol);
 
                }
                /* If computer managed to destroy a ship, decreasing number of successful points hit by length of the ship */
                else if(pres==2 && seek==0)
                {
                  seek=1;                   /* Getting in seek mode for other ships */
                  hpoints-=slength(res)-1;
                }

                 /* If reached 2 failures or destroyed a ship and exceeded seeking points to find neighbour ships then computer generates a new random value to hit */
                 if((failures==2 || pres==2) && hpoints<0)
                 {
                    /* Reseting all variables and generating random value */ 
                    first=0;
                    second=0;
                    seek=0;
                    hpoints=0;
		    generate_random_hit(&hitline,&hitcol);
		    failures=0; 
                 }
                 /* If reached 2 failures but there are neighbour ships around to seek */
                 else if(failures==2 && hpoints>=0)
                 {
                    /* Reseting second point because now computer seeks a ship in another direction */
                    second=0;
                    /* Reseting failures */
                    failures=0;
                     
                    /* Seek mode on */
                    seek=1;

                    /* Getting first hit point */
                    fhitline=nhitline[hpoints];
                    fhitcol=nhitcol[hpoints];

                    /* Reducing counter of points to seek for neighbour ships */
                    hpoints--;

                    /* Hitting next point */
		    hit_next_point(&hitline,&hitcol,fhitline,fhitcol);

                 }
                 /* If Computer hasn't failed twice continues to seek a ship in the particular direction of the first two points.*/
                 else
                 {
                   /* Computer hits next point after at least 2 point hit are successful */
                   do{
                          /* Hitting next point in same line */
                          if(shitline==fhitline)
		          {
		            if(shitcol<fhitcol && shitcol-1>=0)
		            {
                         
		                hitcol=shitcol-1;
		                hitline=shitline;
		                flag=1;
		             
		            }
		            else if(shitcol+1<=9)
		            { 
                          
		                hitcol=shitcol+1;
		                hitline=shitline;
		                flag=1;
		             
		            }
		               
		               
		         }
                         /* Hitting next point in same column */
		         else
		         {
		            if(shitline<fhitline && shitline-1>=0)
		            {
                    
		                hitline=shitline-1;
		                hitcol=shitcol;
		                flag=1;
		             
		            }
		            else if(shitline+1<=9)
		            {
                
		                hitline=shitline+1;
		                hitcol=shitcol;
		                flag=1;
		            }
		         }

                         /* If next point is out of bound or hit before then marking as a failure and changing direction */
                         if(flag==0 || rtable[hitline][hitcol]==HIT)
                         {
                           /* Stop seeking next point if reached 2 failures */
                           if(failures==2)
                           break;

                           failures++;

                           /* Swapping last successful hit point with first hit point in order to hit in the other direction */
                           swap(&shitline,&fhitline);
                           swap(&shitcol,&fhitcol);
                         }

                 	}while(flag==0 || rtable[hitline][hitcol]==HIT);

                    /* If reached 2 failures and not in seek mode */
                    if(failures==2 && seek==0)
                    {
                      /* Getting in seek mode */
                      seek=1;
                      /* If just destroyed in ship holding the remaining successful points */ 
                      if(pres==2)
                      hpoints-=slength(res)-1;
                     
                   
                    }

                    /* If reached 2 failures or destroyed a ship and no other neighbour ships in the area, reseting all variables and generating a new point to hit */
                    if((failures==2 || pres==2) && hpoints<0)
                    {
                      first=0;
                      second=0;
                      seek=0;
                      hpoints=0;
		      generate_random_hit(&hitline,&hitcol);
		      failures=0; 
                    }
                    /* Reached 2 failures but there are still ships near for computer to seek */
                    else if(failures==2 && hpoints>=0)
                    {
                      /* Reseting second point because now computer seeks a ship in another direction */
                      second=0;
                      /* Reseting failures */
                      failures=0;
                     
                      /* Seek mode on */
                      seek=1;

                      /* Getting first hit point */
                      fhitline=nhitline[hpoints];
                      fhitcol=nhitcol[hpoints];

                      /* Reducing counter of points to seek for neighbour ships */
                      hpoints--;

                      /* Hitting next point */
		      hit_next_point(&hitline,&hitcol,fhitline,fhitcol);


                     }
                      
                        
                 }
               }
       
        /* Marking rival human's player hitpoint as HIT in order not to hit again in next moves */
        rtable[hitline][hitcol]=HIT;

        /* Holding computer move to use it to next move */
        phitline=hitline;
        phitcol=hitcol;

        /* Transforming to string form */
        sprintf(hitpoint,"%c%d",hitline+65,hitcol+1);
        
        /* 5. Computer Pipes move to player */
        write(p2[WRITE],hitpoint,3); 
   
        /************************************** END OF COMPUTER MOVE *****************************/
        
        /* 6. Computer getting endofgame signal from human player */
        read(p1[READ],endofgame,2); 
 
        /* If game has ended computer player (Parent process) waits first for child process to terminate and then terminates itself */
        if(!strcmp(endofgame,"Y"))
           return 0;

      }//end of while
   
       return -1;
}

/* Generates a Random Point for COMPUTER HIT. */
void  generate_random_hit(int* hitline, int* hitcol)
{  
  do{
       *hitline = rand() % SIZE;   // hitpoint line
       *hitcol  = rand() % SIZE;   // hitpoint column
             
     }while(rtable[*hitline][*hitcol]==HIT);

}

/* Computer hits next point after found a first random where a human's ship is placed */
void  hit_next_point(int* hitline, int *hitcol, int phitline, int phitcol)
{
  int i=0,flag=0;

  do{
                  /* Computer hits a near point */
                  switch(i)
                  {
                       case 0:   if(phitline-1>=0)
                                 {
                                   *hitline=phitline-1;
                                   *hitcol=phitcol;
                                   flag=1;
                                 }
                                 break;
                                
                       case 1:   if(phitline+1<=9)
                                 {
                                   *hitline=phitline+1;
                                   *hitcol=phitcol;
                                   flag=1;
                                 }
                                 break;
                       case 2:   if(phitcol-1>=0)
                                 {
                                   *hitcol=phitcol-1;
                                   *hitline=phitline;
                                   flag=1;
                                 }
                                 break;
                       case 3:   if(phitcol+1<=9)
                                 {
                                   *hitcol=phitcol+1;
                                   *hitline=phitline;
                                   flag=1;
                                 }
                                 break;
                       default:  generate_random_hit(hitline,hitcol);
                                 flag=1;
                                 break;
                   }
         
                   i++;  
          
                }while(flag==0 || rtable[*hitline][*hitcol]==HIT);
}

/* Computer Function to place ships randomly in grid based on battleship game rules */
int computer_ships()
{
   int bow[2],stern[2];
   int direction, point;
   int i=0;


   /* Placing computer player's ships randomly (Ensuring that they placed in valid positions in table) */
   for(i=0;i<=4;i++)
   {
     srand((unsigned)time(NULL));
     do
     {
        /* Generate Random bowpoint for ship */
        bow[0]=rand()%SIZE;
        bow[1]=rand()%SIZE;

        /* Generate Random sternpoint for ship */
        stern[0]=rand()%SIZE;
        stern[1]=rand()%SIZE;

      }while(check_values(bow,stern,ship[i])==-1);
   }
}

/* Checking validity for random bowpoint sternpoint produced randomly by ships. If function returns 0 means that a computer ship is placed randomly into grid */
int check_values(int *bow,int *stern,Ship ship)
{
   int i;

   /* False values for bow, stern produced (Neither horizontal nor vertical placing) */
   if(bow[0]!=stern[0] && bow[1]!=stern[1])
      return -1;
   /* Horizontal placing */
   else if(bow[0]==stern[0])
   {
      /* Just having bow[1] value bigger than stern[1] value */
      if(bow[1]<stern[1])
      swap(&bow[1],&stern[1]);

      /* Checking if bow point - stern point distance aggrees with ship's length */
      if(bow[1]-stern[1]+1!=ship.length)
      return -1;

      /* Checking ship's points not to be placed upon another ship area */
      for(i=stern[1];i<=bow[1];i++)
          if(table[bow[0]][i]!=EMPTY)
          return -1;
 
      /* Placing ship in computer's algorithm table */
      for(i=stern[1];i<=bow[1];i++)
          table[bow[0]][i]=ship.type;

      return 0;
          
   }
   /* Vertical Placing (Same checkings) */
   else 
   {
      if(bow[0]<stern[0])
      swap(&bow[0],&stern[0]);

      if(bow[0]-stern[0]+1!=ship.length)
      return -1;

      for(i=stern[0];i<=bow[0];i++)
          if(table[i][bow[1]]!=EMPTY)
          return -1;
 
      for(i=stern[0];i<=bow[0];i++)
          table[i][bow[1]]=ship.type;

      return 0;
          
   }          

}


