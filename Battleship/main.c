/***************************************** Main Function of BattleShip Game ***********************************************************/
#include <curses.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include "Computer.h"

/* Definition of a signal handler */
void signal_handler(int);
  
main()
{
   /* Variables definitions */
   int startline=2,startcol=30; 
   int p1[2], p2[2];   // Pipe descriptors
   int pid;
   
   /* Wraping SIGINT (Ctrl-C) signal to signal_handler */
   signal(SIGINT,signal_handler);

   /****************** CREATING 2 PIPES *******************/
  
   /* PIPE 1: Parent Process (Computer) reads , Child Process (Human) writes */ 
   if(pipe(p1)==-1)
   {
     perror("pipe error");
     exit(EXIT_FAILURE);
   }
  
   /* PIPE 2: Parent Process (Computer) writes , Child Process (Human) reads */ 
   if(pipe(p2)==-1)
   {
     perror("pipe error");
     exit(EXIT_FAILURE);
   }

   /* Initializing ncurses window for the game's interface */ 
   initscr();

   /* Clearing screen */
   clear();
  
   /* Ignoring default actions of some keys */
   cbreak();
 
   /* Refreshing, changes appear on screen */
   refresh();
   

   /* Welcoming Message to start the game */
   mvaddstr(25,60,"----------------------WELCOME TO BATTLESHIP GAME------------------------");
   mvaddstr(26,80,"(Press Any Key to Start game)");
   getch();
   emptyline(25,60);
   emptyline(26,80);

   /* Printing the battlegrids of the players */
   mvaddstr(startline,startcol,"-------RIVAL-------");
   battlegrid(6,startcol); 
   mvaddstr(startline+15,startcol,"--------YOU---------");
   battlegrid(21,startcol); 

   refresh();

   /* Initializing algorithm tables */
   Init_Tables();
   /* Initializing Ships */
   Init_Ships();
  
   /* Checking fork success */
   if((pid=fork())==-1)
   {
      endwin();
      exit(EXIT_FAILURE);
   }
   /************ Parent Process: Computer Player *******************/
   else if(pid>0)
   {
      int status=0;

      close(p1[WRITE]);  // Computer reads from pipe 1 
      close(p2[READ]);   // Computer writes in pipe 2

      /* Computer Player Function */
      if(Computer_Player(p1,p2)!=0) // Checking for failure
      {
         /* Closing remaining two pipe descriptors */
         close(p1[READ]);
         close(p2[WRITE]);
         /* Parent Waiting for child to terminate first */
         wait(&status);
         /* After child, parent terminates */
         exit(EXIT_FAILURE);
      }

      /* Closing remaining two pipe descriptors */
      close(p1[READ]);  
      close(p2[WRITE]);
      /* Parent Waiting for child to terminate first */
      wait(&status);
      /* After child, parent terminates */
      exit(EXIT_SUCCESS);

   }
   /************ Child Process: Human Player ***********************/
   else
   {
      close(p1[READ]);  // Human writes in pipe 1
      close(p2[WRITE]); // Computer reads from pipe 2
      
      /* Human Player Function */
      if(Human_Player(p1,p2)!=0)
      {
         /* Closing game's interface window before terminating */
         endwin();
         /* Closing remaining two pipe descriptors */
         close(p1[WRITE]);
         close(p2[READ]);
         /* Child terminates */
         exit(EXIT_FAILURE);
      }

      /* Closing game's interface window before terminating */
      endwin();
      /* Closing remaining two pipe descriptors */
      close(p1[WRITE]);
      close(p2[READ]);
      /* Child terminates */
      exit(EXIT_SUCCESS); 
   }

}

/* Signal Handler: If user inputs Ctrl-C then closing interface window and terminate game */
void signal_handler(int signo)
{
   endwin();
   exit(EXIT_FAILURE);
}

