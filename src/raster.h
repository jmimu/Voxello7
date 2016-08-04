#ifndef RASTER_H
#define RASTER_H

#include <stdint.h>

#define MAXANIMLEN 50
#define ANIMUNIT 100 //one step in anim is... (in ms/time_factor)

/***
	Generic functions to handle :
	 - raster loading
	 - raster drawing
*/

struct Raster{//TODO: add origin?
	int w,h;
	uint32_t *pix;

	struct Raster* previous; //used only for memory cleaning
	struct Raster* next; //used only for memory cleaning
};

struct Anim{
	struct Raster* rasters[MAXANIMLEN];
	uint8_t len;
	float time_factor; //time is increased by dt*time_factor
	float time;//time counter of animation
	uint8_t step;//current/ANIMUNIT
};


extern struct Raster* lastRaster;

struct Raster* raster_load(const char* filename);
void raster_draw(struct Raster* raster, int x, int y,uint16_t z);
void raster_draw_zoom(struct Raster* raster, int x, int y, uint16_t z, int w, int h);

void raster_unloadall();

struct Anim* anim_create(float _time_factor);
int anim_add_raster(struct Anim* anim,struct Raster* raster);//0 if error
void anim_frame(struct Anim* anim, float dt);
struct Raster* anim_get_raster(struct Anim* anim);
//void anim_free(struct Anim* anim);


#endif // RASTER_H
