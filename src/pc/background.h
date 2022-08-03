#ifndef BACKGROUND_H
#define BACKGROUND_H

#include <SDL2/SDL.h>

//TODO: render as skybox https://learnopengl.com/Advanced-OpenGL/Cubemaps
struct Background
{
	SDL_Surface *background;
	int w,h;
};

struct Background * background_create(char* filename);
void background_draw(struct Background * bg, double ang_l, double ang_r);
void background_delete(struct Background * bg);

#endif // BACKGROUND_H
