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
struct VoxVInterval
{
	double zenMin;
	double zenMax;
	struct VoxVInterval * next;
	struct VoxVInterval * previous;
};

struct VoxRay
{
	struct VoxRender *render;
	struct VoxWorld *world;
	struct Pt3d * cam;
	double ang_hz;//horizontal angle of the plane
	double incX,incY;//how much to increment for 1 step in x or y
	double currentLambda;//where we are on the ray
	double nextXLambda;//next X intersection
	double nextYLambda;//next Y intersection

	int currentX,currentY;
	bool lastIntersectionWasX;

	int dirX,dirY;//-1, 0 or 1

	struct VoxVInterval *first_VInterval;
};



struct VoxRender
{	
	struct VoxWorld *world;
	struct Pt3d cam;
	double center_ang_hz;
	double center_ang_vert;
	double fov_hz;//field of view hz
	double fov_vert;//field of view hz
	struct VoxRay ray;
	double zen2line_factor,zen2line_offset;
	double current_hz_angle;
	double step_hz_angle;
	double start_vert_angle;
	double stop_vert_angle;

	double clip_min;//min dist for intersection
	double clip_max;//max dist for intersection
};



void VoxRay_reinit(struct VoxRay * ray,struct Pt3d *cam, double ang_hz,
				double ang_zen_min, double ang_zen_max, bool trace);
double VoxRay_lambdaNextIntersection(struct VoxRay * ray);
bool VoxRay_findNextIntersection(struct VoxRay * ray,bool trace);//returns false if out of bounds
void VoxRay_draw(struct VoxRay * ray,int screen_col,bool trace);
void Voxray_show_info(struct VoxRay * ray);


struct VoxRender * VoxRender_create(struct VoxWorld *_world,double _fov_hz);
void VoxRender_setCam(struct VoxRender * render,struct Pt3d _cam,double _center_ang_hz,double _center_ang_vert);
void VoxRender_render(struct VoxRender * render,bool trace);
void VoxRender_limit_tilt(struct VoxRender *render, double * angleZ);

#endif // VOXRENDER_H
