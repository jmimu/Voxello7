#include "graph.h"

#include <stdio.h>

#include "dbg.h"

#ifndef WITH_OMP
  int omp_get_max_threads(){return 1;}
  int omp_get_thread_num(){return 0;}
#endif

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
	graph.render2ScreenFactor = ((((double)graph.window_w)/graph.window_h)/(((double)graph.render_w)/graph.render_h));
	printf("render2ScreenFactor: %f\n",graph.render2ScreenFactor);

	gfxInitDefault();
	consoleInit(GFX_TOP, NULL);
	printf("\x1b[21;16HPress Start to exit.");
	gfxSetDoubleBuffering(GFX_BOTTOM, false);
	graph.pixels = gfxGetFramebuffer(GFX_BOTTOM, GFX_LEFT, NULL, NULL);
	
	//graph.pixels = (uint32_t*) malloc(graph.render_w*graph.render_h*sizeof(uint32_t));
	graph.zbuf = (uint16_t*) malloc(graph.render_w*graph.render_h*sizeof(uint16_t));
	graph.threadColPixels = (uint32_t**) malloc (nb_threads*sizeof(uint32_t*));
	graph.threadColzbuf = (uint16_t**) malloc (nb_threads*sizeof(uint16_t*));
	for (int i=0;i<nb_threads;i++)
	{
		graph.threadColPixels[i]=(uint32_t*) malloc (graph.render_h*sizeof(uint32_t));
		graph.threadColzbuf[i]=(uint16_t*) malloc (graph.render_h*sizeof(uint16_t));
	}
	return true;
}



void graph_start_frame()
{
	//for (int i=0;i<graph.render_w*graph.render_h;i++)
		//graph.pixels[i]=0xFF582012; //a bit slow ??
	memset(graph.pixels, 30, graph.render_w*graph.render_h*3);

#ifdef DBG_GRAPH
		graph_vline(graph.render_w/2,0,graph.render_h,0x40B000);
#endif
}

void graph_end_frame()
{
	// Flush and swap framebuffers
	gfxFlushBuffers();
	gfxSwapBuffers();

	//Wait for VBlank
	gspWaitForVBlank();
}


void graph_putpixel_rgb(int x,int y,uint8_t r,uint8_t g,uint8_t b)
{
	//graph.pixels[x+y*graph.render_w]=(r<<16)+(g<<8)+(b)+0xFF000000;
	long i = (y+x*graph.render_h)*3;
	graph.pixels[i++]=b;
	graph.pixels[i++]=g;
	graph.pixels[i]=r;
}

void graph_putpixel(int x,int y,uint32_t rgba)
{
	//graph.pixels[x+y*graph.render_w]=rgba;
	long i = (y+x*graph.render_h)*3;
	graph.pixels[i++]=rgba;
	graph.pixels[i++]=rgba>>8;
	graph.pixels[i]=rgba>>16;
}

void graph_vline(int x,int y1,int y2,uint32_t rgba)
{
	int ymin;
	int ymax;
	long i;
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
	i = (graph.render_h-ymin+x*graph.render_h)*3;
	for (int y=ymin+1;y<=ymax;y++)
	{
	  graph.pixels[i++]=rgba;
	  graph.pixels[i++]=rgba>>8;
	  graph.pixels[i]=rgba>>16;
		i++;
	}
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
		if (graph.threadColzbuf[thread][y]>z)
		{
			graph.threadColPixels[thread][y]=rgba;
			graph.threadColzbuf[thread][y]=z;
		}
	}
}

void graph_clear_threadCol(int thread,uint16_t z)
{
	//TODO: optimization : have a clean column, and copy it here
	//memset (graph.threadColPixels[thread], v, graph.render_h*4 );
	for (int y=0;y<graph.render_h;y++)
	{
		graph.threadColPixels[thread][y]=0;//0xFF402010;
		graph.threadColzbuf[thread][y]=z;
	}

}

void graph_write_threadCol(int thread, int x)
{
	unsigned long i_zb=x;
	unsigned long i_fb=(x*graph.render_h)*3;
	uint32_t rgba;
	for (int y=graph.render_h-1;y>=0;y--)
	{
	  rgba = graph.threadColPixels[thread][y];
		graph.pixels[i_fb++]=rgba;
		graph.pixels[i_fb++]=rgba>>8;
		graph.pixels[i_fb++]=rgba>>16;
		graph.zbuf[i_zb]=graph.threadColzbuf[thread][y];
		i_zb+=graph.render_w;
	}
}


void graph_close()
{
  gfxExit();
}


void graph_test()
{
	for (int x=0;x<graph.render_w;x++)
	{
		graph_vline(x,0,graph.render_h,x);
	}
}

uint32_t color_bright(uint32_t color, float factor)
{
    int r=(color&0xFF0000)>>16;
    int g=(color&0xFF00)>>8;
    int b=color&0xFF;
    int a=(color&0xFF000000)>>24;
    r*=factor;
    g*=factor;
    b*=factor;
    if (r>255) r=255;
    if (g>255) g=255;
    if (b>255) b=255;
    return (a<<24)+(r<<16)+(g<<8)+b;
}

uint32_t color_alpha(uint32_t color,float factor)
{
	int r=(color&0xFF0000)>>16;
	int g=(color&0xFF00)>>8;
	int b=color&0xFF;
	int a=(color&0xFF000000)>>24;
	a*=factor;
	if (a>255) a=255;
	return (a<<24)+(r<<16)+(g<<8)+b;
}

/*
void ScreenshotBMP(const char * filename)
{
    SDL_Surface * surf = SDL_CreateRGBSurfaceFrom(graph.pixels,
			graph.render_w, graph.render_h, 8*4, graph.render_w*4, 0,0,0,0);
    SDL_SaveBMP(surf, filename);

    SDL_FreeSurface(surf);
}*/
