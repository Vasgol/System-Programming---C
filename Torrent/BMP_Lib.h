#ifndef PEER_BITMAP_H
#define PEER_BITMAP_H

/*
   Function allocating a bitmap of "length" bits
   and initializing them to zero. Bitmap should
   be passed by reference. If an error occurs
   return -1, upon successful execution 0
   is returned.
*/

int BMP_Init(unsigned char** bmp, unsigned int length);

/*
   Function used to release space allocated
   into heap from BMP_Init.
*/
void BMP_Release(unsigned char* bmp);

/*
   Function setting bitmap's bit "which" to 1.
   Upon successful execution 0 is returned.
*/
int BMP_Set(unsigned char* bmp, unsigned int which);

/*
   Function setting bitmap's bit "which" to 0.
   Upon successful execution 0 is returned.
*/
int BMP_Unset(unsigned char* bmp, unsigned int which);

/* Check whether the corresponding bit is set.
 * Returns:
 * 1 ) 0 if the bit is un-set
 * 2 ) 1 if the bit is set */
int BMP_Is_Set(unsigned char* bmp, unsigned int which);

/*
   Function returning the position of a random zero (invalid)
   bit of bitmap. Random means that zero bits are
   not returned sequentially.
*/

int BMP_FindEmpty(unsigned char* bmp, unsigned int length);

#endif
