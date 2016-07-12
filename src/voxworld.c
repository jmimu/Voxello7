#include "voxworld.h"

#include <stdlib.h>
#include "dbg.h"
#include "trigo.h"

const uint8_t EMPTY=0xFF;
const int UNINIT=-1;



//TODO: add check & clear
struct VoxWorld * voxworld_create(int _szX,int _szY,int _szZ)
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
	for (int y=0;y<world->szY;y++)
	{
		world->data[y] = (struct RLE_block**)  malloc(world->szX*sizeof(struct RLE_block*));
		check_mem(world->data[y]);
		for (int x=0;x<world->szX;x++)
		{
			world->data[y][x] = (struct RLE_block*) malloc(nbr_RLE_block*sizeof(struct RLE_block));
			check_mem(world->data[y][x]);
			for (int z=0;z<nbr_RLE_block-1;z++)
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

	//init colormap
	world->colorMap[0]=0xFF000000;//black
	world->colorMap[1]=0xFFFFFFFF;//white
	world->colorMap[2]=0xFFFF0000;//red
	world->colorMap[3]=0xFF00FF00;//green
	world->colorMap[4]=0xFF0000FF;//blue
	world->colorMap[5]=0xFF00FFFF;//yellow
	world->colorMap[6]=0xFFFFFF00;//cyan
	world->colorMap[7]=0xFFFF00FF;//violet
	
	return world;
error:
	voxworld_delete(world);
	return NULL;
}

void voxworld_delete(struct VoxWorld * world)
{
	free(world->curr_exp_col);
	free(world->curr_compr_col);
	
	for (int y=0;y<world->szY;y++)
	{
		for (int x=0;x<world->szX;x++)
		{
			if (world->data[y][x]) free(world->data[y][x]);
		}
		if (world->data[y]) free(world->data[y]);
	}
	if (world->data) free(world->data);
	free(world);
}

void voxworld_printf(struct VoxWorld * world)
{
	int i,z;
	printf("====== VoxWorld ======\n");
	for (int y=0;y<world->szY;y++)
	{
		printf("---- y=%d\n",y);
		for (int x=0;x<world->szX;x++)
		{
			printf("  ---- x=%d\n	",x);
			z=0;
			i=0;
			while (z<world->szZ)
			{
				z+=world->data[y][x][i].n;
				printf("%d*[%d] ",world->data[y][x][i].n,world->data[y][x][i].v);
				i++;
			}
			printf("\n");
		}
	}
	printf("==== VoxWorld end ====\n");
}

void voxworld_empty_curr_exp_col(struct VoxWorld * world)
{
	memset(world->curr_exp_col,EMPTY,world->szZ);
}


//expand one compressed column into curr_exp_col
void voxworld_expand_col(struct VoxWorld * world,int x, int y)
{
	int i=0;
	int k=0;
	int z=0;
	struct RLE_block rle;
	
	while (z<world->szZ)
	{
		rle=world->data[y][x][i];
		for (k=0;k<rle.n;k++)
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
	world->curr_compr_col_size=0;
	int z=0;
	struct RLE_block rle={.n=0,.v=world->curr_exp_col[0]};
	for (z=0;z<world->szZ;z++)
	{
		if (rle.v==world->curr_exp_col[z])
		{
			rle.n++;
		}else{
			world->curr_compr_col[world->curr_compr_col_size]=rle;
			rle.n=1;
			rle.v=world->curr_exp_col[z];
			world->curr_compr_col_size++;
		}
	}
	world->curr_compr_col[world->curr_compr_col_size]=rle;
	world->curr_compr_col_size++;

#ifdef DBG_VOX
	printf("Compression results: ");
	for (int i=0;i<world->curr_compr_col_size; i++)
	{
		printf("%d*[%d] ",world->curr_compr_col[i].n,world->curr_compr_col[i].v);
	}
	printf("end\n");
#endif
}

//write curr_compr_col into data
bool voxworld_write_compr_col(struct VoxWorld * world,int x, int y)
{
	if (world->data[y][x]) free(world->data[y][x]);
	world->data[y][x] = (struct RLE_block*) malloc(
					world->curr_compr_col_size*sizeof(struct RLE_block));
	check_mem(world->data[y][x]);

	memcpy(world->data[y][x],world->curr_compr_col,
			world->curr_compr_col_size*sizeof(struct RLE_block));
#ifdef DBG_VOX
	printf("Copy results: ");
	for (int i=0;i<world->curr_compr_col_size; i++)
	{
		printf("%d*[%d] ",world->data[y][x][i].n,world->data[y][x][i].v);
	}
	printf("end\n");
#endif
	return true;
error:
	return false;
}

//empty cube : only edges are put to v
void voxworld_init_empty_cube(struct VoxWorld * world, uint8_t v)
{
	int x,y,z;
	
	//full col for corners
	for (z=0;z<world->szZ;z++)
		world->curr_exp_col[z]=v;
	voxworld_compr_col(world);
	voxworld_write_compr_col(world,0,0);
	voxworld_write_compr_col(world,world->szX-1,0);
	voxworld_write_compr_col(world,0,world->szY-1);
	voxworld_write_compr_col(world,world->szX-1,world->szY-1);
	
	//just first and last for vertical faces
	for (z=1;z<world->szZ-1;z++)
		world->curr_exp_col[z]=EMPTY;
	voxworld_compr_col(world);
	for (x=1;x<world->szX-1;x++)
	{
		voxworld_write_compr_col(world,x,0);
		voxworld_write_compr_col(world,x,world->szY-1);
	}
	for (y=1;y<world->szY-1;y++)
	{
		voxworld_write_compr_col(world,0,y);
		voxworld_write_compr_col(world,world->szX-1,y);
	}
	
	//empty for center
	world->curr_exp_col[0]=EMPTY;
	world->curr_exp_col[world->szZ-1]=EMPTY;
	voxworld_compr_col(world);
	for (x=1;x<world->szX-1;x++)
		for (y=1;y<world->szY-1;y++)
		{
			voxworld_write_compr_col(world,x,y);
		}

//tests
	for (z=0;z<world->szZ;z++)
		world->curr_exp_col[z]=v+1;
	voxworld_compr_col(world);
	voxworld_write_compr_col(world,world->szX/2,world->szY/2);
	for (z=1;z<world->szZ-1;z++)
		world->curr_exp_col[z]=EMPTY;
	voxworld_compr_col(world);
	for (x=1;x<world->szX-1;x++)
	{
		voxworld_write_compr_col(world,x,0);
	}

}


void voxworld_init_land(struct VoxWorld * world)
{
	long z_start;
	int SNOW_START=28;
	int SNOW_END=32;
	
	for (int i=0;i<255;i++)
	{
		world->colorMap[i]=(((i*34)%256)<<16)+(((i*34+85)%256)<<8)+(i*34+190)%256;
	}
	
	for (long x=0;x<world->szX;x++)
		for (long y=0;y<world->szY;y++)
		{
			z_start=_cos(x/(world->szX/4.0))*world->szZ/10+_sin(y/(world->szY/5.2)+1)*world->szZ/8+world->szZ/7;
			if (z_start<=0) z_start=1; 
			voxworld_empty_curr_exp_col(world);
			for (long z=0;z<world->szZ/2;z++)
			{
				if (z<z_start)
					world->curr_exp_col[z]=z_start/2;//0x50;
				else if (z<z_start+1)//for snow
					world->curr_exp_col[z]=rand()%(SNOW_END-SNOW_START)+SNOW_START;//0x50;
				else
					world->curr_exp_col[z]=EMPTY;
			}
			voxworld_compr_col(world);
			voxworld_write_compr_col(world,x,y);
		}
	//add a wall
	
	//colors: 20 and 150
	//structure:
	//   ***************
	//   *   *   *   *  
	//   *   *   *   *  
	//   ***************
	//	 *   *   *   *
	//	 *   *   *   *
	/*unsigned char v=0;
	for (long x=65;x<68;x++)
	{
		for (long y=6;y<30;y++)
		{
			voxworld_empty_curr_exp_col(world);
			for (long z=0;z<world->szZ-10;z++)
			{
				v=150;
				if (z%3==0) v=20;
				if ((y+3*((z/3)%2))%5==0) v=20;
				world->curr_exp_col[z]=v;
			}
			voxworld_compr_col(world);
			voxworld_write_compr_col(world,x,y);
		}
	}
	
	//other
	v=0;
	for (long x=100;x<102;x++)
	{
		for (long y=80;y<82;y++)
		{
			voxworld_empty_curr_exp_col(world);
			for (long z=0;z<world->szZ;z++)
			{
				v=201;
				if (z%2==0) v=78;
				world->curr_exp_col[z]=v;
			}
			voxworld_compr_col(world);
			voxworld_write_compr_col(world,x,y);
		}
	}*/
}
