/********************************************* Human Player (Child Process) functions ***********************************************************************/
#include <curses.h>
#include <string.h>
#include <unistd.h>
#include "Human.h"
    
/************************************ CHILD PROCESS - HUMAN PLAYER **********************************************/
int Human_Player(int* p1, int* p2)
{
  int i, sl, sc, el, ec, total=0;
  char cmd[10],d1[10],d2[10];
  char shipname[20];
  char message[30];
  char endofgame[2]="N", result[2]="N";

  /*************** Human inputs bowpoint, sternpoint for each ship and places them into grid suitably according with rules *********/

  /* Just printing the rules of ship placement */
//  help_rules();

  /* Human's ships' placement */
  for(i=0;i<=4;i++)
  {
                   
     shiptype(shipname,i);  // Just getting every ship name to print   
  
     /* Human player inputs bow, stern point of every ship which are checked. If are valid ship is placed into human player's grid */
     do{
	   refresh();

	   emptyline(40,30);
	   move(40,30);
	   printw("Give bow point of %s (length=%d) : ",shipname,ship[i].length);
	   scanw("%s",d1);
	   emptyline(40,30);
	   move(40,30);
	   printw("Give stern point of %s (length=%d) : ",shipname,ship[i].length);
	   scanw("%s",d2);
 
	  }while(place_ship(d1,d2,ship[i])==-1); // Calling function to place ships of human player doing the necessary checks. 
	 
	  emptyline(40,30);
   }
 
    /* Just printing ships of human player with their hitpoints. These are dynamically changed in screen each time a ship is hit */
    print_ships();

  //  delete_mes();

    /******************************** GAME STARTING *************************************/
	
    /* Printing terminals for human player input and rival computer player answers */     
    emptyline(40,30);
    mvaddstr(39,30,"Commands: HIT (A1 to J10) , EXIT (to terminate game)");
    mvaddstr(40,30,"YOU > ");
    mvaddstr(42,30,"RIVAL > ");

    i=0;

    /***** Endless loop until player is over or user player inputs EXIT *****/    
    while(strcmp(cmd,"EXIT"))
   {
       int l,c,x,y,hitline,hitcol;
       char hitpoint[4]="\0";
      
       emptyline(40,36);
       move(40,36);
       
       i++;

       /* Human player plays by typing <COMMAND> <GRIDPOINT> where COMMAND=HIT or EXIT and GRIDPOINT A1 to J10 */
       move(40,36);
       scanw("%s %s",cmd,d1);
 

       /* Getting algorithm tables line and column values from human hitting gridpoint */
       l=d1[0]-65;
       c=d1[1]-'0'-1;
       if(strlen(d1)==3)
       c=9;
   
       emptyline(41,30);
       

       /* Semantic checking COMMAND and GRIDPOINT validities */
       if(strcmp(cmd,"HIT")!=0 && strcmp(cmd,"EXIT")!=0)
       {
	 move(41,36);
	 printw("Wrong command: %s",cmd);
	 emptyline(40,36);
	 continue;
       }
       else if(mapping(d1,&x,&y,6)==-1)
       {
	 move(41,36);
	 printw("Invalid point %s! Hit a point in grid (A1 to J10)!",d1);
	 emptyline(40,36);
	 continue;
       }
       /* Checking if gridpoint to be hit is already hit */ 
       else if(rtable[l][c]==HIT)
       {
	 move(41,36);
	 printw("You already hit this point %c%d! Hit another!",l+65,c+1);
	 emptyline(40,36);
	 continue;
       }
      
  
       /* Marking point human hits as HIT in rivals algorithm table in order not to be hit again in next moves */
       rtable[l][c]=HIT;

       /* 1. Child Process (Human) pipes point that hits to Parent Process (Computer) */            
       write(p1[WRITE],d1,3);
       /* 2. Child Process (Human) pipes the result of computer's hit to Parent Process (Computer) */
       write(p1[WRITE],result,2);
       /* 3. Computer pipes user the result of the hit (successful or not) */
       read(p2[READ],result,2);

       /* Printing a Message in RIVAL terminal (Computer) of Human player's hit */
       hitresult(message,result[0]);
       emptyline(42,38);
       move(42,38);        
       printw("%s",message);

       /* Printing the result of Human player hit in RIVAL GRID */
       if(strcmp(result,"N"))
       {
	  mapping(d1,&l,&c,6);
	  mvaddstr(l,c,"x");
       }
       else
       {
	  mapping(d1,&l,&c,6);
	  mvaddstr(l,c,"+");
       }

       /* 4. Computer informing user if successfully hit all battleships. In this case game ends (Human WINS) */
       read(p2[READ],endofgame,2);

       if(!strcmp(endofgame,"Y"))
       {
	  emptyline(39,30);
	  emptyline(40,30);
	  emptyline(42,30);
          mvaddstr(39,30,"END OF GAME");
	  mvaddstr(41,30,"Congratulations! You WON!");
          mvaddstr(42,30,"(Press any key to exit)");
	  getch();
	  return 0;
       }

       /* 4. Getting Computer Move from pipe */
       read(p2[READ],hitpoint,3);
   
       
       /* Getting human player's algorithm table's gridpoint position that computer hit */ 
	if(strlen(hitpoint)==3)
	  hitcol=10;
	else
	  hitcol=hitpoint[1]-'0';
	

	hitline=hitpoint[0]-65;

        /* Printing a message in RIVAL terminal of the point computer that hit */
	move(42,38+strlen(message));
	printw("Computer hits %s ",hitpoint);
 
        /* Printing Computer's point in human player's grid appering in screen */
	if(table[hitline][hitcol-1]==EMPTY) // Computer missed
	{
	  mapping(hitpoint,&l,&c,21);
	  mvaddstr(l,c,"+");
	  strcpy(result,"N");  // Holding result N (Negative)
	}
	else // Computer succeeded 
	{
	  mapping(hitpoint,&l,&c,21);
	  mvaddstr(l,c,"x");

	  strcpy(result,"P"); // Holding result P (Positive)

	  if(--ship[table[hitline][hitcol-1]].hitpoints==0)  // Reducing hitpoints of ship that is hit 
	     result[0]=stype(table[hitline][hitcol-1]);      // If all hitpoints of a ship hit then holding as result the sunk ship to pipe it to computer player 

	  print_ships_hp(21+table[hitline][hitcol-1],80,ship[table[hitline][hitcol-1]].hitpoints);  // Printing player's ship hitpoints dynamically in screen

	  total++;       // Incrementing total successful points that computer hit
	 
	  if(total==17)  // Total hitpoints of ships are reached. That means that all ships of human player are sunk.
	  strcpy(endofgame,"Y");
	}

	/* 5. Informing Computer if all battleships are hit. In this case Computer Player (Parent process) wins! */
	write(p1[WRITE],endofgame,2);
	
	if(!strcmp(endofgame,"Y"))
	{
	  emptyline(39,30);
	  emptyline(40,30);
	  emptyline(42,30);
          mvaddstr(39,30,"END OF GAME");
	  mvaddstr(41,30,"Computer WON!");
          mvaddstr(42,30,"(Press any key to exit)");
	  getch();
	  return 0;
	}
          
    }

    return -1;
}


/* Human Player function used for placing the ships suitably into grid and informing human player algorithm table */
int place_ship(char* bowpoint, char* sternpoint, Ship ship)
{
    int colbindex,colsindex;
    int bow[2], gridbow[2];
    int stern[2], gridstern[2];
    int i,j;
    int startpoint, endpoint, stablepoint;
    int start, end;
   
  
    /* Checking bowpoint and stempoint given by user validity getting also real grid values in screen displayed */
    if(mapping(bowpoint,&gridbow[0],&gridbow[1],21)==-1 || mapping(sternpoint,&gridstern[0],&gridstern[1],21)==-1)
    {
      move(41,60);
      printw("Invalid bow point or stern point! (Valid points A1-J10)"); 
      return -1;
    }
    else
        emptyline(41,60);

    if(strlen(bowpoint)==3)
    colbindex=10;
    else
    colbindex=bowpoint[1]-'0';
 
    if(strlen(sternpoint)==3)
    colsindex=10;
    else
    colsindex=sternpoint[1]-'0';
    
    /* Getting table indexes */
    bow[0]=gridbow[0]-21;
    bow[1]=colbindex-1;
    stern[0]=gridstern[0]-21;
    stern[1]=colsindex-1;

    /* Ship must be placed only horizontally or vertically */ 
    if(bow[0]!=stern[0] && bow[1]!=stern[1])
    {
     move(41,60);
     printw("Ships must be placed horizontally or vertically");
     return -1;
    }
    /* Checking bow and stern points in table. They must be free (Not another ship placed in this position) */
    if(table[bow[0]][bow[1]]!=EMPTY || table[bow[0]][bow[1]]!=EMPTY)
    {
     move(41,60);
     printw("Ship cannot be placed. Other Ship(s) in this area!");     
     return -1;
    }
    else
        emptyline(41,60);

    /* Horizontal positioning of the ship */
    if(bow[0]==stern[0])
    {
     
      stablepoint=bow[0];   // Stable point -> same line in horizontal positioning
      startpoint=bow[1];
      start=gridbow[1];
      endpoint=stern[1];
      end=gridstern[1];
    }
    /* Vertical positioning of the ship */
    else
    { 
      stablepoint=bow[1];   // Stable point -> same column in vertical positioning
      startpoint=bow[0];
      start=gridbow[0];
      endpoint=stern[0];
      end=gridstern[0];
    }

    /* Ensuring that startpoint in grid has smaller value than endpoint. */
    if(startpoint>endpoint)
    {
       swap(&startpoint,&endpoint);
       swap(&start,&end);
    }
  
    /* Checking if bow-stern positions of the ship aggree with its length */
    if(endpoint-startpoint!=ship.length-1)
    { 
     char shipname[20];
   
     shiptype(shipname,ship.type);

     move(41,60);
     printw("%s has length %d. Follow rule 1.",shipname,ship.length);
     return -1;
    }
      
    /* Horizontal placing */ 
    if(bow[0]==stern[0])
    {
      /* First Checking if there is another ship in any point between bow and stern */
      for(i=startpoint;i<=endpoint;i++)
        if(table[stablepoint][i]!=EMPTY)
        {
	   move(41,60);
	   printw("Ship cannot be placed. Other Ship(s) in this area!");     
	   return -1;
    	}	
       
      /* Placing the ship. Grid points content has now the particular ship type */
      for(i=startpoint, j=start;i<=endpoint;i++, j+=2)
      {
          table[stablepoint][i]=ship.type;
          mvaddstr(stablepoint+21,j,"o");
      }
    }
    else
    {

      /* First Checking if there is another ship in any point between bow and stern */
      for(i=startpoint;i<=endpoint;i++)
        if(table[i][stablepoint]!=EMPTY)
        {
	   move(41,60);
	   printw("Ship cannot be placed. Other Ship(s) in this area!");     
	   return -1;
    	}	
       
       
      /* Placing the ship. Grid points content has now the particular ship type */
      for(i=startpoint;i<=endpoint;i++)
      {
          table[i][stablepoint]=ship.type;
          mvaddstr(i+21,gridbow[1],"o");
      }
    }
      
    return 0; 
    
}

/* Printing message depending from result getting from hitting a rival point */
void hitresult(char* message, char result)
{
  
      switch(result)
      {
         case 'P' : strcpy(message,"You hit a ship point! ");
                    break;
         case 'N' : strcpy(message,"No ship in this point! ");
                    break;
         case 'A' : strcpy(message,"You destroyed the Aircraft Carrier! ");
                    break;
         case 'B' : strcpy(message,"You destroyed the Battleship! ");
                    break;
         case 'F' : strcpy(message,"You destroyed the Frigate! ");
                    break;
         case 'S' : strcpy(message,"You destroyed the Submarine! ");
                    break;
         case 'M' : strcpy(message,"You destroyed the Minesweeper! ");
                    break;
      }
}

/* Just printing rules of ship placement for the human player */
void help_rules()
{
  mvaddstr(32,30,"Please place your Ships giving bow point and stern point for each Ship according to the following rules: ");
  mvaddstr(33,30,"---------------------------------------------------------------------------------------------------------");
  mvaddstr(34,30,"1. Bow and stern points' distance of each ship must be exactly match with ship's length.");
  mvaddstr(35,30,"2. Ships must be placed horizontally or vertically");
  mvaddstr(36,30,"3. Ships can't be placed into areas that belong to other ships.");
  mvaddstr(37,30,"---------------------------------------------------------------------------------------------------------");
  refresh();
}

/* Deleting above help when game starts */
void delete_mes()
{
  int i;

  for(i=32;i<=37;i++)
     emptyline(i,30);

  refresh();
}

