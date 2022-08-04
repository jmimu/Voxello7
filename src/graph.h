#ifndef GRAPH_H
#define GRAPH_H

#ifdef __PC__
  #include <SDL2/SDL.h>
  #include <SDL2/SDL_opengl.h>
  #include "pc/shader.h"
#endif
#ifdef __3DS__
  #include <3ds.h>
#endif

#include <stdint.h>
#include <stdbool.h>

//zbuf in fix point
#define ZBUF_FACTOR 8

#define DBG_GRAPH

#ifdef WITH_OMP
  #include <omp.h>
#else
  int omp_get_max_threads();
  int omp_get_thread_num();
#endif

/***
    Generic functions to handle :
     - window opening
     - primitive drawing
     - FPS calculating & managing (later...)
     - TODO: zbuffer
*/

  //each Thread has a full image, they will be merged with a shader
struct GraphData
{
#ifdef __PC__
    uint32_t *pixels;
#endif
#ifdef __3DS__
    uint8_t *pixels; //where voxrender writes
#endif
    uint16_t *zbuf;
    uint32_t *normale;
};

struct Graph{
        int window_w,window_h;
        int render_w,render_h;
    #ifdef __PC__
        SDL_Window* window;
        //SDL_Renderer *renderer; //screen
        SDL_Surface *surface; //pixels surface (for bg blit)
        //SDL_Texture *texture; //pixels texture
        SDL_GLContext context;
        struct Shader* shader;
        unsigned int EBO, VBO, VAO, textureId;
        //uint32_t *pixels; //surface data, where voxrender writes
    #endif
    #ifdef __3DS__
        //uint8_t *pixels; //where voxrender writes
    #endif
        struct GraphData rasterData;
        struct GraphData* threadsData;

        //uint16_t *zbuf; //zbuffer unit: voxel side*8
        //uint32_t **threadColPixels;//one column for one thread
        //uint16_t **threadColzbuf;//one column for one thread

        double render2ScreenFactor;
};

extern struct Graph graph;
extern int nb_threads;

bool graph_init(int _window_w,int _window_h,
                int _render_w,int _render_h,const char* title);
void graph_create_quad();
void graph_create_data();
void graph_putpixel_rgb(int x,int y,uint8_t r,uint8_t g,uint8_t b);//todo: add z!
void graph_putpixel(int x,int y,uint32_t rgba);//todo: add z!
void graph_vline(int x,int y1,int y2,uint32_t rgba);
void graph_vline_threadCol(int thread, int x, int y1, int y2, uint32_t rgba, uint16_t z, uint32_t normale);
//void graph_clear_threadCol(int thread, uint16_t z);
//void graph_write_threadCol(int thread, int x);
void graph_close();
void graph_start_frame();
void graph_end_frame();
#ifdef __PC__
  //void ScreenshotBMP(const char * filename);
#endif

void graph_test();

uint32_t color_bright(uint32_t color, float factor);
uint32_t color_alpha(uint32_t color,float factor);

#endif // GRAPH_H
