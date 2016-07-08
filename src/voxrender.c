#include "voxrender.h"
#include "graph.h"
#include "trigo.h"
#include "voxworld.h"
#include <assert.h>

int zen2line(struct VoxRender * render,double zen);
struct VoxVInterval * VoxVInterval_delete(struct VoxVInterval * interval);//return next interval
struct VoxVInterval * VoxVInterval_add(struct VoxVInterval * interval,double zen0,double zen1);//return new interval

//return next interval
struct VoxVInterval * VoxVInterval_delete(struct VoxVInterval * interval)
{
	assert(interval!=NULL);
	
	struct VoxVInterval * next_interval=interval->next;
	if (interval->previous)
		interval->previous->next=interval->next;
	if (interval->next) interval->next->previous=interval->previous;
	free(interval);
	return next_interval;
}

//return new interval
struct VoxVInterval * VoxVInterval_add(struct VoxVInterval * interval,double zen0,double zen1)
{
	struct VoxVInterval * new_interval=(struct VoxVInterval*)malloc(sizeof(struct VoxVInterval));
	new_interval->zenMin=zen0;
	new_interval->zenMax=zen1;
	new_interval->previous=interval;
	if (interval)
	{
		new_interval->next=interval->next;
		if (interval->next) interval->next->previous=new_interval;
		interval->next=new_interval;
	}else{
		new_interval->next=NULL;
	}
	return new_interval;
}



void VoxRay_reinit(struct VoxRay * ray,struct Pt3d *cam, double ang_hz,
				double ang_zen_min, double ang_zen_max, bool trace)
{
	ray->ang_hz=ang_hz;//horizontal angle of the plane
	ray->cam=cam;
	double cosAngHz=_cos(ray->ang_hz);
	double sinAngHz=_sin(ray->ang_hz);

	ray->dirX=0;
	if (sinAngHz>0) ray->dirX=+1;
	if (sinAngHz<0) ray->dirX=-1;
	ray->dirY=0;
	if (cosAngHz>0) ray->dirY=+1;
	if (cosAngHz<0) ray->dirY=-1;

	ray->incX=abs_inv_sin(ray->ang_hz);//how much to increment for 1 step in x or y
	ray->incY=abs_inv_cos(ray->ang_hz);
	ray->currentLambda=0;//where we are on the ray

	double offsetX,offsetY;
	if (sinAngHz>0)
	{
		offsetX=(floor(ray->cam->x+1)-ray->cam->x)*ray->incX;
		ray->currentX=floor(ray->cam->x);
	}
	else
	{
		offsetX=(ray->cam->x-floor(ray->cam->x))*ray->incX;
		ray->currentX=floor(ray->cam->x);
	}

	if (cosAngHz>0)
	{
		offsetY=(floor(ray->cam->y+1)-ray->cam->y)*ray->incY;
		ray->currentY=floor(ray->cam->y);
	}
	else
	{
		offsetY=(ray->cam->y-floor(ray->cam->y))*ray->incY;
		ray->currentY=floor(ray->cam->y);
	}

	ray->nextXLambda=offsetX;
	ray->nextYLambda=offsetY;
	ray->lastIntersectionWasX=false;//has no sens for now

	if (trace)
		printf("Cam:  ang: %f   pos: %f %f %f\n",ang_hz,cam->x,cam->y,cam->z);
	if (trace)
		printf("Ray:   incX: %f, incY: %f, offX: %f, offY: %f\n",ray->incX,ray->incY,offsetX,offsetY);
	if (trace)
		printf("Ray:   currentX: %d, currentY: %d\n",ray->currentX,ray->currentY);


	//TODO: make it in one pass
	while (VoxRay_lambdaNextIntersection(ray) < ray->render->clip_min)
		VoxRay_findNextIntersection(ray,trace);

	if (trace)
		printf("currentLambda: %f,  nextXLambda: %f,  nextYLambda: %f\n",ray->currentLambda,ray->nextXLambda,ray->nextYLambda);

	ray->first_VInterval=(struct VoxVInterval*)malloc(sizeof(struct VoxVInterval));
	ray->first_VInterval->zenMin=ang_zen_min;
	ray->first_VInterval->zenMax=ang_zen_max;
	ray->first_VInterval->previous=NULL;
	ray->first_VInterval->next=NULL;
}

double VoxRay_lambdaNextIntersection(struct VoxRay * ray)
{
	if (ray->nextXLambda<ray->nextYLambda)
		return ray->nextXLambda;
	else
		return ray->nextYLambda;
}


//returns false if out of bounds
bool VoxRay_findNextIntersection(struct VoxRay * ray,bool trace)
{
	if (ray->nextXLambda<ray->nextYLambda)
	{
		ray->currentX+=ray->dirX;
		ray->currentLambda=ray->nextXLambda;
		if (trace)
			printf("intersection X  %f",ray->nextXLambda);
		ray->nextXLambda+=ray->incX;
		ray->lastIntersectionWasX=true;
	}else{
		ray->currentY+=ray->dirY;
		ray->currentLambda=ray->nextYLambda;
		if (trace)
			printf("intersection Y  %f",ray->nextYLambda);
		ray->nextYLambda+=ray->incY;
		ray->lastIntersectionWasX=false;
	}
	//std::cout<<"Next: "<<ray->nextXLambda<<" "<<ray->nextYLambda<<std::endl;
	if (trace)
		printf(" current %d %d\n",ray->currentX,ray->currentY);

	//test if out of bounds
	if ((ray->dirX>0)&&(ray->currentX>ray->world->szX)) return false;
	if ((ray->dirX<0)&&(ray->currentX<0)) return false;
	if ((ray->dirY>0)&&(ray->currentY>ray->world->szY)) return false;
	if ((ray->dirY<0)&&(ray->currentY<0)) return false;
	if (ray->currentLambda > ray->render->clip_max) return false;
	return true;
}

void VoxRay_draw(struct VoxRay * ray,int screen_col,bool trace)
{
	int nextX;
	int nextY;
	struct RLE_block * currentCol;
	double zMin,zMax;
	int voxIndex=0;
	int voxZ=0;
	int previous_voxZ=0;
	int previous_v=UNINIT;
	uint8_t v;
	Uint32 color;
	
	while ((ray->first_VInterval)&&(VoxRay_findNextIntersection(ray,trace)))
	{
		nextX=ray->currentX;
		nextY=ray->currentY;

		if (trace)
			printf("nextX: %d  nextY: %d\n",nextX,nextY);
		
		if ((nextX>=0)&&(nextX<ray->world->szX)
				&&(nextY>=0)&&(nextY<ray->world->szY))
		{
			currentCol=ray->world->data[nextY][nextX];
			struct VoxVInterval * interval=ray->first_VInterval;
			struct VoxVInterval *next_first_VInterval=NULL;
			struct VoxVInterval *next_current_VInterval=NULL;
			while (interval)
			{
				//compute z range of intersection
				zMin=ray->cam->z+ray->currentLambda*_tan(interval->zenMin);
				zMax=ray->cam->z+ray->currentLambda*_tan(interval->zenMax)+1;
				if (zMin<0) zMin=0;
				if (zMax>ray->world->szZ-0.00001)
					zMax=ray->world->szZ-0.00001;
				
				if (trace)
					printf("interval: %f %f =>  Z: %f %f\n",interval->zenMin,interval->zenMax,zMin,zMax);

				if (zMin>zMax)
				{
					interval=VoxVInterval_delete(interval);
					continue;
				}

				//get voxel at zMin
				voxIndex=0;
				voxZ=0;
				previous_voxZ=0;
				previous_v=UNINIT;
				v=EMPTY;
				
				while (voxZ+currentCol[voxIndex].n<floor(zMin))
				{
					v=currentCol[voxIndex].v;
					voxZ+=currentCol[voxIndex].n;
					voxIndex++;
				}

				if (trace)
					printf("start at voxIndex: %d, voxZ: %d, v=%d\n",voxIndex, voxZ, v);

				//test: draw whole voxel space
				double zen0=_atan((zMin-ray->cam->z)/ray->currentLambda);
				if (zen0<interval->zenMin)
					zen0=interval->zenMin;
				double zen1;

				while (voxZ<zMax)
				{
					if (previous_v==UNINIT)
					{
						v=currentCol[voxIndex].v;
						previous_v=v;
					}else{
						previous_v=v;
						v=currentCol[voxIndex].v;
					}
					previous_voxZ=voxZ;
					voxZ+=currentCol[voxIndex].n;
					voxIndex++;
					zen1=_atan((voxZ-ray->cam->z)/ray->currentLambda);
					if (zen1>interval->zenMax)
						zen1=interval->zenMax;
					if (zen1<zen0) //TODO: understand why it occurs...
						continue;
					if (trace)
						printf("value:%d for zen in %f %f\n",v,zen0,zen1);
					
					if (v==EMPTY)
					{
						//test if need to draw top of vox below
						if ((previous_v!=EMPTY)&&(zen0<0))
						{
							double next_lambda=VoxRay_lambdaNextIntersection(ray);
							double zen_tmp=_atan((previous_voxZ-ray->cam->z)/next_lambda);
							color=ray->world->colorMap[previous_v];
							color=color_bright(color,0.6);
							graph_vline(screen_col,zen2line(ray->render,zen0),
									zen2line(ray->render,zen_tmp),color);
							if (trace)
								printf("draw top %d %d : %x (zen_tmp=%f)\n",zen2line(ray->render,zen0),zen2line(ray->render,zen_tmp),color,zen_tmp);
							zen0=zen_tmp;
						}
						
						//create a new interval for next vox column 
						if (!next_first_VInterval)
						{
							next_first_VInterval=VoxVInterval_add(NULL,zen0,zen1);
							next_current_VInterval=next_first_VInterval;
						}else{
							next_current_VInterval=VoxVInterval_add(next_current_VInterval,zen0,zen1);
						}
						if (trace)
							printf("save for next interval %f %f\n",zen0,zen1);
						
					}else{
						//test if need to draw bottom of vox
						if ((previous_v==EMPTY)&&(zen0>0))
						{
							double next_lambda=VoxRay_lambdaNextIntersection(ray);
							double zen_tmp=_atan((previous_voxZ-ray->cam->z)/next_lambda);
							color=ray->world->colorMap[v];
							color=color_bright(color,0.6);
							graph_vline(screen_col,zen2line(ray->render,zen_tmp),
									zen2line(ray->render,zen0),color);
							if (trace)
								printf("draw bottom %d %d : %x, (zen_tmp=%f)\n",zen2line(ray->render,zen_tmp),zen2line(ray->render,zen0),color,zen_tmp);
							//remove this interval to last next_current_VInterval:
							if (next_current_VInterval)
								next_current_VInterval->zenMax=zen_tmp;
						}

						color=ray->world->colorMap[v];
						if (ray->lastIntersectionWasX)
							color=color_bright(color,0.8);
						graph_vline(screen_col,zen2line(ray->render,zen0),
								zen2line(ray->render,zen1),color);
						if (trace)
							printf("draw %d %d : %x\n",zen2line(ray->render,zen0),zen2line(ray->render,zen1),color);

					}
					zen0=zen1;
				}
				interval=VoxVInterval_delete(interval);
			}
			ray->first_VInterval=next_first_VInterval;
		}

		//next intersection
	}
	if (trace)
		printf("\n");


}

void Voxray_show_info(struct VoxRay * ray)
{
	if (ray->lastIntersectionWasX)
		printf("intersection: X\n");
	else
		printf("intersection: Y\n");
	printf("At (%d,%d) %f\n",ray->currentX,ray->currentY,ray->currentLambda);
}



struct VoxRender * VoxRender_create(struct VoxWorld *_world,double _fov_hz)
{
	struct VoxRender *render = (struct VoxRender *) malloc(sizeof(struct VoxRender));
	render->world=_world;
	render->fov_hz=_fov_hz;
	render->fov_vert=_fov_hz*graph.render_h/graph.render_w;
	
	render->ray.render=render;
	render->ray.world=render->world;

	render->clip_min=2;
	render->clip_max=50;
	return render;
}

void VoxRender_setCam(struct VoxRender * render,struct Pt3d _cam,double _center_ang_hz,double _center_ang_vert)
{
	render->cam=_cam;
	render->center_ang_hz=_center_ang_hz;
	render->center_ang_vert=_center_ang_vert;

	//compute agular steps
	render->current_hz_angle=render->center_ang_hz-render->fov_hz/2;
	render->step_hz_angle=render->fov_hz/graph.render_w;
	render->start_vert_angle=render->center_ang_vert-render->fov_vert/2;
	render->stop_vert_angle=render->center_ang_vert+render->fov_vert/2;

	//zen2line formula is:
	//l(zen)=H-H*(zen-start_vert_angle)/(stop_vert_angle-start_vert_angle)
	//=zen*(-H/(stop_vert_angle-start_vert_angle))+H(1+start_vert_angle/(stop_vert_angle-start_vert_angle))
	//=zen*zen2line_factor+zen2line_offset
	render->zen2line_factor=(-graph.render_h/(render->stop_vert_angle-render->start_vert_angle));
	render->zen2line_offset=graph.render_h*(1+render->start_vert_angle/(render->stop_vert_angle-render->start_vert_angle));

}


void VoxRender_render(struct VoxRender * render,bool trace)
{
	for (int currentCol=0;currentCol<graph.render_w;currentCol++)
	{
		//if (currentCol==200)
		{
			VoxRay_reinit(&render->ray,&render->cam,render->current_hz_angle,
				render->start_vert_angle,render->stop_vert_angle, (trace&&(currentCol==graph.render_w/2)));
			VoxRay_draw(&render->ray,currentCol, (trace&&(currentCol==graph.render_w/2)) );
		}
		render->current_hz_angle+=render->step_hz_angle;
	}
}


int zen2line(struct VoxRender * render,double zen)
{
	return zen*render->zen2line_factor+render->zen2line_offset;
}

void VoxRender_limit_tilt(struct VoxRender *render, double * angle_vert)
{
	if ((*angle_vert)-render->fov_vert/2<-PI/2)
		(*angle_vert)=render->fov_vert/2-PI/2;
	if ((*angle_vert)+render->fov_vert/2>PI/2)
		(*angle_vert)=-render->fov_vert/2+PI/2;
}
