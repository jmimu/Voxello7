#ifndef SPRITE_H
#define SPRITE_H

#define MAXNAMELEN 30

#include "raster.h"
#include "pt3d.h"
#include "voxrender.h"
/***
	A sprite is an object in 3d world with a raster (later, animations)
	a real w and real h, a 3d position (all in voxel unit)
*/

struct Sprite{
	float real_w,real_h;
	struct Pt3d pos;
	struct Anim* anim;
	char name[MAXNAMELEN];
};


struct Sprite* sprite_create(const char* _name,
	float _x, float _y, float _z, float _w, float _h, struct Anim* _anim);
void sprite_draw(struct VoxRender * render, struct Sprite* spr, uint32_t normale);


#endif // SPRITE_H
