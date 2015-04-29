/*********** Human Player Functions' Definitions used by the child process of the battleship program *****************/
#include "Ships.h"
#include "Tables.h"

int place_ship(char* bowpoint, char* sternpoint, Ship ship);
int Human_Player(int* p1, int* p2);
void hitresult(char* message, char result);
void help_rules();
void delete_mes();

