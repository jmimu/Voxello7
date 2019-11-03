#include "render_threads.h"

#include "voxworld.h"
#include "graph.h"

void *thread_render(void* arg)
{
	struct Render_Th_data * data = (struct Render_Th_data *)arg;
	printf("Start render thread %d\n",data->th_num);
	fflush(stdout);
	while(1)
	{
		printf("sem_wait %d\n",data->th_num);
		fflush(stdout);
		sem_wait(&render_sem);
		printf("render %d\n",data->th_num);
		fflush(stdout);
		for (int c=data->th_num;c<graph.render_w;c+=NB_RENDER_THREADS)
		{
			voxray_reinit(&data->render->ray[data->th_num],&data->render->cam,c,false);
			voxray_draw(&data->render->ray[data->th_num],c,false );
		}
		printf("sem_post %d\n",data->th_num);
		fflush(stdout);
		sem_post(&render_thread_sems[data->th_num]);
	}
	printf("Done render thread\n");
	pthread_exit(NULL);
}



int start_render_th(struct VoxRender * render)
{
	int res;
	res = sem_init(&render_sem,0,0);

	for (int i=0;i<NB_RENDER_THREADS;i++)
	{
		ren_threads_data[i].th_num=i;
		ren_threads_data[i].render=render;
		res = sem_init(&render_thread_sems[i],0,1);
		printf("Will create render thread %d, %d\n",i,ren_threads_data[i].th_num=i);
		res = pthread_create(&(ren_threads[i]), NULL, thread_render, (void*)&(ren_threads_data[i]));
		if (res != 0)
		{
			perror("Thread creation failed!\n");
			return res;
		}
		printf("Has created render thread %d, %d\n",i,ren_threads_data[i].th_num=i);
	}
	return 0;
}
