#include "graph.h"

#include <stdio.h>

#include "dbg.h"

struct Graph graph;

bool graph_init(int _window_w,int _window_h,
				int _render_w,int _render_h,const char* title)
{
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
  return true;
}



void graph_start_frame()
{
  for (int i=0;i<graph.render_w*graph.render_h;i++)
	graph.myPixels[i]=0xFF001020;
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
	int ymin=y1;
	int ymax=y2;
	if (y1>y2)
	{
		ymin=y2;
		ymax=y1;
	}
	if (ymin<0) ymin=0;
	if (ymax>=graph.render_h) ymax=graph.render_h-1;
	unsigned int i=x+ymin*graph.render_w;
	for (int y=ymin;y<=ymax;y++)
	{
		graph.myPixels[i]=rgba;
		i+=graph.render_w;
	}
}


void graph_close()
{
  SDL_DestroyTexture(graph.texture);
  SDL_DestroyRenderer(graph.renderer);
  SDL_DestroyWindow(graph.window);
  
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
	int r=color>>16;
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
