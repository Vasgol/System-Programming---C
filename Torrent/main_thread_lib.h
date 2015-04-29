/***************************************** Main Thread Library Functions' Definitions ******************************************************/
#include <stdio.h>
#include "thread_lib.h"                     /* Data structures */

int Get_Conf_Data(Peer *p, char* buf1, char* buf2);
int Parse_Conf_File(FILE *fp, Peer *p);
long GetFileSize(FILE *fp);
int Tracker_Request(Peer *p, int tsock);
int Tracker_Response(Peer *p, int tsock);
int Parse_Response_Body(int sock, char* body);
int Parse_Response_Line(int sock, char* description, char* value);
int Join_Response(Peer *p, int sock);


/* Help Functions */
int strcmpin(char *string1, char *string2);


