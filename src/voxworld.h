#ifndef VOXWORLD_H
#define VOXWORLD_H

#include <stdint.h>
#include <stdbool.h>

#define VOX_TYPE uint16_t
extern const VOX_TYPE EMPTY;
extern const VOX_TYPE SPECIAL_MASK;
extern const VOX_TYPE WATER;
extern const int UNINIT;


/*******
 * Voxels are saved as RLE pairs : (number,value)
 * data[y][x] is a RLE_block*
 *******/

//#define DBG_VOX

#define MAX_RLE_N 65000 //255

struct RLE_block
{
	uint16_t n; 
    VOX_TYPE v;
};

struct VoxWorld
{
	int szX,szY,szZ;
    struct RLE_block *** data;
	unsigned short ** col_size;//in bytes
	unsigned short ** col_full_start;//z of first full voxel (szZ if col empty)
	unsigned short ** col_full_end;//z of last full voxel +1 (0 if col empty)
	
	//working columns, allocated 1 time
    VOX_TYPE * curr_exp_col;//size=szZ
	struct RLE_block *curr_compr_col;//size=szZ
	int curr_compr_col_size;//number of RLE_block
	unsigned short curr_col_full_start;
	unsigned short curr_col_full_end;
};

uint32_t color_15to24(VOX_TYPE v);
VOX_TYPE color_24to15(uint32_t c);

struct VoxWorld * voxworld_create(int _szX,int _szY,int _szZ);
void voxworld_delete(struct VoxWorld * world);
void voxworld_empty_curr_exp_col(struct VoxWorld * world);
void voxworld_expand_col(struct VoxWorld * world,int x, int y);
void voxworld_compr_col(struct VoxWorld * world);
bool voxworld_write_compr_col(struct VoxWorld * world,int x, int y);
int voxworld_get_ground_z(struct VoxWorld * world, int x, int y);

void voxworld_init_empty_cube(struct VoxWorld * world, VOX_TYPE v);
void voxworld_init_full_cube(struct VoxWorld * world);
void voxworld_init_land(struct VoxWorld * world);
void voxworld_init_land2(struct VoxWorld * world);
void voxworld_init_stairs(struct VoxWorld * world);
void voxworld_init_cave(struct VoxWorld * world);
void voxworld_init_rand(struct VoxWorld * world);
void voxworld_init_Menger(struct VoxWorld * world);

void voxworld_printf(struct VoxWorld * world);
void voxworld_print_col(struct VoxWorld * world,int x, int y);
unsigned long voxworld_getsize(struct VoxWorld * world);
struct VoxWorld * voxworld_copy(struct VoxWorld * world);

#endif // VOXWORLD_H
