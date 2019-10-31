#include "voxrender.h"
#include "graph.h"
#include "trigo.h"
#include "voxworld.h"
#include <assert.h>
#include <omp.h>

//TODO: work in center-relative coords and transform only before drawing?
int z_to_l(int z, double cam_z, double lambda, double fc);
int l_to_z(int l, double cam_z, double lambda, double fc);

int z_to_l(int z, double cam_z, double lambda, double fc)
{
	return fc*(z-cam_z)/lambda+graph.render_h/2-0.5;
}

int l_to_z(int l, double cam_z, double lambda, double fc)
{
	return (l-graph.render_h/2+0.5)*lambda/fc+cam_z;
}


void voxray_delete(struct VoxRay * ray)
{
	free(ray->VIntervals_A);
	free(ray->VIntervals_B);
}


void voxray_reinit(struct VoxRay * ray,struct Pt3d *cam, int c, bool trace)
{
	ray->cam=cam;
	//t (tx,ty) is the hz vector of the ray in world frame
	double t_x= (c+0.5-graph.render_w/2)*ray->render->ang_hz_cos+ray->render->f*ray->render->ang_hz_sin; 
	double t_y=-(c+0.5-graph.render_w/2)*ray->render->ang_hz_sin+ray->render->f*ray->render->ang_hz_cos; 
	ray->dirX=0;
	if (t_x>0) ray->dirX=+1;
	if (t_x<0) ray->dirX=-1;
	ray->dirY=0;
	if (t_y>0) ray->dirY=+1;
	if (t_y<0) ray->dirY=-1;

	if (trace)
		printf("c:%d  t: %f %f\n",c,t_x,t_y);

	if (fabs(t_x)<0.0001)
		ray->incX=100000;
	else
		ray->incX=fabs(ray->render->fc[c]/t_x);//how much to increment for 1 step in x or y

	if (fabs(t_y)<0.0001)
		ray->incY=100000;
	else
		ray->incY=fabs(ray->render->fc[c]/t_y);
	
	ray->currentLambda=0;//where we are on the ray

	double offsetX,offsetY;
	if (t_x>0)
	{
		offsetX=(floor(ray->cam->x+1)-ray->cam->x)*ray->incX;
		ray->currentX=floor(ray->cam->x);
	}
	else
	{
		offsetX=(ray->cam->x-floor(ray->cam->x))*ray->incX;
		ray->currentX=floor(ray->cam->x);
	}

	if (t_y>0)
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
		printf("Cam:  pos: %f %f %f\n",cam->x,cam->y,cam->z);
	if (trace)
		printf("Ray:   incX: %f, incY: %f, offX: %f, offY: %f\n",ray->incX,ray->incY,offsetX,offsetY);
	if (trace)
		printf("Ray:   currentX: %d, currentY: %d\n",ray->currentX,ray->currentY);


	//TODO: make it in one pass
	while (voxray_lambdaNextIntersection(ray) < ray->render->clip_min)
		voxray_findNextIntersection(ray,trace);

	if (trace)
		printf("currentLambda: %f,  nextXLambda: %f,  nextYLambda: %f\n",ray->currentLambda,ray->nextXLambda,ray->nextYLambda);
	
	ray->current_VIntervals_num=1;
	(*ray->current_VIntervals)[0].l_min=0;
	(*ray->current_VIntervals)[0].l_max=graph.render_h;
	ray->next_VIntervals_num=0;
}

double voxray_lambdaNextIntersection(struct VoxRay * ray)
{
	if (ray->nextXLambda<ray->nextYLambda)
		return ray->nextXLambda;
	else
		return ray->nextYLambda;
}


//returns false if out of bounds
bool voxray_findNextIntersection(struct VoxRay * ray,bool trace)
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

void voxray_draw(struct VoxRay * ray,int screen_col,bool trace)
{
	int x;
	int y;
	struct RLE_block * currentCol;
	int zMin,zMax;
	double fc=ray->render->fc[screen_col];
	int voxIndex=0;
	int voxZ=0;
	int previous_voxZ=0;
	int previous_v=UNINIT;
	uint8_t v;
	Uint32 color;
	graph_clear_threadCol(ray->thread,ray->render->clip_max*ZBUF_FACTOR);
	//graph_clear_threadColZ(ray->thread);
	if ((screen_col==graph.render_w/2))
		graph_vline_threadCol(ray->thread,0,graph.render_h-1,0xFF808080,ray->render->clip_max*ZBUF_FACTOR-1);
	unsigned short current_VInterval_i=0;
		
	while ((ray->current_VIntervals_num>0)&&(voxray_findNextIntersection(ray,trace)))
	{
		//simple way to be faster when far. TODO: use sub-res worlds
		if (ray->currentLambda > ray->render->clip_sub1)
		{
			if (!voxray_findNextIntersection(ray,trace))
				break;
		}
		if (ray->currentLambda > ray->render->clip_sub2)
		{
			voxray_findNextIntersection(ray,trace);
			voxray_findNextIntersection(ray,trace);
			voxray_findNextIntersection(ray,trace);
			if (!voxray_findNextIntersection(ray,trace))
				break;
		}
		if (ray->currentLambda > ray->render->clip_sub3)
		{
			voxray_findNextIntersection(ray,trace);
			voxray_findNextIntersection(ray,trace);
			voxray_findNextIntersection(ray,trace);
			voxray_findNextIntersection(ray,trace);
			voxray_findNextIntersection(ray,trace);
			voxray_findNextIntersection(ray,trace);
			voxray_findNextIntersection(ray,trace);
			if (!voxray_findNextIntersection(ray,trace))
				break;
		}
		x=ray->currentX;
		y=ray->currentY;

		if (trace)
			printf("x: %d  y: %d\n",x,y);
		
		if ((x>=0)&&(x<ray->world->szX)
				&&(y>=0)&&(y<ray->world->szY))
		{
			//printf("x: %d  y: %d\n",x,y);
			currentCol=ray->world->data[y][x];
			current_VInterval_i=0;
			struct VoxVInterval * interval=0;
			ray->next_VIntervals_num=0;
			while (current_VInterval_i<ray->current_VIntervals_num)
			{
				interval=&((*ray->current_VIntervals)[current_VInterval_i]);
				//compute z range of intersection
				zMin=l_to_z(interval->l_min, ray->cam->z, ray->currentLambda, fc);
				zMax=l_to_z(interval->l_max, ray->cam->z, ray->currentLambda, fc)+1;//+1 to look for bottom of vox above
				if (zMin<0)
					zMin=0;
				if (zMax<0)
					continue;//zMax=0;
				if (zMin>ray->world->szZ-1)
				{
					//all the next intervas are out of the world too
					ray->current_VIntervals_num=current_VInterval_i;
					break;
					//zMin=ray->world->szZ-1;
				}
				if (zMax>ray->world->szZ-1)
					zMax=ray->world->szZ-1;

				//opimisation: test col_full_start and col_full_end
				if ((zMin>ray->world->col_full_end[y][x])||(zMax<ray->world->col_full_start[y][x]))
				{
					if (trace)
						printf("full: %d %d =>  Z: %d %d\n",ray->world->col_full_end[y][x],ray->world->col_full_start[y][x],zMin,zMax);
					//create a new interval for next vox column 
					if (interval->l_min<interval->l_max)
					{
						if(ray->next_VIntervals_num<ray->max_VIntervals_num)
						{
							(*ray->next_VIntervals)[ray->next_VIntervals_num].l_min=interval->l_min;
							(*ray->next_VIntervals)[ray->next_VIntervals_num].l_max=interval->l_max;
							ray->next_VIntervals_num++;
						}else{
							printf("Error, too many VIntervals\n");
						}
					}

					current_VInterval_i++;
					continue;
				}
				if (trace)
					printf("interval: %d %d =>  Z: %d %d\n",interval->l_min,interval->l_max,zMin,zMax);

				if (zMin>zMax)
				{
					printf("ERROR: zMin>zMax\n");
					current_VInterval_i++;
					continue;
				}

				//get voxel at zMin
				voxIndex=0;
				voxZ=0;
				previous_voxZ=0;
				previous_v=UNINIT;
				v=UNINIT;
				
				while (voxZ+currentCol[voxIndex].n<zMin+1)
				{
					v=currentCol[voxIndex].v;
					voxZ+=currentCol[voxIndex].n;
					voxIndex++;
					previous_v=v;
				}

				if (trace)
					printf("start at voxIndex: %d, voxZ: %d, v=%d\n",voxIndex, voxZ, v);

				int l0=z_to_l(zMin, ray->cam->z, ray->currentLambda, fc);
				if (l0<interval->l_min)
					l0=interval->l_min;
				int l0_previous=l0;
				int l1;

				while (voxZ<=zMax)
				{
					v=currentCol[voxIndex].v;
					previous_voxZ=voxZ;
					voxZ+=currentCol[voxIndex].n;
					voxIndex++;

					if (trace)
						printf("v: %d (%d), voxZ: %d (%d) voxIndex: %d\n",v, previous_v, voxZ, previous_voxZ,voxIndex);

					l1=z_to_l(voxZ, ray->cam->z, ray->currentLambda, fc);
					if (l1>interval->l_max)
						l1=interval->l_max;
					if (l1<l0) //TODO: understand why it occurs...
						continue;
					if (trace)
						printf("value:%d for l in %d %d\n",v,l0,l1);
					
					if (v==EMPTY)
					{
						//test if need to draw top of vox below
						if ((previous_v!=EMPTY)&&(previous_v!=UNINIT)&&(l0-graph.render_h/2<0))
						{
							double next_lambda=voxray_lambdaNextIntersection(ray);
							int l_tmp=z_to_l(previous_voxZ, ray->cam->z, next_lambda, fc);
							if (l_tmp>l0)//TODO: how l_tmp<l0 is possible?
							{
								if (l_tmp>l1) l_tmp=l1;
								color=ray->world->colorMap[previous_v];
								color=color_bright(color,0.6);
								if (ray->currentLambda>ray->render->clip_dark)
									color=color_bright(color,1-(ray->currentLambda-ray->render->clip_dark)/
										(ray->render->clip_max-ray->render->clip_dark));//clipping
								if (ray->currentLambda>ray->render->clip_alpha)
									color=color_alpha(color,1-(ray->currentLambda-ray->render->clip_alpha)/
										(ray->render->clip_max-ray->render->clip_alpha));//clipping
								//TODO: suspicious l_tmp, have to use zbuffer, why?
								graph_vline_threadCol(ray->thread,l0,l_tmp,color,(ray->currentLambda+next_lambda)*ZBUF_FACTOR/2);
								if (trace)
									printf("draw top %d %d : %x\n",l0,l_tmp,color);
								l0=l_tmp;
							}
						}
						
						//create a new interval for next vox column 
						if (l0<l1)
						{
							if(ray->next_VIntervals_num<ray->max_VIntervals_num)
							{
								(*ray->next_VIntervals)[ray->next_VIntervals_num].l_min=l0;
								(*ray->next_VIntervals)[ray->next_VIntervals_num].l_max=l1;
								ray->next_VIntervals_num++;
							}else{
								printf("Error, too many VIntervals\n");
							}
						}
						if (trace)
							printf("save for next interval %d %d\n",l0,l1);
						
					}else{
						//test if need to draw bottom of vox
						if ((previous_v==EMPTY)&&(l0-graph.render_h/2>0))
						{
							double next_lambda=voxray_lambdaNextIntersection(ray);
							int l_tmp=z_to_l(previous_voxZ, ray->cam->z, next_lambda, fc);
							if (l_tmp<l0) //how is it possible?
							{
								if (trace)
									printf("prepare to draw bottom %d %d (%d)\n",l_tmp,l0,l0_previous);
								if (l_tmp<l0_previous) l_tmp=l0_previous;
								color=ray->world->colorMap[v];
								color=color_bright(color,0.6);
								if (ray->currentLambda>ray->render->clip_dark)
									color=color_bright(color,1-(ray->currentLambda-ray->render->clip_dark)/
										(ray->render->clip_max-ray->render->clip_dark));//clipping
								if (ray->currentLambda>ray->render->clip_alpha)
									color=color_alpha(color,1-(ray->currentLambda-ray->render->clip_alpha)/
										(ray->render->clip_max-ray->render->clip_alpha));//clipping
								graph_vline_threadCol(ray->thread,l_tmp,l0,color,(ray->currentLambda+next_lambda)*ZBUF_FACTOR/2);
								if (trace)
									printf("draw bottom %d %d : %x\n",l_tmp,l0,color);
								//remove this interval to last next_current_VInterval:
								if ((ray->next_VIntervals_num>0)
									&&((*ray->next_VIntervals)[ray->next_VIntervals_num-1].l_max>l_tmp)) //TODO: why???
										(*ray->next_VIntervals)[ray->next_VIntervals_num-1].l_max=l_tmp;
							}
						}

						color=ray->world->colorMap[v];
						if (ray->lastIntersectionWasX)
							color=color_bright(color,0.8);
						if (ray->currentLambda>ray->render->clip_dark)
							color=color_bright(color,1-(ray->currentLambda-ray->render->clip_dark)/
								(ray->render->clip_max-ray->render->clip_dark));//clipping
						if (ray->currentLambda>ray->render->clip_alpha)
							color=color_alpha(color,1-(ray->currentLambda-ray->render->clip_alpha)/
								(ray->render->clip_max-ray->render->clip_alpha));//clipping
						graph_vline_threadCol(ray->thread,l0,l1,color,ray->currentLambda*ZBUF_FACTOR);
						if (trace)
							printf("draw %d %d : %x\n",l0,l1,color);

					}
					if (trace)
						printf("l0_previous=l0 %d %d\n",l0_previous,l0);
					l0_previous=l0;
					l0=l1;
					if (trace)
						printf("end of z: %d/%d\n",voxZ,zMax);
					previous_v=v;
				}
				current_VInterval_i++;
			}
			voxray_swap_intervals(ray);
		}

		//next intersection
	}
	
	#pragma omp critical
	graph_write_threadCol(ray->thread,screen_col);
	//graph_write_threadColZ(ray->thread,screen_col);
	
	if (trace)
		printf("\n");


}

void voxray_swap_intervals(struct VoxRay * ray)
{
	if (ray->current_VIntervals==&(ray->VIntervals_A))
	{
		ray->current_VIntervals=&(ray->VIntervals_B);
		ray->next_VIntervals=&(ray->VIntervals_A);
	}else{
		ray->current_VIntervals=&(ray->VIntervals_A);
		ray->next_VIntervals=&(ray->VIntervals_B);
	}
	ray->current_VIntervals_num=ray->next_VIntervals_num;
	ray->next_VIntervals_num=0;
}

void Voxray_show_info(struct VoxRay * ray)
{
	if (ray->lastIntersectionWasX)
		printf("intersection: X\n");
	else
		printf("intersection: Y\n");
	printf("At (%d,%d) %f\n",ray->currentX,ray->currentY,ray->currentLambda);
}



struct VoxRender * voxrender_create(struct VoxWorld *_world,double f_eq35mm)
{
	struct VoxRender *render = (struct VoxRender *) malloc(sizeof(struct VoxRender));
	render->world=_world;
	render->f=graph.render_w*f_eq35mm/35.0;
	render->fc=(double*)malloc(graph.render_w*sizeof(double));
	for (int c=0;c<graph.render_w;c++)
	{
		render->fc[c]=sqrt(render->f*render->f+(c+0.5-graph.render_w/2)*(c+0.5-graph.render_w/2));
		//printf("render->fc[%d]=%f\n",c,render->fc[c]);
	}
	
	printf("Num of threads: %d\n",omp_get_max_threads());

	render->ray=(struct VoxRay*)malloc(omp_get_max_threads()*sizeof(struct VoxRay));
	for (int i=0;i<omp_get_max_threads();i++)
	{
		render->ray[i].thread=i;
		render->ray[i].render=render;
		render->ray[i].world=render->world;
		render->ray[i].max_VIntervals_num=graph.render_h/2;//change this limit if needed
		render->ray[i].current_VIntervals_num=0;
		render->ray[i].VIntervals_A=(struct VoxVInterval *)malloc(render->ray[i].max_VIntervals_num*sizeof(struct VoxVInterval));
		render->ray[i].current_VIntervals=&(render->ray[i].VIntervals_A);
		render->ray[i].next_VIntervals_num=0;
		render->ray[i].VIntervals_B=(struct VoxVInterval *)malloc(render->ray[i].max_VIntervals_num*sizeof(struct VoxVInterval));
		render->ray[i].next_VIntervals=&(render->ray[i].VIntervals_B);
	}

	render->clip_min=1;
	render->clip_dark=300;
	render->clip_alpha=1950;
	render->clip_max=2000;
	
	render->clip_sub1=(render->clip_dark*9+render->clip_max*1)/10;
	render->clip_sub2=(render->clip_dark*5+render->clip_max*1)/6;
	render->clip_sub3=(render->clip_dark*3+render->clip_max*1)/4;
	return render;
}

void voxrender_setCam(struct VoxRender * render,struct Pt3d _cam,double _ang_hz)
{
	render->cam=_cam;
	render->ang_hz=_ang_hz;
	render->ang_hz_cos=_cos(render->ang_hz);
	render->ang_hz_sin=_sin(render->ang_hz);
}


void voxrender_render(struct VoxRender * render,bool trace)
{
	static int part=0;
	int nb_parts=1;

	int th_id, nthreads;
	#pragma omp parallel private(th_id)
	{
		th_id = omp_get_thread_num();
		nthreads=omp_get_num_threads();
		int start=th_id*graph.render_w/nthreads;
		int stop=(th_id+1)*graph.render_w/nthreads;
		//Choose between threads equality
		//or threads independance
		for (int c=th_id*nb_parts+part;c<graph.render_w;c+=nthreads*nb_parts)
		//for (int c=start+part;c<stop;c+=nb_parts)
		{
			voxray_reinit(&render->ray[th_id],&render->cam,c,(trace&&(c==graph.render_w/2)));
			voxray_draw(&render->ray[th_id],c,(trace&&(c==graph.render_w/2)) );
		}
	}
	part++;
	part%=nb_parts;
}

struct Pt3d voxrender_proj(struct VoxRender * render,struct Pt3d P)
{
	static struct Pt3d q;
	P.x-=render->cam.x;
	P.y-=render->cam.y;
	q.x=render->ang_hz_cos*P.x-render->ang_hz_sin*P.y;
	q.y=render->ang_hz_sin*P.x+render->ang_hz_cos*P.y;
	q.z=P.z-render->cam.z;
	if (fabs(q.y)<render->clip_min)
	{
		q.x=-10000;
		q.y=-10000;
	}else{
		q.x=render->f*q.x/q.y+(graph.render_w>>1);
		q.z=-render->f*q.z/q.y+(graph.render_h>>1);
	}
	return(q);
}

void voxrender_delete(struct VoxRender * render)
{
	free(render->fc);
	for (int i=0;i<omp_get_max_threads();i++)
		voxray_delete(&(render->ray[i]));
	free(render->ray);
	free(render);
}
