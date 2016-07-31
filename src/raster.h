#ifndef RASTER_H
#define RASTER_H


/***
	Generic functions to handle :
	 - raster loading
	 - raster drawing
*/

struct Raster{
	int w,h;
	uint32_t *pix;
	char * name;

	struct Raster* previous; //used only for memory cleaning
	struct Raster* next; //used only for memory cleaning
};

extern struct Raster* lastRaster;

struct Raster* raster_load(const char* filename);
void raster_draw(struct Raster* raster, int x, int y,uint16_t z);
void raster_draw_zoom(struct Raster* raster, int x, int y, int w, int h);

void raster_unloadall();


#endif // RASTER_H
