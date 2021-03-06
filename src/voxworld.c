#include "voxworld.h"

#include <stdlib.h>
#include <math.h>
#include "dbg.h"
#include "trigo.h"

const VOX_TYPE EMPTY=0xffff;
const int UNINIT=-1;



//TODO: add check & clear
struct VoxWorld * voxworld_create(int _szX,int _szY,int _szZ)
{
	printf("Creating a %.1e voxels world...\n",((double)_szX)*_szY*_szZ);
	struct VoxWorld *world = (struct VoxWorld *) malloc(sizeof(struct VoxWorld));
	world->szX=_szX;
	world->szY=_szY;
	world->szZ=_szZ;
	
	//to create an empty world of height world->szZ, we need
	int nbr_RLE_block=1+world->szZ/MAX_RLE_N;
	int last_RLE_block_size=world->szZ%MAX_RLE_N;
	
	//data[y][x] is a RLE_block
	world->data = (struct RLE_block***) malloc(world->szY*sizeof(struct RLE_block**));
	check_mem(world->data);
	world->col_size = (unsigned short **) malloc(world->szY*sizeof(unsigned short*));
	check_mem(world->col_size);
	world->col_full_start = (unsigned short **) malloc(world->szY*sizeof(unsigned short*));
	check_mem(world->col_full_start);
	world->col_full_end = (unsigned short **) malloc(world->szY*sizeof(unsigned short*));
	check_mem(world->col_full_end);
	for (int y=0;y<world->szY;y++)
	{
		world->data[y] = (struct RLE_block**)  malloc(world->szX*sizeof(struct RLE_block*));
		check_mem(world->data[y]);
		world->col_size[y] = (unsigned short*)  malloc(world->szX*sizeof(unsigned short));
		check_mem(world->col_size[y]);
		world->col_full_start[y] = (unsigned short*)  malloc(world->szX*sizeof(unsigned short));
		check_mem(world->col_full_start[y]);
		world->col_full_end[y] = (unsigned short*)  malloc(world->szX*sizeof(unsigned short));
		check_mem(world->col_full_end[y]);
		for (int x=0;x<world->szX;x++)
		{
			world->data[y][x] = (struct RLE_block*) malloc(nbr_RLE_block*sizeof(struct RLE_block));
			check_mem(world->data[y][x]);
			world->col_size[y][x] = nbr_RLE_block*sizeof(struct RLE_block);
			world->col_full_start[y][x] = world->szZ;
			world->col_full_end[y][x] = 0;
			for (int z=0;z<nbr_RLE_block-1;z++)
			{
				world->data[y][x][z]=(struct RLE_block){.n=MAX_RLE_N,.v=EMPTY};
			}
			world->data[y][x][nbr_RLE_block-1]=(struct RLE_block){.n=last_RLE_block_size,.v=EMPTY};
		}
	}

	//create working colums memory space
    world->curr_exp_col=(VOX_TYPE *) malloc(world->szZ*sizeof(VOX_TYPE));
	voxworld_empty_curr_exp_col(world);
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
	
	for (int y=0;y<world->szY;y++)
	{
		if (world->data[y])
			for (int x=0;x<world->szX;x++)
				if (world->data[y][x]) free(world->data[y][x]);
		if (world->data[y]) free(world->data[y]);
		if (world->col_size[y]) free(world->col_size[y]);
		if (world->col_full_start[y]) free(world->col_full_start[y]);
		if (world->col_full_end[y]) free(world->col_full_end[y]);
	}
	if (world->data) free(world->data);
	if (world->col_size) free(world->col_size);
	if (world->col_full_start) free(world->col_full_start);
	if (world->col_full_end) free(world->col_full_end);
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

void voxworld_print_col(struct VoxWorld * world,int x, int y)
{
	int i=0;
	int z=0;
	struct RLE_block rle;
	
	printf("Col %d %d: ",x,y);
	while (z<world->szZ)
	{
		rle=world->data[y][x][i];
		printf("@%d: %d, ",z,rle.v);
		z+=rle.n;
		i++;
	}
	printf("end.\n");
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
	world->curr_col_full_start=world->szZ;
	world->curr_col_full_end=0;
	for (z=0;z<world->szZ;z++)
	{
		if ((z<world->curr_col_full_start)&&(world->curr_exp_col[z]!=EMPTY))
			world->curr_col_full_start=z;
		if ((world->curr_col_full_end<=z)&&(world->curr_exp_col[z]!=EMPTY))
			world->curr_col_full_end=z+1;
		if ((rle.v==world->curr_exp_col[z])&&(rle.n<MAX_RLE_N))
		{
			rle.n++;
		}else{
			//record rle
			world->curr_compr_col[world->curr_compr_col_size]=rle;
			world->curr_compr_col_size++;
			//new rle
			rle.n=1;
			rle.v=world->curr_exp_col[z];
		}
	}
	//record last rle
	world->curr_compr_col[world->curr_compr_col_size]=rle;
	world->curr_compr_col_size++;

#ifdef DBG_VOX
	printf("Compression results: ");
	for (int i=0;i<world->curr_compr_col_size; i++)
	{
		printf("%d*[%d] ",world->curr_compr_col[i].n,world->curr_compr_col[i].v);
	}
	printf("end\n");
	printf("col_full_start: %d, col_full_end: %d\n",world->curr_col_full_start,world->curr_col_full_end);
#endif
}

//write curr_compr_col into data
bool voxworld_write_compr_col(struct VoxWorld * world,int x, int y)
{
	if (world->data[y][x]) free(world->data[y][x]);
	world->data[y][x] = (struct RLE_block*) malloc(
					world->curr_compr_col_size*sizeof(struct RLE_block));
	check_mem(world->data[y][x]);

	world->col_size[y][x] = world->curr_compr_col_size*sizeof(struct RLE_block);
	world->col_full_start[y][x] = world->curr_col_full_start;
	world->col_full_end[y][x] = world->curr_col_full_end;

	memcpy(world->data[y][x],world->curr_compr_col,
			world->curr_compr_col_size*sizeof(struct RLE_block)); //TODO: why use curr_compr_col?
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

int voxworld_get_ground_z(struct VoxWorld * world, int x, int y)
{
    if ((x>=0)&&(x<world->szX)&&(y>=0)&&(y<world->szY))
        return world->col_full_end[x][y];
    else
        return 0;
}


//empty cube : only edges are put to v
void voxworld_init_empty_cube(struct VoxWorld * world, uint16_t v)
{
	int x,y,z;
	
	printf("Filling world...\n");

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

void voxworld_init_full_cube(struct VoxWorld * world)
{
	int x,y,z;
	printf("Filling world...\n");
	voxworld_empty_curr_exp_col(world);
	world->curr_exp_col[0]=10;
	world->curr_exp_col[1]=10;
	world->curr_exp_col[world->szZ-1]=20;
	world->curr_exp_col[world->szZ-2]=20;
	voxworld_compr_col(world);
	for (x=0;x<world->szX;x++)
		for (y=0;y<world->szY;y++)
		{
			voxworld_write_compr_col(world,x,y);
		}

	world->curr_exp_col[1]=EMPTY;
	world->curr_exp_col[world->szZ-2]=EMPTY;
	for (z=3;z<world->szZ-3;z++)
		world->curr_exp_col[z]=z;
	voxworld_compr_col(world);
	for (x=1;x<world->szX-1;x++)
		for (y=1;y<world->szY-1;y++)
		{
			voxworld_write_compr_col(world,x,y);
		}

}

void voxworld_init_stairs(struct VoxWorld * world)
{
	int x,y,z;
	printf("Filling world...\n");
	
	for (x=0;x<world->szX;x++)
		for (y=0;y<world->szY;y++)
		{
			for (z=0;z<world->szZ;z++)
			{
				if (z<=x+y)
					world->curr_exp_col[z]=x*2+y+1;
				else
					world->curr_exp_col[z]=EMPTY;
			}
			voxworld_compr_col(world);
			voxworld_write_compr_col(world,x,y);
		}

}

void voxworld_init_cave(struct VoxWorld * world)
{
	int x,y,z;
	int yc,zc,r;
	printf("Filling world...\n");
	
	yc=world->szY/2;
	zc=world->szZ/3;
	r=world->szZ/10;
	
	for (x=0;x<world->szX;x++)
	{
		r=x/100+world->szZ/10;
		zc=x/100+world->szZ/4;
		yc=x/200+world->szY/2;
		for (y=0;y<world->szY;y++)
		{
			for (z=0;z<world->szZ;z++)
			{
				if ((y-yc)*(y-yc)+(z-zc)*(z-zc)>r*r)
					world->curr_exp_col[z]=x*2+y+1;
				else
					world->curr_exp_col[z]=EMPTY;
			}
			voxworld_compr_col(world);
			voxworld_write_compr_col(world,x,y);
		}
	}
}

void voxworld_init_land2(struct VoxWorld * world)
{
	int x,y,z,h,hi,i,c;
	double l;
	printf("Filling world...\n");
	
	const int nb_sinc=30;
	int sinc_x[nb_sinc];
	int sinc_y[nb_sinc];
	int sinc_h[nb_sinc];
	double sinc_sx[nb_sinc];
	double sinc_sy[nb_sinc];
	int max_z =150;
	for (i=0;i<nb_sinc;i++)
	{
		sinc_x[i]=rand()%world->szX;
		sinc_y[i]=rand()%world->szY;
		sinc_h[i]=rand()%(max_z*2/3)+(max_z/3);
		sinc_sx[i]=rand()%100+40;
		sinc_sy[i]=rand()%100+40;
	}
	
	for (x=0;x<world->szX;x++)
	{
		for (y=0;y<world->szY;y++)
		{
			h=0;
			for (i=0;i<nb_sinc;i++)
			{
				l= sqrt(((x-sinc_x[i])/sinc_sx[i])*((x-sinc_x[i])/sinc_sx[i]) + ((y-sinc_y[i])/sinc_sy[i])*((y-sinc_y[i])/sinc_sy[i]));
				hi=_sin( l ) / (l+0.001)  * sinc_h[i];
				if (h<hi) h = hi;
			}
			h += rand()%2 ;
			c = (255.0*h)/max_z + (rand()%(h/10+1))-h/20;
			for (z=0;z<world->szZ;z++)
			{
				if (z<=h)
					//world->curr_exp_col[z]=world->colorMap[(int)(h*255.0/world->szZ)];
					world->curr_exp_col[z]=c;
				else
					world->curr_exp_col[z]=EMPTY;
			}
			voxworld_compr_col(world);
			#pragma omp critical
			{
				voxworld_write_compr_col(world,x,y);
			}
		}
	}

	printf("Filling world done.\n");
}

/*bool isBadMenger(int v)//check if 1 in base 3
{
	while (v>0)
	{
		if (v%3==1)
			return true;
		v/=3;
	}
	return false;
}*/

bool isBadMenger(int v1, int v2, int v3) //check if 1 in base 3 on the same digits
{
	while ((v1>0)||(v2>0)||(v3>0))
	{
		if ((v1%3==1)&&(v2%3==1))
			return true;
		if ((v1%3==1)&&(v3%3==1))
			return true;
		if ((v2%3==1)&&(v3%3==1))
			return true;
		v1/=3;
		v2/=3;
		v3/=3;
	}
	return false;
}

void voxworld_init_Menger(struct VoxWorld * world)
{
	int x,y,z;
	printf("Filling world...\n");
	for (x=0;x<world->szX;x++)
	{
		for (y=0;y<world->szY;y++)
		{
			for (z=0;z<world->szZ;z++)
			{
				if (isBadMenger(x,y,z))
					world->curr_exp_col[z]=EMPTY;
				else
					world->curr_exp_col[z]=2;
			}
			voxworld_compr_col(world);
			voxworld_write_compr_col(world,x,y);
		}
	}
	printf("Filling world done.\n");
}


void voxworld_init_rand(struct VoxWorld * world)
{
	int x,y,z,ref;
	printf("Filling world...\n");

	for (x=0;x<world->szX;x++)
		for (y=0;y<world->szY;y++)
		{
			for (z=0;z<world->szZ;z++)
			{
				ref=(z-world->szZ/2)*(z-world->szZ/2);
				//printf("%d ",ref);
				if (rand()%5000<ref)
					world->curr_exp_col[z]=rand()%256;
				else
					world->curr_exp_col[z]=EMPTY;
			}
			voxworld_compr_col(world);
			voxworld_write_compr_col(world,x,y);
		}

}

void voxworld_init_land(struct VoxWorld * world)
{
	long z_start;
	int SNOW_START=28;
	int SNOW_END=32;
	printf("Filling world...\n");
		
	for (long x=0;x<world->szX;x++)
		for (long y=0;y<world->szY;y++)
		{
			z_start=_cos(x/(world->szX/(PI*3)+1))*world->szZ/4+_sin(y/(world->szY/(PI*5)+1)+1)*world->szZ/5+world->szZ/2;
			z_start/=5;
			//z_start=x+y-1;
			if (z_start<=0) z_start=1; 
			voxworld_empty_curr_exp_col(world);
			for (long z=0;z<world->szZ;z++)
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
	if ((world->szX>110)&&(world->szY>90)&&(world->szZ>10))
	{
		unsigned char v=0;
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
		}
	}
}



unsigned long voxworld_getsize(struct VoxWorld * world)
{
	unsigned long size=0;
	for (long x=0;x<world->szX;x++)
		for (long y=0;y<world->szY;y++)
			size+=world->col_size[y][x];
	return size;
}

struct VoxWorld * voxworld_copy(struct VoxWorld * world)
{
	printf("Copying world...\n");
	struct VoxWorld *new_world = voxworld_create(world->szX,world->szY,world->szZ);
		
	for (long x=0;x<world->szX;x++)
		for (long y=0;y<world->szY;y++)
		{
			if (new_world->data[y][x]) free(new_world->data[y][x]);
			new_world->col_size[y][x] = world->col_size[y][x];
			new_world->col_full_start[y][x] = world->col_full_start[y][x];
			new_world->col_full_end[y][x] = world->col_full_end[y][x];
			new_world->data[y][x] = (struct RLE_block*) malloc(new_world->col_size[y][x]);
			check_mem(new_world->data[y][x]);
			memcpy(new_world->data[y][x],world->data[y][x],new_world->col_size[y][x]);
		}

	return new_world;
error:
	voxworld_delete(new_world);
	return NULL;
}

uint32_t color_15to24(VOX_TYPE v)
{
    int r=((v>>10)&0x1F)*0xff/0x1f;
    int g=((v>>5)&0x1F)*0xff/0x1f;
    int b=((v>>0)&0x1F)*0xff/0x1f;
    int a=0xff;
    return (a<<24)+(r<<16)+(g<<8)+b;
}

VOX_TYPE color_24to15(uint32_t c)
{
    int r=((c>>19)&0x1F);
    int g=((c>>11)&0x1F);
    int b=((c>> 3)&0x1F);
    return (r<<10)+(g<<5)+b;
}
