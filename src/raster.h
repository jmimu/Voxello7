#ifndef RASTER_H
#define RASTER_H

#include <stdint.h>

#define MAXANIMLEN 50
#define ANIMUNIT 1000 //one step in anim is...

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
	int speed; //add to step every frame
	long current;//num of current raster (change every ANIMUNIT)
	uint8_t step;//current/ANIMUNIT
};


extern struct Raster* lastRaster;

struct Raster* raster_load(const char* filename);
void raster_draw(struct Raster* raster, int x, int y,uint16_t z);
void raster_draw_zoom(struct Raster* raster, int x, int y, uint16_t z, int w, int h);

void raster_unloadall();

struct Anim* anim_create(int _speed);
int anim_add_raster(struct Anim* anim,struct Raster* raster);//0 if error
void anim_frame(struct Anim* anim);//one game frame
struct Raster* anim_get_raster(struct Anim* anim);
//void anim_free(struct Anim* anim);


#endif // RASTER_H
