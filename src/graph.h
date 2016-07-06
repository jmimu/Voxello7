#ifndef GRAPH_H
#define GRAPH_H

#include <SDL2/SDL.h>
#include <stdbool.h>

/***
	Generic functions to handle :
	 - window opening
	 - primitive drawing
	 - FPS calculating & managing (later...)
	 - TODO: zbuffer
*/

struct Graph{
		int window_w,window_h;
		int render_w,render_h;
		SDL_Window* window;
		SDL_Renderer *renderer;
		SDL_Texture *texture;		
		uint32_t *myPixels;
};

extern struct Graph graph;

bool graph_init(int _window_w,int _window_h,
				int _render_w,int _render_h,const char* title);

void graph_putpixel_rgb(int x,int y,uint8_t r,uint8_t g,uint8_t b);//todo: add z!
void graph_putpixel(int x,int y,uint32_t rgba);//todo: add z!
void graph_vline(int x,int y1,int y2,uint32_t rgba);
void graph_close();
void graph_start_frame();
void graph_end_frame();

void graph_test();

#endif // GRAPH_H
