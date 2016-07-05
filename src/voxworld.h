#ifndef VOXWORLD_H
#define VOXWORLD_H

#include <stdint.h>

#define EMPTY 0xFF


/*******
 * Voxels are saved as RLE pairs : (number,value)
 *
 *******/

struct RLE_block
{
	unsigned char n; 
	unsigned char v; 
};

struct VoxWorld
{
    long szX,szY,szZ,hz_Size;
    struct RLE_block *** data;
    uint32_t mColorMap[256];//argb
};

struct VoxWorld * voxworld_create(long _SzX,long _SzY,long _SzZ);
void voxworld_delete(struct VoxWorld * world);


#endif // VOXWORLD_H
