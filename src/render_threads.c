#include "render_threads.h"

#include "voxworld.h"
#include "graph.h"

void *thread_render(void* arg)
{
	struct Render_Th_data * data = (struct Render_Th_data *)arg;
	struct VoxRay * ray = &rth_rays[data->th_num];
	printf("Start render thread %d\n",data->th_num);
	fflush(stdout);
	while(1)
	{
		//printf("sem_wait %d\n",data->th_num);fflush(stdout);
		sem_wait(&rth_start_sems[data->th_num]);
		//printf("render %d\n",data->th_num);fflush(stdout);

		/*for (int c=data->th_num*graph.render_w/NB_RENDER_THREADS;c<(data->th_num+1)*graph.render_w/NB_RENDER_THREADS;c++)
		{
			voxray_reinit(ray,&data->render->cam,c,false);
			voxray_draw(ray,c,false );
		}*/

		for (int c=data->th_num;c<graph.render_w;c+=NB_RENDER_THREADS)
		{
			voxray_reinit(ray,&data->render->cam,c,false);
			voxray_draw(ray,c,false );
		}

		//printf("sem_post %d\n",data->th_num);fflush(stdout);
		sem_post(&rth_done_sems[data->th_num]);
	}
	printf("Done render thread\n");
	pthread_exit(NULL);
}



int start_render_th(struct VoxRender * render)
{
	int res;
	for (int i=0;i<NB_RENDER_THREADS;i++)
	{
		threadColPixels[i]=(uint32_t*) malloc (graph.render_h*sizeof(uint32_t));
		threadColzbuf[i]=(uint16_t*) malloc (graph.render_h*sizeof(uint16_t));
	}

	for (int i=0;i<NB_RENDER_THREADS;i++)
	{
		rth_rays[i].thread=i;
		rth_rays[i].render=render;
		rth_rays[i].world=render->world;
		rth_rays[i].max_VIntervals_num=graph.render_h/2;//change this limit if needed
		rth_rays[i].current_VIntervals_num=0;
		rth_rays[i].VIntervals_A=(struct VoxVInterval *)malloc(rth_rays[i].max_VIntervals_num*sizeof(struct VoxVInterval));
		rth_rays[i].current_VIntervals=&(rth_rays[i].VIntervals_A);
		rth_rays[i].next_VIntervals_num=0;
		rth_rays[i].VIntervals_B=(struct VoxVInterval *)malloc(rth_rays[i].max_VIntervals_num*sizeof(struct VoxVInterval));
		rth_rays[i].next_VIntervals=&(rth_rays[i].VIntervals_B);
	}

	for (int i=0;i<NB_RENDER_THREADS;i++)
	{
		ren_threads_data[i].th_num=i;
		ren_threads_data[i].render=render;
		res = sem_init(&rth_start_sems[i],0,0);
		res = sem_init(&rth_done_sems[i],0,0);
		printf("Will create render thread %d, %d\n",i,ren_threads_data[i].th_num);
		res = pthread_create(&(ren_threads[i]), NULL, thread_render, (void*)&(ren_threads_data[i]));
		if (res != 0)
		{
			perror("Thread creation failed!\n");
			return res;
		}
		printf("Has created render thread %d, %d\n",i,ren_threads_data[i].th_num);
	}
	return 0;
}


void graph_vline_threadCol(int thread,int y1,int y2,uint32_t rgba,uint16_t z)
{
	int ymin;
	int ymax;
	if (y1>y2)
	{
		ymin=graph.render_h-1-y1;
		ymax=graph.render_h-1-y2;
	}else{
		ymin=graph.render_h-1-y2;
		ymax=graph.render_h-1-y1;
	}
	if (ymin<0) ymin=0;
	if (ymax>=graph.render_h) ymax=graph.render_h-1;

	for (int y=ymin+1;y<=ymax;y++)
	{
		if (threadColzbuf[thread][y]>z)
		{
			threadColPixels[thread][y]=rgba;
			threadColzbuf[thread][y]=z;
		}
	}
}

void graph_clear_threadCol(int thread,uint16_t z)
{
	//TODO: optimization : have a clean column, and copy it here
	//memset (graph.threadColPixels[thread], v, graph.render_h*4 );
	for (int y=0;y<graph.render_h;y++)
	{
		threadColPixels[thread][y]=0;//0xFF402010;
		threadColzbuf[thread][y]=z;
	}

}

void graph_write_threadCol(int thread, int x)
{
	unsigned int i=x;
	for (int y=0;y<graph.render_h;y++)
	{
		graph.pixels[i]=threadColPixels[thread][y];
		graph.zbuf[i]=threadColzbuf[thread][y];
		i+=graph.render_w;
	}
}

void del_render_th()
{
	for (int i=0;i<NB_RENDER_THREADS;i++)
	{
		free(threadColPixels[i]);
		free(threadColzbuf[i]);
	}
}
