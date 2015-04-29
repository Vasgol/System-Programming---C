/*********** Computer Player Functions' Definitions used by the child process of the battleship program *****************/
#include "Ships.h"
#include "Tables.h"

int Computer_Player(int *p1, int *p2);
void  generate_random_hit(int* hitline, int* hitcol);
void  hit_next_point(int* hitline, int *hitcol, int phitline, int phitcol);
int computer_ships();
int check_values(int *bow,int *stern,Ship ship);

