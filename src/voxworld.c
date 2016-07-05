#include "voxworld.h"

#include <stdlib.h>
#include "dbg.h"

//TODO: add check & clear
struct VoxWorld * voxworld_create(long _szX,long _szY,long _szZ)
{
	struct VoxWorld *world = (struct VoxWorld *) malloc(sizeof(struct VoxWorld));
	world->szX=_szX;
	world->szY=_szY;
	world->szZ=_szZ;
	
	//to create an empty world of height world->szZ, we need
	int nbr_RLE_block=1+world->szZ/255;
	int last_RLE_block_size=world->szZ%255;
	
	//data[y][x] is a RLE_block
	world->data = (struct RLE_block***) malloc(world->szY*sizeof(struct RLE_block**));
	check_mem(world->data);
	for (long y=0;y<world->szY;y++)
	{
		world->data[y] = (struct RLE_block**)  malloc(world->szX*sizeof(struct RLE_block*));
		check_mem(world->data[y]);
		for (long x=0;x<world->szX;x++)
		{
			world->data[y][x] = (struct RLE_block*) malloc(nbr_RLE_block*sizeof(struct RLE_block));
			check_mem(world->data[y][x]);
			for (long z=0;z<nbr_RLE_block-1;z++)
			{
				world->data[y][x][z]=(struct RLE_block){.n=255,.v=EMPTY};
			}
			world->data[y][x][nbr_RLE_block-1]=(struct RLE_block){.n=last_RLE_block_size,.v=EMPTY};
		}
	}
	
	//create working colums memory space
	world->curr_exp_col=(uint8_t *) malloc(world->szZ*sizeof(uint8_t));
	world->curr_compr_col=(struct RLE_block *) malloc(world->szZ*sizeof(struct RLE_block));
	world->curr_compr_col_size=0;
	
	return world;
error:
	voxworld_delete(world);
	return NULL;
}

void voxworld_delete(struct VoxWorld * world)
{
	free(world->curr_exp_col);
	free(world->curr_compr_col);
	
	for (long y=0;y<world->szY;y++)
	{
		for (long x=0;x<world->szX;x++)
		{
			if (world->data[y][x]) free(world->data[y][x]);
		}
		if (world->data[y]) free(world->data[y]);
	}
	if (world->data) free(world->data);
	free(world);
}

//expand one compressed column into curr_exp_col
void voxworld_expand_col(struct VoxWorld * world,long x, long y)
{
	int i=0;
	int k=0;
	int z=0;
	RLE_block rle;
	
	while (z<world->szZ)
	{
		rle=world->data[y][x][i];
		for (int k=0;k<rle.n;k++)
		{
			world->curr_exp_col[z]=rle.v;
			z++;
		}
		i++;
	}

#ifdef DBG_VOX
	printf("Decompression results: ");
	for (int z=0;z<world->szZ; z++)
	{
		printf("%d ",world->curr_exp_col[z]);
	}
	printf("end\n");
#endif
}

//compress curr_exp_col into curr_compr_col
void voxworld_compr_col(struct VoxWorld * world)
{
	int curr_compr_col_size=0;
	int z=0;
	RLE_block rle={.n=0,.v=world->curr_exp_col[0]};
	for (z=0;z<world->szZ;z++)
	{
		if (rle.v==world->curr_exp_col[z])
		{
			rle.n++;
		}else{
			world->curr_compr_col[curr_compr_col_size]=rle;
			rle.n=1;
			rle.v=world->curr_exp_col[z];
			curr_compr_col_size++;
		}
	}
	world->curr_compr_col[curr_compr_col_size]=rle;
	curr_compr_col_size++;

#ifdef DBG_VOX
	printf("Compression results: ");
	for (int i=0;i<curr_compr_col_size; i++)
	{
		printf("(%d %d) ",world->curr_compr_col[i].n,world->curr_compr_col[i].v);
	}
	printf("end\n");
#endif
}

//write curr_compr_col into data
bool voxworld_write_compr_col(struct VoxWorld * world,long x, long y)
{
	if (world->data[y][x]) free(world->data[y][x]);
	world->data[y][x] = (struct RLE_block*) malloc(
					world->curr_compr_col_size*sizeof(struct RLE_block));
	check_mem(world->data[y][x]);

	memcpy(world->data[y][x],world->curr_compr_col,
			world->curr_compr_col_size*sizeof(struct RLE_block));

	return true;
error:
	return false;
}
