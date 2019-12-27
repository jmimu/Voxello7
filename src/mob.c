#include "mob.h"

#include <stdlib.h>
#include <stdio.h>
#include "dbg.h"

struct Mob* mob_create(struct Sprite* _spr, float _vx, float _vy, float _vz)
{
	struct Mob* mob=(struct Mob*)malloc(sizeof(struct Mob));
	mob->spr=_spr;
	mob->speed.x=_vx;
	mob->speed.y=_vy;
	mob->speed.z=_vz;
	mob->g.x=0;
	mob->g.y=0;
	mob->g.z=-1;
	mob->toDestroy=0;
	return mob;
}

void mob_draw(struct VoxRender * render, struct Mob* mob)
{
	sprite_draw(render,mob->spr);
}

void mob_update(struct VoxWorld * world, struct Mob* mob, double_t dt)
{
	mob->spr->pos.x+=mob->speed.x*dt;
	mob->spr->pos.y+=mob->speed.y*dt;
	mob->spr->pos.z+=mob->speed.z*dt;
	mob->speed.x+=mob->g.x*dt;
	mob->speed.y+=mob->g.y*dt;
	mob->speed.z+=mob->g.z*dt;
	
	if (mob->spr->pos.z<0)
	{
		mob->toDestroy=1;
		return;
	}
	
	//test collision TODO: use full sprite size, check intermediate positions (use raycasting?)
	if (((int)mob->spr->pos.x>=0)&&((int)mob->spr->pos.x<world->szX)&&
		((int)mob->spr->pos.y>=0)&&((int)mob->spr->pos.y<world->szY)&&
		((int)mob->spr->pos.z>=0)&&((int)mob->spr->pos.z<world->szZ))
	{
		voxworld_expand_col(world,mob->spr->pos.x,mob->spr->pos.y);
		if (world->curr_exp_col[(int)mob->spr->pos.z]!=EMPTY)
		{
			printf("Boum!\n");
			mob->toDestroy=1;
			int explode_size = 20;
			int x_min = mob->spr->pos.x - explode_size;
			if (x_min<0) x_min=0;
			int x_max = mob->spr->pos.x + explode_size;
			if (x_max>world->szX-1) x_max=world->szX-1;

			int y_min = mob->spr->pos.y - explode_size;
			if (y_min<0) y_min=0;
			int y_max = mob->spr->pos.y + explode_size;
			if (y_max>world->szY-1) y_max=world->szY-1;

			int z_min = mob->spr->pos.z - explode_size;
			if (z_min<0) z_min=0;
			int z_max = mob->spr->pos.z + explode_size;
			if (z_max>world->szZ-1) z_max=world->szZ-1;
			
			for (int x=x_min;x<=x_max;x++)
				for (int y=y_min;y<=y_max;y++)
				{
					voxworld_expand_col(world,x,y);
					for (int z=z_min;z<=z_max;z++)
					{
						if ((x-mob->spr->pos.x)*(x-mob->spr->pos.x)
							+(y-mob->spr->pos.y)*(y-mob->spr->pos.y)
							+(z-mob->spr->pos.z)*(z-mob->spr->pos.z)<explode_size*explode_size)
						{
							//TODO: add particle
							world->curr_exp_col[z]=EMPTY;
						}
					}
					voxworld_compr_col(world);
					voxworld_write_compr_col(world,x,y);
				}
		}
	}
}

