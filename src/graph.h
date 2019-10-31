#ifndef GRAPH_H
#define GRAPH_H

#include <SDL2/SDL.h>
#include <stdbool.h>

//zbuf in fix point
#define ZBUF_FACTOR 8

#define DBG_GRAPH
#define OPENGL3

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
		SDL_Renderer *renderer; //screen
		SDL_Surface *surface; //pixels texture (only kept for its format)
		SDL_Texture *texture; //pixels texture
		SDL_Texture *background;
#ifdef OPENGL3
		SDL_GLContext context;
#endif
		uint32_t *pixels; //where voxrender writes
		
		uint16_t *zbuf; //zbuffer unit: voxel side*8
		uint32_t **threadColPixels;//one column for one thread
		uint16_t **threadColzbuf;//one column for one thread
};

extern struct Graph graph;
extern int nb_threads;

bool graph_init(int _window_w,int _window_h,
				int _render_w,int _render_h,const char* title);

void graph_putpixel_rgb(int x,int y,uint8_t r,uint8_t g,uint8_t b);//todo: add z!
void graph_putpixel(int x,int y,uint32_t rgba);//todo: add z!
void graph_vline(int x,int y1,int y2,uint32_t rgba);
void graph_vline_threadCol(int thread,int y1,int y2,uint32_t rgba,uint16_t z);
void graph_clear_threadCol(int thread, uint16_t z);
void graph_write_threadCol(int thread, int x);
void graph_close();
void graph_start_frame();
void graph_end_frame(double ang_l,double ang_r);
void ScreenshotBMP(const char * filename);

void graph_test();

uint32_t color_bright(uint32_t color,float factor);
uint32_t color_alpha(uint32_t color,float factor);

#endif // GRAPH_H
