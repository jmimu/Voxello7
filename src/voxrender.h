#ifndef VOXRENDER_H
#define VOXRENDER_H

#include "pt3d.h"
#include "voxworld.h"
#include <stdbool.h>

/*****************
 * One ray of raycasting
 * = one vertical plane
 * = one screen column
 * ***************/


//VoxVInterval are arranged in growing order
//interval between line min and line max
struct VoxVInterval
{
	int l_min;
	int l_max;
	struct VoxVInterval * next;
	struct VoxVInterval * previous;
};

struct VoxRay
{
	struct VoxRender *render;
	struct VoxWorld *world;
	struct Pt3d * cam;
	double incX,incY;//how much to increment for 1 step in x or y
	double currentLambda;//where we are on the ray
	double nextXLambda;//next X intersection
	double nextYLambda;//next Y intersection

	int currentX,currentY;
	bool lastIntersectionWasX;

	int dirX,dirY;//-1, 0 or 1

	struct VoxVInterval *first_VInterval;
};


/*
 * Projection on a plan, nodal point = cam,
 * focal=f, plan size=graph render size
 * */
struct VoxRender
{	
	struct VoxWorld *world;
	struct Pt3d cam;
	double ang_hz;
	double ang_hz_cos,ang_hz_sin;
	struct VoxRay ray;

	double clip_min;//min dist for intersection
	double clip_dark;//startx dist for darkness
	double clip_max;//max dist for intersection

	double f;//focal (in pixels)
	double * fc;//distance from screen column to nodal point
	
};



void voxray_reinit(struct VoxRay * ray,struct Pt3d *cam, int c, bool trace);
double voxray_lambdaNextIntersection(struct VoxRay * ray);
bool voxray_findNextIntersection(struct VoxRay * ray,bool trace);//returns false if out of bounds
void voxray_draw(struct VoxRay * ray,int c,bool trace);
void Voxray_show_info(struct VoxRay * ray);


struct VoxRender * voxrender_create(struct VoxWorld *_world,double f_eq35mm);
void voxrender_setCam(struct VoxRender * render,struct Pt3d _cam,double _ang_hz);
void voxrender_render(struct VoxRender * render,bool trace);
void voxrender_delete(struct VoxRender * render);

#endif // VOXRENDER_H
