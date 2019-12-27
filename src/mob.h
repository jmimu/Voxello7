#ifndef MOB_H
#define MOB_H

#include "sprite.h"
#include "pt3d.h"
#include "voxworld.h"

/***
	A mob is an object in 3d world with a sprite, a momentum and collisions
*/

struct Mob{
	struct Sprite * spr;
	struct Pt3d speed;
	struct Pt3d g;
	int toDestroy;//1=true
};


struct Mob* mob_create(struct Sprite* _spr, float _vx, float _vy, float _vz);
void mob_draw(struct VoxRender * render, struct Mob* mob);
void mob_update(struct VoxWorld * world, struct Mob* mob, double_t dt);


#endif // MOB_H
