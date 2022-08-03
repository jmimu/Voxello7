#include "graph.h"

#include <stdio.h>
#include <stdlib.h>

#ifdef __PC__
  #include <omp.h>
  #include <SDL2/SDL_image.h>
  #include "pc/raster.h"
#endif

#ifndef WITH_OMP
  int omp_get_max_threads() {return 1;}
  int omp_get_thread_num() {return 0;}
#endif

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
    graph.render2ScreenFactor = ((((double)graph.window_w)/graph.window_h)/(((double)graph.render_w)/graph.render_h));
    printf("render2ScreenFactor: %f\n",graph.render2ScreenFactor);
#ifdef __PC__
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

    // OGL part (inspired from https://gist.github.com/koute/7391344)
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
    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
    SDL_GL_SetAttribute( SDL_GL_ACCELERATED_VISUAL, 1 );
    SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 8 );
    SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 8 );
    SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 8 );
    SDL_GL_SetAttribute( SDL_GL_ALPHA_SIZE, 8 );

    graph.window = SDL_CreateWindow(title,SDL_WINDOWPOS_CENTERED,
                SDL_WINDOWPOS_CENTERED,graph.window_w,graph.window_h,
                SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN_DESKTOP*0);//SDL_WINDOW_FULLSCREEN_DESKTOP

    graph.context = SDL_GL_CreateContext(graph.window);

    check(graph.context,"SDL_GL_CreateContext error: %s\n",SDL_GetError());

    graph.shader = createShader("src/shaders/vertex.shader", "src/shaders/fragment.shader");
    graph_create_quad();

    //if SDL_WINDOW_FULLSCREEN_DESKTOP
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");  // make the scaled rendering look smoother.
    //SDL_RenderSetLogicalSize(sdlRenderer, 640, 480);

    glDisable( GL_DEPTH_TEST );
    glClearColor( 0.5, 0.0, 0.0, 0.0 );
    glViewport( 0, 0, graph.window_w,graph.window_h );

    //graph.renderer = SDL_CreateRenderer(graph.window, -1, 0);
    //graph.surface = SDL_CreateRGBSurface(0,graph.render_w,graph.render_h,32,0x00ff0000,0x0000ff00,0x000000ff,0xff000000);
    //graph.texture = SDL_CreateTextureFromSurface(graph.renderer,graph.surface);

    graph.surface = SDL_CreateRGBSurface(0,graph.render_w,graph.render_h,32,0x00ff0000,0x0000ff00,0x000000ff,0xff000000);
    //graph.pixels = graph.surface->pixels;//(uint32_t*) malloc(graph.render_w*graph.render_h*sizeof(uint32_t));
    graph_create_data();
#endif
#ifdef __3DS__
    gfxInitDefault();
    consoleInit(GFX_TOP, NULL);
    printf("\x1b[21;16HPress Start to exit.");
    gfxSetDoubleBuffering(GFX_BOTTOM, false);
    graph.pixels = gfxGetFramebuffer(GFX_BOTTOM, GFX_LEFT, NULL, NULL);
#endif
    //graph.zbuf = (uint16_t*) malloc(graph.render_w*graph.render_h*sizeof(uint16_t));
    //graph.threadColPixels = (uint32_t**) malloc (nb_threads*sizeof(uint32_t*));
    //graph.threadColzbuf = (uint16_t**) malloc (nb_threads*sizeof(uint16_t*));
    /*for (int i=0;i<nb_threads;i++)
    {
        graph.threadColPixels[i]=(uint32_t*) malloc (graph.render_h*sizeof(uint32_t));
        graph.threadColzbuf[i]=(uint16_t*) malloc (graph.render_h*sizeof(uint16_t));
    }*/
    return true;
error:
    return false;
}

void graph_create_quad()
{
    glGenVertexArrays(1, &graph.VAO);//vertex Array Object
    glGenBuffers(1, &graph.VBO);//vertex buffer
    glGenBuffers(1, &graph.EBO);//index buffer
    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(graph.VAO);

    //GL_ARRAY_BUFFER = target
    glBindBuffer(GL_ARRAY_BUFFER, graph.VBO); //dire a la machine a etats qu'on va travailler avec ce buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(verticesData), verticesData, GL_STATIC_DRAW); //on remplit le buffer binde

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, graph.EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indicesData), indicesData, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)0); //attribut 0, 2 coordonnees, decales de 6*4o, offset 0o => add it to vertex array
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(2*sizeof(float))); //attribut 1, 3 coordonnees, decales de 6*4o, offset 3*4o
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(5 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // remember: do NOT unbind the EBO while a VAO is active as the bound element buffer object IS stored in the VAO; keep the EBO bound.
    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);


    glGenTextures(1, &graph.textureId);
    glBindTexture(GL_TEXTURE_2D, graph.textureId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

}

void graph_create_data()
{
    graph.rasterData.pixels = graph.surface->pixels;
    graph.rasterData.zbuf = (uint16_t*) malloc(graph.render_w*graph.render_h*sizeof(uint16_t));
    graph.rasterData.normale = (uint32_t*) malloc(graph.render_w*graph.render_h*sizeof(uint32_t));

    nb_threads=omp_get_max_threads();
    graph.threadsData = malloc(nb_threads*sizeof(struct GraphData));
    for (int i=0;i<nb_threads;i++)
    {
        graph.threadsData[i].pixels = (uint32_t*) malloc(graph.render_w*graph.render_h*sizeof(uint32_t));
        graph.threadsData[i].zbuf = (uint16_t*) malloc(graph.render_w*graph.render_h*sizeof(uint16_t));
        graph.threadsData[i].normale = (uint32_t*) malloc(graph.render_w*graph.render_h*sizeof(uint32_t));
    }
}

void graph_start_frame()
{
#ifdef __PC__
    memset(graph.rasterData.pixels, 0x00, graph.render_w*graph.render_h*4);
    memset(graph.rasterData.zbuf, 0xFF, graph.render_w*graph.render_h*2);
    memset(graph.rasterData.normale, 0x80, graph.render_w*graph.render_h*4);
    nb_threads=omp_get_max_threads();
    for (int i=0;i<nb_threads;i++)
    {
        memset(graph.threadsData[i].pixels, 0x00, graph.render_w*graph.render_h*4);
        memset(graph.threadsData[i].zbuf, 0xFF, graph.render_w*graph.render_h*2);
        memset(graph.threadsData[i].normale, 0x80, graph.render_w*graph.render_h*4);
    }
    glClear( GL_COLOR_BUFFER_BIT );
#endif

#ifdef DBG_GRAPH
    graph_vline(graph.render_w/2,0,graph.render_h,0x40B000);//0xFF00B040
#endif
}

void graph_end_frame()
{
#ifdef __PC__
    //merge all threads' pixels
    nb_threads=omp_get_max_threads();
#ifdef WITH_OMP
	#pragma omp parallel for schedule(guided)
#endif
    for (int p=1;p<graph.render_w*graph.render_h;p++)
    {
        for (int i=1;i<nb_threads;i++)
        {
            graph.threadsData[0].pixels[p]+=graph.threadsData[i].pixels[p];
            graph.threadsData[0].zbuf[p]+=graph.threadsData[i].zbuf[p];
            graph.threadsData[0].normale[p]+=graph.threadsData[i].normale[p];
        }
        graph.threadsData[0].zbuf[p]+=nb_threads-1;//because we add -1/0xFFFF for each unused threads
    }

    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,  graph.render_w,  graph.render_h, 0, GL_RGBA, GL_UNSIGNED_BYTE, graph.rasterData.pixels);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,  graph.render_w,  graph.render_h, 0, GL_RGBA, GL_UNSIGNED_BYTE, graph.threadsData[0].pixels);

    glUseProgram(graph.shader->shaderProgram);
    glUniform1i(glGetUniformLocation(graph.shader->shaderProgram, "textureVox"), 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, graph.textureId);

    glBindVertexArray( graph.VAO );
    glDrawElements( GL_TRIANGLES, sizeof(indicesData)/sizeof(indicesData[0]), GL_UNSIGNED_INT, 0);

    SDL_GL_SwapWindow( graph.window );
#endif
#ifdef __3DS__
    // Flush and swap framebuffers
    gfxFlushBuffers();
    gfxSwapBuffers();

    //Wait for VBlank
    gspWaitForVBlank();
#endif
}


void graph_putpixel_rgb(int x,int y,uint8_t r,uint8_t g,uint8_t b)
{
#ifdef __PC__
    graph.rasterData.pixels[x+y*graph.render_w]=(r<<16)+(g<<8)+(b)+0xFF000000;
#endif
#ifdef __3DS__
    long i = (y+x*graph.render_h)*3;
    graph.pixels[i++]=b;
    graph.pixels[i++]=g;
    graph.pixels[i]=r;
#endif
}

void graph_putpixel(int x,int y,uint32_t rgba)
{
#ifdef __PC__
    graph.rasterData.pixels[x+y*graph.render_w]=rgba;
#endif
#ifdef __3DS__
    long i = (y+x*graph.render_h)*3;
    graph.pixels[i++]=rgba;
    graph.pixels[i++]=rgba>>8;
    graph.pixels[i]=rgba>>16;
#endif
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
#ifdef __PC__
    i=x+ymin*graph.render_w;
    for (int y=ymin+1;y<=ymax;y++)
    {
        graph.rasterData.pixels[i]=rgba;
        i+=graph.render_w;
    }
#endif
#ifdef __3DS__
    i = (graph.render_h-ymin+x*graph.render_h)*3;
    for (int y=ymin+1;y<=ymax;y++)
    {
      graph.pixels[i++]=rgba;
      graph.pixels[i++]=rgba>>8;
      graph.pixels[i]=rgba>>16;
        i++;
    }
#endif
}

void graph_vline_threadCol(int thread,int x, int y1,int y2,uint32_t rgba,uint16_t z)
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

    long i = x + (ymin+1)*graph.render_w;
    for (int y=ymin+1;y<=ymax;y++)
    {
        if (graph.threadsData[thread].zbuf[i]>z)
        {
            graph.threadsData[thread].pixels[i]=rgba;
            graph.threadsData[thread].zbuf[i]=z;
        }
        i += graph.render_w;
    }
}
/*
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
*/
/*void graph_write_threadCol(int thread, int x)
{
#ifdef __PC__
    unsigned int i=x;
    for (int y=0;y<graph.render_h;y++)
    {
        graph.pixels[i]=graph.threadColPixels[thread][y];
        graph.zbuf[i]=graph.threadColzbuf[thread][y];
        i+=graph.render_w;
    }
#endif
#ifdef __3DS__
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
#endif
}*/


void graph_close()
{
#ifdef __PC__
    glDeleteTextures(1, &graph.textureId);
    deleteShader(graph.shader);
    glDeleteVertexArrays(1, &graph.VAO);
    glDeleteBuffers(1, &graph.VBO);
    glDeleteBuffers(1, &graph.EBO);
    SDL_GL_DeleteContext(graph.context);

    SDL_DestroyWindow(graph.window);

    for (int i=0;i<nb_threads;i++)
    {
        free(graph.threadsData[i].pixels);
        free(graph.threadsData[i].zbuf);
        free(graph.threadsData[i].normale);
    }
    SDL_FreeSurface(graph.surface);
    free(graph.threadsData);
    free(graph.rasterData.zbuf);
    free(graph.rasterData.normale);
    raster_unloadall();
    IMG_Quit();
    SDL_Quit();
#endif
#ifdef __3DS__
    gfxExit();
#endif
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

#ifdef __PC__
/*void ScreenshotBMP(const char * filename)
{
    SDL_Surface * surf = SDL_CreateRGBSurfaceFrom(graph.pixels,
            graph.render_w, graph.render_h, 8*4, graph.render_w*4, 0,0,0,0);
    SDL_SaveBMP(surf, filename);

    SDL_FreeSurface(surf);
}*/
#endif
