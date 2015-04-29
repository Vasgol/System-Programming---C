/*********************** Tables Functions *************************/
#include "Tables.h"
#include <string.h>


/* Initialization of Tables */
void Init_Tables()
{
   int i,j;
   int l, c;


   for(i=0;i<SIZE;i++)
   {
        for(j=0;j<SIZE;j++)
        {
            table[i][j]=EMPTY;
            rtable[i][j]=NOT_HIT;
        }
   }

}

/* Prints BattleShip Tables using curses functions */
void battlegrid(int sline, int scol)
{
    int i,j,k=65,l=0;

    for(i=scol;i<scol+2*SIZE;i+=2)
    {
      l++;
      move(sline-2,i);
      printw("%d ",l);
      refresh();
    }
    
    refresh();
    
    for(i=sline;i<sline+SIZE;i++)
    {
     for(j=scol;j<scol+2*SIZE;j+=2)
     {

        move(i,scol-4);
        printw("%c",k);
        move(i,j);
        printw("^");
        
        refresh();
     }
     
     move(i,i+1);
       
     k++;
    }
}

/* Mapping a point (for example A1) to real position in battlegrid which appears during game */
int mapping(char* d, int *line, int *column, int startline)
{
  int i,j;

  /* Checking point validity */
  if(d[0]<'A' || d[0]>'J' || d[1]<'1' || d[1]>'9' || (strlen(d)==3 && (d[1]!='1' || d[2]!='0') ) || strlen(d)>3)
    return -1;
  
  /* Mapping: Getting line and column real values in battlegrid of the game */
  *line=(d[0]-65)+startline;
  if(strlen(d)<3)
  *column=(d[1]-'0')*2+28;
  else
  *column=10*2+28;
  return 0;
  
}

/* Just a function to move cursor in a particular position in a line and empty its content */
void emptyline(int line, int column)
{
  mvaddstr(line,column,"                                                                                                		                ");
} 

/* Prints computer table (Just a help function) */
void print_comp_table()
{
   int i,j,x,y;
   int k=6;
   char c;

   for(i=4,x=0;x<SIZE;i++,x++)
   {
      
        for(j=60,y=0;y<SIZE;j+=2,y++)
        {
          move(k,j);
          c=stype(table[x][y]);
          printw("%c ",c);
        }
 
       k++;
       move(k,50);
   }

   refresh();
}

/* Prints user table (Just a help function) */
void print_user_table()
{
   int i,j,x,y;
   int k=21;
   char c;

   for(i=4,x=0;x<SIZE;i++,x++)
   {
      
        for(j=60,y=0;y<SIZE;j+=2,y++)
        {
          move(k,j);
          c=stype(table[x][y]);
          printw("%c ",c);
        }
 
       k++;
       move(k,50);
   }

   refresh();
}


/* Just a help function used for swapping integer values */
void swap(int *x, int *y)
{
   int z;
  
   z=*x;
   *x=*y;
   *y=z;
}

