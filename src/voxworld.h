#ifndef VOXWORLD_H
#define VOXWORLD_H

#include <stdint.h>
#include <stdbool.h>

extern const uint8_t EMPTY;
extern const int UNINIT;


/*******
 * Voxels are saved as RLE pairs : (number,value)
 * data[y][x] is a RLE_block*
 *******/

//#define DBG_VOX

struct RLE_block
{
	uint8_t n; 
	uint8_t v; 
};

struct VoxWorld
{
	int szX,szY,szZ;
	struct RLE_block *** data;
	unsigned short ** col_size;//in bytes
	uint32_t colorMap[255];//argb
	
	//working columns, allocated 1 time
	uint8_t * curr_exp_col;//size=szZ
	struct RLE_block *curr_compr_col;//size=szZ
	int curr_compr_col_size;//number of RLE_block
};

struct VoxWorld * voxworld_create(int _szX,int _szY,int _szZ);
void voxworld_delete(struct VoxWorld * world);
void voxworld_empty_curr_exp_col(struct VoxWorld * world);
void voxworld_expand_col(struct VoxWorld * world,int x, int y);
void voxworld_compr_col(struct VoxWorld * world);
bool voxworld_write_compr_col(struct VoxWorld * world,int x, int y);
void voxworld_init_empty_cube(struct VoxWorld * world, uint8_t v);
void voxworld_init_land(struct VoxWorld * world);
void voxworld_init_stairs(struct VoxWorld * world);

void voxworld_printf(struct VoxWorld * world);
unsigned long voxworld_getsize(struct VoxWorld * world);
struct VoxWorld * voxworld_copy(struct VoxWorld * world);

#endif // VOXWORLD_H
