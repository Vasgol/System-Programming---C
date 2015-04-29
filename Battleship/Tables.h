/************************** Definitions Of tables *******************************/


#define     READ    0    /* Defining Pipe read end */ 
#define     WRITE   1    /* Defining Pipe write end */
  
#define     SIZE    10   /* Defining tables' SIZE */


#define     EMPTY   -1   /* Empty position in table */
#define     HIT      1   /* Position marked as Hit in table */
#define     NOT_HIT  0   /* Position marked as NOT Hit in table */


/* Player = Human or Computer */
int table[SIZE][SIZE]; 		/* Player's table */
int rtable[SIZE][SIZE];         /* Player's Rival table */


void Init_Tables();          /* Initialization of tables */

/* Just functions used for testing algorithm tables for each process (Computer-Human) */
void print_comp_table();  
void print_user_table();

 
