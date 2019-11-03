#ifndef RENDER_THREADS_H
#define RENDER_THREADS_H

#ifndef NB_RENDER_THREADS
#define NB_RENDER_THREADS 8
#endif

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
sem_t rth_start_sems[NB_RENDER_THREADS]; //to synchro all ren_threads
sem_t rth_done_sems[NB_RENDER_THREADS];//to synchro main loop with all ren_threads

struct VoxRay rth_rays[NB_RENDER_THREADS];//one per thread
uint32_t *threadColPixels[NB_RENDER_THREADS];//one column for one thread
uint16_t *threadColzbuf[NB_RENDER_THREADS];//one column for one thread

void *thread_render(void* arg);
int start_render_th(struct VoxRender * render);
void graph_vline_threadCol(int thread,int y1,int y2,uint32_t rgba,uint16_t z);
void graph_clear_threadCol(int thread, uint16_t z);
void graph_write_threadCol(int thread, int x);

void del_render_th();

#endif // RENDER_THREADS_H
