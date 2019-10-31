#include "graph.h"

#include <stdio.h>
#include <omp.h>
#include <SDL2/SDL_image.h>

#include "dbg.h"
#include "raster.h"
#include "trigo.h"

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
	printf("SDL2 initialized.\n");

	int flags=IMG_INIT_JPG|IMG_INIT_PNG;
	int initted=IMG_Init(flags);
	if((initted&flags) != flags) {
		printf("IMG_Init: Failed to init required jpg and png support!\n");
		printf("IMG_Init: %s\n", IMG_GetError());
		return false;
	}
	printf("SDL2_image initialized.\n");

	graph.window = SDL_CreateWindow(title,SDL_WINDOWPOS_CENTERED,
				SDL_WINDOWPOS_CENTERED,graph.window_w,graph.window_h,
				SDL_WINDOW_OPENGL);//SDL_WINDOW_FULLSCREEN_DESKTOP

#ifdef OPENGL3	
	//OGL part (from Headerphile.com OpenGL Tutorial part 1)
	graph.context = SDL_GL_CreateContext(graph.window);
	// Set our OpenGL version.
	// SDL_GL_CONTEXT_CORE gives us only the newer version, deprecated functions are disabled
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	// 3.2 is part of the modern versions of OpenGL, but most video cards whould be able to run it
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
	// Turn on double buffering with a 24bit Z buffer.
	// You may need to change this to 16 or 32 for your system
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	// This makes our buffer swap syncronized with the monitor's vertical refresh
	SDL_GL_SetSwapInterval(1);
	//test GL context:
	int value = 0;
	SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &value);
	printf("SDL_GL_CONTEXT_MAJOR_VERSION : %d \n",value);
	SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &value);
	printf("SDL_GL_CONTEXT_MINOR_VERSION : %d \n",value);
	//end OGL part
#endif
	
	
	//if SDL_WINDOW_FULLSCREEN_DESKTOP
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");  // make the scaled rendering look smoother.
	//SDL_RenderSetLogicalSize(sdlRenderer, 640, 480);

	graph.renderer = SDL_CreateRenderer(graph.window, -1, 0);
	graph.surface = SDL_CreateRGBSurface(0,graph.render_w,graph.render_h,32,0x00ff0000,0x0000ff00,0x000000ff,0xff000000);
	graph.texture = SDL_CreateTextureFromSurface(graph.renderer,graph.surface);
	
	SDL_Surface* backsurf = IMG_Load("data/back2.jpg");
	SDL_Surface* backsurf_conv=SDL_ConvertSurface(backsurf,graph.surface->format,0);
	graph.background = SDL_CreateTextureFromSurface(graph.renderer, backsurf);
	SDL_FreeSurface(backsurf);
	SDL_FreeSurface(backsurf_conv);
	
	graph.pixels = (uint32_t*) malloc(graph.render_w*graph.render_h*sizeof(uint32_t));
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
	for (int i=0;i<graph.render_w*graph.render_h;i++)
		graph.pixels[i]=0xFF582012;
#ifdef DBG_GRAPH
		graph_vline(graph.render_w/2,0,graph.render_h,0xFF00B040);
#endif
}

void graph_end_frame(double ang_l,double ang_r)
{
	//for (unsigned int i=0;i<graph.render_w*graph.render_h;i++)//show zbuffer
	//	graph.pixels[i]=0xFF000000 | (graph.zbuf[i]>>1)+10;
	SDL_UpdateTexture(graph.texture, NULL, graph.pixels, graph.render_w * sizeof (uint32_t));
	//SDL_RenderClear(renderer);
	
	int w,h;
	if (ang_l<0) {ang_l+=2*PI;ang_r+=2*PI;}
	if (ang_r>4*PI) {ang_l-=2*PI;ang_r-=2*PI;}
	SDL_QueryTexture(graph.background, NULL, NULL,&w,&h);
	w/=2;
	SDL_Rect SrcR;
	SrcR.x = ang_l*w/(2*PI);
	SrcR.y = 0;
	SrcR.w = (ang_r-ang_l)*w/(2*PI);
	SrcR.h = h;

	SDL_RenderCopy(graph.renderer, graph.background, &SrcR, NULL);
	SDL_RenderCopy(graph.renderer, graph.texture, NULL, NULL);
	SDL_RenderPresent(graph.renderer);
}


void graph_putpixel_rgb(int x,int y,uint8_t r,uint8_t g,uint8_t b)
{
	graph.pixels[x+y*graph.render_w]=(r<<16)+(g<<8)+(b)+0xFF000000;
}

void graph_putpixel(int x,int y,uint32_t rgba)
{
	graph.pixels[x+y*graph.render_w]=rgba;
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
		graph.pixels[i]=rgba;
		i+=graph.render_w;
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
	unsigned int i=x;
	for (int y=0;y<graph.render_h;y++)
	{
		graph.pixels[i]=graph.threadColPixels[thread][y];
		graph.zbuf[i]=graph.threadColzbuf[thread][y];
		i+=graph.render_w;
	}
}


void graph_close()
{
#ifdef OPENGL3	
	SDL_GL_DeleteContext(graph.context);
#endif
	SDL_DestroyTexture(graph.texture);
	SDL_DestroyTexture(graph.background);
	SDL_FreeSurface(graph.surface);
	SDL_DestroyRenderer(graph.renderer);
	SDL_DestroyWindow(graph.window);
	for (int i=0;i<nb_threads;i++)
	{
		free(graph.threadColPixels[i]);
		free(graph.threadColzbuf[i]);
	}
	free(graph.threadColPixels);  
	free(graph.threadColzbuf);  
	free(graph.pixels);  
	free(graph.zbuf);  
	raster_unloadall();
	IMG_Quit();
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


void ScreenshotBMP(const char * filename)
{
    SDL_Surface * surf = SDL_CreateRGBSurfaceFrom(graph.pixels,
			graph.render_w, graph.render_h, 8*4, graph.render_w*4, 0,0,0,0);
    SDL_SaveBMP(surf, filename);

    SDL_FreeSurface(surf);
}
