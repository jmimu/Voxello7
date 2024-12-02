#include "sprite.h"

#include <stdio.h>
#include "dbg.h"
#include "graph.h"


struct Sprite* sprite_create(const char* _name,
	float _x, float _y, float _z, float _w, float _h, struct Anim* _anim)
{
	struct Sprite* spr=(struct Sprite*)malloc(sizeof(struct Sprite));
	strncpy(spr->name,_name,MAXNAMELEN);
	spr->name[MAXNAMELEN-1]=0;
	spr->pos.x=_x;
	spr->pos.y=_y;
	spr->pos.z=_z;
	spr->real_w=_w;
	spr->real_h=_h;
	spr->anim=_anim;
	return spr;
}

void sprite_draw(struct VoxRender * render,struct Sprite* spr, uint32_t normale)
{
	float color_factor=1;
	struct Raster* raster=anim_get_raster(spr->anim);
	//projection of origin of sprite, y=dist perp to cam
	struct Pt3d proj=voxrender_proj(render,spr->pos);
	if (proj.y>render->clip_max) return;
	if (proj.y>render->clip_dark)
		color_factor=1-(proj.y-render->clip_dark)/(render->clip_max-render->clip_dark);
	/*if ((proj.x<-(raster->w>>2))||(proj.x-(raster->w>>2)>graph.render_w)||
	    (proj.y<-(raster->h>>2))||(proj.y-(raster->h>>2)>graph.render_h))
		return;*/
	int show_w=render->f*spr->real_w/proj.y;
	int show_h=render->f*spr->real_h/proj.y;
	//raster_draw(spr->raster,proj.x-(spr->raster->w>>2),proj.z-(spr->raster->h),proj.y*ZBUF_FACTOR);
	raster_draw_zoom(raster,proj.x-(show_w>>1),
		proj.z-show_h,proj.y*ZBUF_FACTOR,show_w,show_h,color_factor, normale);
}

