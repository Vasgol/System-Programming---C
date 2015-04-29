/********************************************* Ship Functions ******************************************************************/
#include <curses.h>
#include <string.h>
#include "Ships.h"

/* Initializing players' Ships */
void Init_Ships()
{

   int i;
   int lengths[5]={5, 4, 3, 3, 2};

   for(i=0;i<5;i++)
   {
     ship[i].type=i;
     ship[i].length=lengths[i];
     ship[i].hitpoints=lengths[i];
     }
}

/* Printing a unique character for each Ship (Just a help function) */
char stype(int type)
{
      switch(type)
      {
         case -1 : return '-';
         case  0 : return 'A';
         case  1 : return 'B';
         case  2 : return 'F';
         case  3 : return 'S';
         case  4 : return 'M';
      }
}

/* Printing Ship length for ship type given */
int slength(char type)
{
      switch(type)
      {
         case 'A' : return 5;
         case 'B' : return 4;
         case 'F' : return 3;
         case 'S' : return 3;
         case 'M' : return 2;
      }
}


/* Printing a string for each Ship when human player is asked to place his ships */
void shiptype(char* shipname, int type)
{
    switch(type)
    {
       case AIRCRAFT_CARRIER: 	strcpy(shipname,"Aircraft Carrier");
                              	break;
       case BATTLESHIP: 	strcpy(shipname,"Battleship");
                              	break;
       case FRIGATE: 		strcpy(shipname,"Frigate");
                              	break;
       case SUBMARINE: 		strcpy(shipname,"Submarine");
                              	break;
       case MINESWEEPER: 	strcpy(shipname,"Minesweeper");
                              	break;
    }
}


/* Printing Ships hitpoints */
void print_ships_hp(int line, int col, int hitpoints)
{
   int i;

   emptyline(line,col);

   for(i=0;i<hitpoints*2;i+=2)
    mvaddstr(line,col+i,"* ");
   

   if(hitpoints==0)
    mvaddstr(line,col,"Destroyed");
   

   refresh();
}

/* Printing Human Ships names and hitpoints in screen */
void print_ships()
{
   int i;

    mvaddstr(19,60,"YOUR SHIPS");
    mvaddstr(19,80,"HITPOINTS");
    mvaddstr(20,60,"-----------------------------");
    mvaddstr(21,60,"Aircraft Carrier");
    print_ships_hp(21,80,ship[AIRCRAFT_CARRIER].hitpoints);
    mvaddstr(22,60,"Battleship");
    print_ships_hp(22,80,ship[BATTLESHIP].hitpoints);
    mvaddstr(23,60,"Frigate");
    print_ships_hp(23,80,ship[FRIGATE].hitpoints);
    mvaddstr(24,60,"Submarine");
    print_ships_hp(24,80,ship[SUBMARINE].hitpoints);
    mvaddstr(25,60,"Minesweeper");
    print_ships_hp(25,80,ship[MINESWEEPER].hitpoints);

    refresh();
}
