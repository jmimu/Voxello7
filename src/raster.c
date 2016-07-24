#include "graph.h"

#include <stdio.h>

#include "dbg.h"
#include "raster.h"
#include <SDL2/SDL_image.h>

struct Raster* lastRaster=0;

struct Raster* raster_load(const char* filename)
{
	struct Raster* raster=(struct Raster*)malloc(sizeof(struct Raster));
	SDL_Surface* surf=IMG_Load(filename);
	check(surf!=0,"Impossible to read image file \"%s\"",filename);
	printf("Opened image file \"%s\"",filename);
	check(surf->format->format==SDL_PIXELFORMAT_ARGB8888,"Bad format for image file \"%s\", must be ARGB8888",filename);

	raster->pix=(uint32_t *)malloc(surf->w*surf->h*sizeof(uint32_t));
	memcpy(raster->pix,surf->pixels,surf->w*surf->h);
	raster->h=surf->h;
	raster->w=surf->w;


	SDL_FreeSurface(surf);
	//update raster chain
	raster->previous=lastRaster;
	raster->next=0;
	lastRaster->next=raster;
	lastRaster=raster;
	return raster;

error:
	if (surf) SDL_FreeSurface(surf);
	free(raster);
	return 0;
}

void raster_draw(struct Raster* raster, int x, int y)
{

}

void raster_unload(struct Raster* raster)
{
	if (raster->next)
		raster->next->previous=raster->previous;
	if (raster->previous)
		raster->previous->next=raster->next;
	if (raster==lastRaster)
		lastRaster=raster->previous;

	free(raster->pix);
	free(raster->name);
}

void raster_unloadall()
{
	struct Raster* next;
	while (lastRaster)
	{
		next=lastRaster->previous;
		raster_unload(lastRaster);
		lastRaster=next;
	}
}
