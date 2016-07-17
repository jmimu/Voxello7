#ifndef GRAPH_H
#define GRAPH_H

#include <SDL2/SDL.h>
#include <stdbool.h>

#define DBG_GRAPH

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
		uint32_t **threadColPixels;//one column for one thread
};

extern struct Graph graph;
extern int nb_threads;

bool graph_init(int _window_w,int _window_h,
				int _render_w,int _render_h,const char* title);

void graph_putpixel_rgb(int x,int y,uint8_t r,uint8_t g,uint8_t b);//todo: add z!
void graph_putpixel(int x,int y,uint32_t rgba);//todo: add z!
void graph_vline(int x,int y1,int y2,uint32_t rgba);
void graph_vline_threadCol(int thread,int y1,int y2,uint32_t rgba);
void graph_clear_threadCol(int thread,uint8_t v);
void graph_write_threadCol(int thread, int x);
void graph_close();
void graph_start_frame();
void graph_end_frame();
void ScreenshotBMP(const char * filename);

void graph_test();

uint32_t color_bright(uint32_t color,float factor);


#endif // GRAPH_H
