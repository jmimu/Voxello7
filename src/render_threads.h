#ifndef RENDER_THREADS_H
#define RENDER_THREADS_H

#define NB_RENDER_THREADS 4

#include <pthread.h>
#include "voxrender.h"
#include "semaphore.h"

struct Render_Th_data
{
	int th_num;
	struct VoxRender * render;
};


pthread_t ren_threads[NB_RENDER_THREADS];
struct Render_Th_data ren_threads_data[NB_RENDER_THREADS];
sem_t render_sem; //to synchro all ren_threads
sem_t render_thread_sems[NB_RENDER_THREADS];//to synchro main loop with all ren_threads


void *thread_render(void* arg);
int start_render_th(struct VoxRender * render);



#endif // RENDER_THREADS_H
