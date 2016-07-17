#include "graph.h"

#include <stdio.h>
#include <omp.h>

#include "dbg.h"

struct Graph graph;
int nb_threads=-1;

bool graph_init(int _window_w,int _window_h,
				int _render_w,int _render_h,const char* title)
{
	nb_threads=omp_get_max_threads();
	graph.window_w=_window_w;
	graph.window_h=_window_h;
	graph.render_w=_render_w;
	graph.render_h=_render_h;
	if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
	{
		printf("SDL2 could not initialize! SDL2_Error: %s\n",SDL_GetError());
		return false;
	}
	graph.window = SDL_CreateWindow(title,SDL_WINDOWPOS_CENTERED,
				SDL_WINDOWPOS_CENTERED,graph.window_w,graph.window_h,
				SDL_WINDOW_SHOWN);//SDL_WINDOW_FULLSCREEN_DESKTOP
	//if SDL_WINDOW_FULLSCREEN_DESKTOP
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");  // make the scaled rendering look smoother.
	//SDL_RenderSetLogicalSize(sdlRenderer, 640, 480);

	graph.renderer = SDL_CreateRenderer(graph.window, -1, 0);


	graph.texture = SDL_CreateTexture(graph.renderer,
						   SDL_PIXELFORMAT_ARGB8888,
						   SDL_TEXTUREACCESS_STREAMING,
						   graph.render_w, graph.render_h);
	graph.myPixels=(uint32_t*) malloc (graph.render_w*graph.render_h*sizeof(uint32_t));
	graph.threadColPixels=(uint32_t**) malloc (nb_threads*sizeof(uint32_t*));
	for (int i=0;i<nb_threads;i++)
	{
		graph.threadColPixels[i]=(uint32_t*) malloc (graph.render_h*sizeof(uint32_t));
	}
	return true;
}



void graph_start_frame()
{
	for (int i=0;i<graph.render_w*graph.render_h;i++)
		graph.myPixels[i]=0x00;
#ifdef DBG_GRAPH
		graph_vline(graph.render_w/2,0,graph.render_h,0xFF00B040);
#endif
}

void graph_end_frame()
{
	SDL_UpdateTexture(graph.texture, NULL, graph.myPixels, graph.render_w * sizeof (uint32_t));
	//SDL_RenderClear(renderer);
	SDL_RenderCopy(graph.renderer, graph.texture, NULL, NULL);
	SDL_RenderPresent(graph.renderer);
}


void graph_putpixel_rgb(int x,int y,uint8_t r,uint8_t g,uint8_t b)
{
	graph.myPixels[x+y*graph.render_w]=(r<<16)+(g<<8)+(b)+0xFF000000;
}

void graph_putpixel(int x,int y,uint32_t rgba)
{
	graph.myPixels[x+y*graph.render_w]=rgba;
}

void graph_vline(int x,int y1,int y2,uint32_t rgba)
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
	unsigned int i=x+ymin*graph.render_w;
	for (int y=ymin+1;y<=ymax;y++)
	{
		graph.myPixels[i]=rgba;
		i+=graph.render_w;
	}
}

void graph_vline_threadCol(int thread,int y1,int y2,uint32_t rgba)
{	int ymin;
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
		graph.threadColPixels[thread][y]=rgba;
}

void graph_clear_threadCol(int thread,uint8_t v)
{
	memset (graph.threadColPixels[thread], v, graph.render_h*4 );
}

void graph_write_threadCol(int thread, int x)
{
	unsigned int i=x;
	for (int y=0;y<graph.render_h;y++)
	{
		graph.myPixels[i]=graph.threadColPixels[thread][y];
		i+=graph.render_w;
	}
}

void graph_close()
{
	SDL_DestroyTexture(graph.texture);
	SDL_DestroyRenderer(graph.renderer);
	SDL_DestroyWindow(graph.window);
	for (int i=0;i<nb_threads;i++)
		free(graph.threadColPixels[i]);
	free(graph.threadColPixels);  
	free(graph.myPixels);
	SDL_Quit();
}


void graph_test()
{
	for (int x=0;x<graph.render_w;x++)
	{
		graph_vline(x,0,graph.render_h,x);
	}
}



uint32_t color_bright(uint32_t color,float factor)
{
	int r=(color&0xFF0000)>>16;
	int g=(color&0xFF00)>>8;
	int b=color&0xFF;
	r*=factor;
	g*=factor;
	b*=factor;
	if (r>255) r=255;
	if (g>255) g=255;
	if (b>255) b=255;
	return (r<<16)+(g<<8)+(b)+0xFF000000;
}


void ScreenshotBMP(const char * filename)
{
    SDL_Surface * surf = SDL_CreateRGBSurfaceFrom(graph.myPixels,
			graph.render_w, graph.render_h, 8*4, graph.render_w*4, 0,0,0,0);
    SDL_SaveBMP(surf, filename);

    SDL_FreeSurface(surf);
}
