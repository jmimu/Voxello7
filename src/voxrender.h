#ifndef VOXRENDER_H
#define VOXRENDER_H

#include "pt3d.h"
#include "voxworld.h"
#include <stdbool.h>

#define FACTOR_DARK 0.6
#define FACTOR_BRIGHT 1.5

/*****************
 * One ray of raycasting
 * = one vertical plane
 * = one screen column
 * ***************/


//VoxVInterval are arranged in growing order
//interval between line min and line max
struct VoxVInterval
{
	short l_min;
	short l_max;
};

struct VoxRay
{
	int thread;
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

	//the list of intervals has a fixed size
	//ray swaps between VIntervals_A/B for current/next_VIntervals
	struct VoxVInterval **current_VIntervals;//pointers to VIntervals_A or VIntervals_B
	unsigned short current_VIntervals_num;
	struct VoxVInterval **next_VIntervals;
	unsigned short next_VIntervals_num;

	unsigned short max_VIntervals_num;

	struct VoxVInterval *VIntervals_A;//where intervals really are
	struct VoxVInterval *VIntervals_B;
	//char fill[52];
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
	struct VoxRay *ray;//one per thread

	double clip_min;//min dist for intersection
	double clip_dark;//start dist for darkness
	double clip_alpha;//start dist for alpha
	double clip_sub1;//start dist for sub precision
	double clip_sub2;//start dist for second sub precision
	double clip_sub3;//start dist for third sub precision
	double clip_max;//max dist for intersection

	double f;//focal (in pixels)
	double * fc;//distance from screen column to nodal point
	double render2ScreenFactor; //if render area has different ratio from window
};


void voxray_delete(struct VoxRay * ray);
void voxray_swap_intervals(struct VoxRay * ray);

void voxray_reinit(struct VoxRay * ray,struct Pt3d *cam, int c, bool trace);
double voxray_lambdaNextIntersection(struct VoxRay * ray);
bool voxray_findNextIntersection(struct VoxRay * ray,bool trace);//returns false if out of bounds
void voxray_draw(struct VoxRay * ray,int c,bool trace);
void Voxray_show_info(struct VoxRay * ray);


struct VoxRender * voxrender_create(struct VoxWorld *_world,double f_eq35mm);
void voxrender_setCam(struct VoxRender * render,struct Pt3d _cam,double _ang_hz);
void voxrender_render(struct VoxRender * render,bool trace);
struct Pt3d voxrender_proj(struct VoxRender * render,struct Pt3d P);
void voxrender_delete(struct VoxRender * render);

#endif // VOXRENDER_H
