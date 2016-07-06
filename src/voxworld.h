#ifndef VOXWORLD_H
#define VOXWORLD_H

#include <stdint.h>

const uint8_t EMPTY=0xFF;


/*******
 * Voxels are saved as RLE pairs : (number,value)
 * data[y][x] is a RLE_block*
 *******/

#define DBG_VOX

struct RLE_block
{
	uint8_t n; 
	uint8_t v; 
};

struct VoxWorld
{
    long szX,szY,szZ;
    struct RLE_block *** data;
    uint32_t colorMap[255];//argb
    
    //working columns, allocated 1 time
    uint8_t * curr_exp_col;//size=szZ
    struct RLE_block *curr_compr_col;//size=szZ
    long curr_compr_col_size;//number of RLE_block
};

struct VoxWorld * voxworld_create(long _szX,long _szY,long _szZ);
void voxworld_delete(struct VoxWorld * world);
void voxworld_expand_col(struct VoxWorld * world,long x, long y);
void voxworld_compr_col(struct VoxWorld * world);
bool voxworld_write_compr_col(struct VoxWorld * world,long x, long y);
void voxworld_init_empty_cube(struct VoxWorld * world, uint8_t v);
#endif // VOXWORLD_H
