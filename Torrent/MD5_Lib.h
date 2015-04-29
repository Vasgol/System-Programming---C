#ifndef _MD5_H
#define _MD5_H

int MD5Sum_Chunk(char* data, int length, char* MD5Sum);
int MD5Sum_File(char* filename, char* MD5Sum);

#endif
