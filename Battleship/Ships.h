/***************************************************** Ships' Definitions *********************************************************************************/
/* Definitions of Ship Types */
#define   AIRCRAFT_CARRIER  0
#define   BATTLESHIP	    1	
#define   FRIGATE	    2
#define   SUBMARINE	    3
#define   MINESWEEPER	    4


/* Definition of Ship */
typedef struct {
                    int type;       
          	    int length;
                    int hitpoints;
 
                   }Ship;

/* 5 Ships for Player and Computer */
Ship ship[5];

/* Ships functions' definitions */
void Init_Ships();
char stype(int type);
int slength(char type);
void shiptype(char* shipname, int type);
void print_ships_hp(int line, int col, int hitpoints);
void print_ships();


         
